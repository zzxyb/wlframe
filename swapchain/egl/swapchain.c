#include "wlf/swapchain/egl/swapchain.h"
#include "wlf/config.h"
#include "wlf/allocator/egl/allocator.h"
#include "wlf/buffer/egl/buffer.h"
#include "wlf/window/wlf_window.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/renderer/gles/egl.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/platform/wayland/backend.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void swapchain_destroy(struct wlf_swapchain *swapchain) {
	struct wlf_egl_swapchain *egl_swapchain =
		wlf_egl_swapchain_from_swapchain(swapchain);
	wlf_buffer_drop(egl_swapchain->back);
	egl_swapchain->back = NULL;
	swapchain->back = NULL;
	free(egl_swapchain);
}

static void swapchain_present(struct wlf_swapchain *swapchain,
	const pixman_region32_t *damage) {
	struct wlf_egl_swapchain *egl_swapchain =
		wlf_egl_swapchain_from_swapchain(swapchain);
	struct wlf_egl_buffer *egl_buffer =
		wlf_egl_buffer_from_buffer(egl_swapchain->back);
	struct wlf_gles_renderer *gles_renderer =
		wlf_gles_renderer_from_renderer(swapchain->window->state.renderer);
	if (egl_buffer == NULL) {
		wlf_log(WLF_ERROR, "EGL swapchain has no EGL-backed buffer");
		return;
	}
	struct wlf_egl *egl = wlf_egl_buffer_get_egl(egl_buffer);
	EGLSurface surface = wlf_egl_buffer_get_surface(egl_buffer);

	/* Submit wlframe's pacing callback with Mesa's buffer commit. */
	wlf_window_arm_frame(swapchain->window);

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
			/* wlframe/pixman regions use a top-left origin, while EGL
			 * swap-damage rectangles use a bottom-left origin. */
			egl_damage[4*i + 1] = swapchain->height - r->y2;
			egl_damage[4*i + 2] = r->x2 - r->x1;
			egl_damage[4*i + 3] = r->y2 - r->y1;
		}

		if (nrects == 0) {
			nrects = 1;
			memset(egl_damage, 0, sizeof(egl_damage));
		}

		if (gles_renderer->egl->exts.EXT_swap_buffers_with_damage) {
			ret = egl->procs.eglSwapBuffersWithDamageEXT(egl->display, surface, egl_damage,
				nrects);
		} else {
			ret = egl->procs.eglSwapBuffersWithDamageKHR(egl->display, surface, egl_damage,
				nrects);
		}
	} else {
		ret = eglSwapBuffers(egl->display, surface);
	}

	if (!ret) {
		wlf_log(WLF_ERROR, "swapchain_present failed: %s",
			wlf_egl_error_str(eglGetError()));
		return;
	}
}

static bool swapchain_resize(struct wlf_swapchain *swapchain, int width,
		int height) {
	struct wlf_egl_swapchain *egl_swapchain =
		wlf_egl_swapchain_from_swapchain(swapchain);
	struct wlf_egl_buffer *egl_buffer =
		wlf_egl_buffer_from_buffer(egl_swapchain->back);
	if (egl_buffer == NULL || !wlf_egl_buffer_resize(egl_buffer, width, height)) {
		return false;
	}

	swapchain->width = width;
	swapchain->height = height;
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
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_egl_swapchain");
		return NULL;
	}

#if WLF_HAS_LINUX_PLATFORM
	struct wlf_gles_renderer *gles_renderer =
		wlf_gles_renderer_from_renderer(window->state.renderer);
	if (!wlf_backend_is_wayland(window->state.backend)) {
		wlf_log(WLF_ERROR, "EGL swapchain requires a Wayland backend");
		free(swapchain);
		return NULL;
	}

	struct wl_surface *surface = wlf_window_native_handle(window);
	if (surface == NULL) {
		wlf_log(WLF_ERROR, "Wayland EGL swapchain requires wl_surface");
		free(swapchain);
		return NULL;
	}

	struct wlf_allocator *allocator =
		wlf_egl_allocator_create(gles_renderer->egl, surface);
	if (allocator == NULL) {
		free(swapchain);
		return NULL;
	}
#else
	free(swapchain);
	return NULL;
#endif

	wlf_swapchain_init(&swapchain->base, allocator, &swapchain_impl, width, height);
	swapchain->base.window = window;
	if (!wlf_render_format_copy(&swapchain->base.format, format)) {
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}

	swapchain->back = wlf_allocator_create_buffer(allocator, width, height,
		format);
	if (swapchain->back == NULL) {
		wlf_log(WLF_ERROR, "failed to create EGL back buffer");
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}
	swapchain->base.back = swapchain->back;

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
