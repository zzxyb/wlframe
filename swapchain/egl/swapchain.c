#include "wlf/swapchain/egl/swapchain.h"
#include "wlf/window/wlf_window.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/renderer/gles/egl.h"
#include "wlf/types/wlf_pixel_format.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/platform/wayland/backend.h"
#endif

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

static void egl_buffer_destroy(struct wlf_buffer *buffer) {
	wlf_buffer_finish(buffer);
}

static const struct wlf_buffer_impl egl_buffer_impl = {
	.destroy = egl_buffer_destroy,
};

#if WLF_HAS_LINUX_PLATFORM
static bool egl_supports_platform_window_surface(const struct wlf_egl *egl) {
	return egl->exts.EXT_platform_base &&
		(egl->exts.platform.wayland.EXT_platform_wayland ||
		 egl->exts.platform.wayland.KHR_platform_wayland);
}
#endif

static void swapchain_destroy(struct wlf_swapchain *swapchain) {
	struct wlf_egl_swapchain *egl_swapchain =
		wlf_egl_swapchain_from_swapchain(swapchain);
	wlf_buffer_drop(&egl_swapchain->buffer);

	if (egl_swapchain->surface != EGL_NO_SURFACE) {
		struct wlf_gles_renderer *gles_renderer =
			wlf_gles_renderer_from_renderer(swapchain->window->state.renderer);
		struct wlf_egl *egl = gles_renderer->egl;
		eglDestroySurface(egl->display, egl_swapchain->surface);
	}
#if WLF_HAS_LINUX_PLATFORM
	if (egl_swapchain->egl_window != NULL) {
		wl_egl_window_destroy(egl_swapchain->egl_window);
	}
#endif
	free(egl_swapchain);
}

static void swapchain_present(struct wlf_swapchain *swapchain,
		const pixman_region32_t *damage) {
	struct wlf_egl_swapchain *egl_swapchain =
		wlf_egl_swapchain_from_swapchain(swapchain);
	struct wlf_gles_renderer *gles_renderer =
		wlf_gles_renderer_from_renderer(swapchain->window->state.renderer);
	struct wlf_egl *egl = gles_renderer->egl;

	EGLBoolean ret;
	if (damage != NULL && (gles_renderer->egl->exts.EXT_swap_buffers_with_damage ||
				gles_renderer->egl->exts.KHR_swap_buffers_with_damage)) {
		int nrects = 0;
		pixman_box32_t *rects =
			pixman_region32_rectangles((pixman_region32_t *)damage, &nrects);
		EGLint egl_damage[4 * (nrects > 0 ? nrects : 1)];
		for (int i = 0; i < nrects; ++i) {
			const pixman_box32_t *r = &rects[i];
			egl_damage[4*i] = r->x1;
			egl_damage[4*i + 1] = r->y1;
			egl_damage[4*i + 2] = r->x2 - r->x1;
			egl_damage[4*i + 3] = r->y2 - r->y1;
		}

		if (nrects == 0) {
			nrects = 1;
			memset(egl_damage, 0, sizeof(egl_damage));
		}

		if (gles_renderer->egl->exts.EXT_swap_buffers_with_damage) {
			ret = egl->procs.eglSwapBuffersWithDamageEXT(gles_renderer->egl->display, egl_swapchain->surface, egl_damage,
				nrects);
		} else {
			ret = egl->procs.eglSwapBuffersWithDamageKHR(egl->display, egl_swapchain->surface, egl_damage,
				nrects);
		}
	} else {
		ret = eglSwapBuffers(egl->display, egl_swapchain->surface);
	}

	if (!ret) {
		wlf_log(WLF_ERROR, "swapchain_present failed: %s",
			wlf_egl_error_str(eglGetError()));
		return;
	}
}

static bool swapchain_resize(struct wlf_swapchain *swapchain, int width,
		int height) {
	swapchain->width = width;
	swapchain->height = height;
	struct wlf_egl_swapchain *egl_swapchain =
		wlf_egl_swapchain_from_swapchain(swapchain);
	egl_swapchain->buffer.width = width;
	egl_swapchain->buffer.height = height;
#if WLF_HAS_LINUX_PLATFORM
	if (egl_swapchain->egl_window != NULL) {
		wl_egl_window_resize(egl_swapchain->egl_window, width, height, 0, 0);
	}
#endif

	return true;
}

static const struct wlf_swapchain_impl swapchain_impl = {
	.destroy = swapchain_destroy,
	.resize = swapchain_resize,
	.present = swapchain_present,
};

struct wlf_swapchain *wlf_egl_swapchain_create(struct wlf_window *window,
		int width, int height, const struct wlf_render_format *format) {
	struct wlf_egl_swapchain *swapchain = calloc(1, sizeof(*swapchain));
	if (swapchain == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_shm_swapchain");
		return NULL;
	}

	wlf_swapchain_init(&swapchain->base, NULL, &swapchain_impl, width, height);
	swapchain->base.window = window;
	wlf_buffer_init(&swapchain->buffer, &egl_buffer_impl, width, height);
	swapchain->base.back = &swapchain->buffer;
	if (!wlf_render_format_copy(&swapchain->base.format, format)) {
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}

	struct wlf_gles_renderer *gles_renderer =
		wlf_gles_renderer_from_renderer(window->state.renderer);
	eglSwapInterval(gles_renderer->egl->display, 0);

#if WLF_HAS_LINUX_PLATFORM
	if (wlf_backend_is_wayland(window->state.backend)) {
		struct wl_surface *surface = wlf_window_native_handle(window);
		if (surface == NULL) {
			wlf_log(WLF_ERROR, "Wayland EGL swapchain requires wl_surface");
			wlf_swapchain_destroy(&swapchain->base);
			return NULL;
		}

		swapchain->egl_window =
			wl_egl_window_create(surface, width, height);
		if (swapchain->egl_window == NULL) {
			wlf_log(WLF_ERROR, "Failed to create wl_egl_window");
			wlf_swapchain_destroy(&swapchain->base);
			return NULL;
		}

		struct wlf_egl *egl = gles_renderer->egl;
		swapchain->config = wlf_egl_choose_config(egl, format);
		if (swapchain->config == NULL) {
			wlf_swapchain_destroy(&swapchain->base);
			return NULL;
		}

		const EGLint platform_surface_attribs[] = {
			EGL_NONE,
		};
		const EGLint window_surface_attribs[] = {
			EGL_NONE,
		};

		if (egl_supports_platform_window_surface(egl)) {
			swapchain->surface = egl->procs.eglCreatePlatformWindowSurfaceEXT(egl->display,
				swapchain->config, swapchain->egl_window,
				platform_surface_attribs);
		} else {
			swapchain->surface = eglCreateWindowSurface(egl->display,
				swapchain->config,
				(EGLNativeWindowType)swapchain->egl_window,
				window_surface_attribs);
		}
		if (swapchain->surface == EGL_NO_SURFACE) {
			wlf_log(WLF_ERROR, "Failed to create EGL surface: %s",
				wlf_egl_error_str(eglGetError()));
			wl_egl_window_destroy(swapchain->egl_window);
			wlf_swapchain_destroy(&swapchain->base);
			return NULL;
		}
	}
#endif

	return &swapchain->base;
}

bool wlf_swapchain_is_egl(const struct wlf_swapchain *swapchain) {
	return swapchain->impl == &swapchain_impl;
}

struct wlf_egl_swapchain *wlf_egl_swapchain_from_swapchain(
		struct wlf_swapchain *swapchain) {
	assert(swapchain->impl == &swapchain_impl);

	struct wlf_egl_swapchain *egl_swapchain =
		wlf_container_of(swapchain, egl_swapchain, base);

	return egl_swapchain;
}

bool wlf_buffer_is_egl(struct wlf_buffer *buffer) {
	return buffer != NULL && buffer->impl == &egl_buffer_impl;
}

struct wlf_egl_swapchain *wlf_egl_swapchain_from_buffer(
		struct wlf_buffer *buffer) {
	if (!wlf_buffer_is_egl(buffer)) {
		return NULL;
	}

	struct wlf_egl_swapchain *swapchain =
		wlf_container_of(buffer, swapchain, buffer);
	return swapchain;
}
