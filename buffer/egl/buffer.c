#include "wlf/buffer/egl/buffer.h"

#include "wlf/renderer/gles/egl.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <wayland-egl-core.h>

#include <limits.h>
#include <stdlib.h>

static bool egl_supports_platform_window_surface(const struct wlf_egl *egl) {
	return egl->exts.EXT_platform_base &&
		(egl->exts.platform.wayland.EXT_platform_wayland ||
			egl->exts.platform.wayland.KHR_platform_wayland);
}

static void egl_buffer_release_surface(struct wlf_egl_buffer *buffer) {
	if (buffer->surface != EGL_NO_SURFACE) {
		eglDestroySurface(buffer->egl->display, buffer->surface);
		buffer->surface = EGL_NO_SURFACE;
	}

	if (buffer->egl_window != NULL) {
		wl_egl_window_destroy(buffer->egl_window);
		buffer->egl_window = NULL;
	}
}

static void egl_buffer_destroy(struct wlf_buffer *base) {
	struct wlf_egl_buffer *buffer = wlf_egl_buffer_from_buffer(base);
	wlf_buffer_finish(base);
	egl_buffer_release_surface(buffer);
	free(buffer);
}

static const struct wlf_buffer_impl egl_buffer_impl = {
	.destroy = egl_buffer_destroy,
};

struct wlf_egl_buffer *wlf_egl_buffer_create(struct wlf_egl *egl,
		struct wl_surface *surface, uint32_t width, uint32_t height,
		const struct wlf_render_format *format) {
	if (egl == NULL || surface == NULL || format == NULL || width == 0 ||
			height == 0 || width > INT_MAX || height > INT_MAX) {
		return NULL;
	}

	struct wlf_egl_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_egl_buffer");
		return NULL;
	}

	wlf_buffer_init(&buffer->base, &egl_buffer_impl, width, height);
	buffer->egl = egl;
	buffer->surface = EGL_NO_SURFACE;

	buffer->config = wlf_egl_choose_config(egl, format);
	if (buffer->config == NULL) {
		goto error;
	}

	buffer->egl_window = wl_egl_window_create(surface, (int)width, (int)height);
	if (buffer->egl_window == NULL) {
		wlf_log(WLF_ERROR, "Failed to create wl_egl_window");
		goto error;
	}

	const EGLint platform_surface_attribs[] = {
		EGL_NONE,
	};
	const EGLint window_surface_attribs[] = {
		EGL_NONE,
	};

	if (egl_supports_platform_window_surface(egl)) {
		buffer->surface = egl->procs.eglCreatePlatformWindowSurfaceEXT(
			egl->display, buffer->config, buffer->egl_window,
			platform_surface_attribs);
	} else {
		buffer->surface = eglCreateWindowSurface(egl->display, buffer->config,
			(EGLNativeWindowType)buffer->egl_window, window_surface_attribs);
	}
	if (buffer->surface == EGL_NO_SURFACE) {
		wlf_log(WLF_ERROR, "Failed to create EGL surface: %s",
			wlf_egl_error_str(eglGetError()));
		goto error;
	}

	return buffer;

error:
	egl_buffer_release_surface(buffer);
	wlf_buffer_finish(&buffer->base);
	free(buffer);
	return NULL;
}

bool wlf_buffer_is_egl(const struct wlf_buffer *buffer) {
	return buffer != NULL && buffer->impl == &egl_buffer_impl;
}

struct wlf_egl_buffer *wlf_egl_buffer_from_buffer(struct wlf_buffer *buffer) {
	assert(buffer->impl == &egl_buffer_impl);

	struct wlf_egl_buffer *egl_buffer =
		wlf_container_of(buffer, egl_buffer, base);
	return egl_buffer;
}

bool wlf_egl_buffer_resize(struct wlf_egl_buffer *buffer, uint32_t width, uint32_t height) {
	if (buffer == NULL || buffer->egl_window == NULL) {
		return false;
	}

	wl_egl_window_resize(buffer->egl_window, (int)width, (int)height, 0, 0);
	buffer->base.width = width;
	buffer->base.height = height;
	return true;
}

struct wlf_egl *wlf_egl_buffer_get_egl(const struct wlf_egl_buffer *buffer) {
	return buffer != NULL ? buffer->egl : NULL;
}

EGLSurface wlf_egl_buffer_get_surface(const struct wlf_egl_buffer *buffer) {
	return buffer != NULL ? buffer->surface : EGL_NO_SURFACE;
}

bool wlf_egl_buffer_configure_swap_interval(struct wlf_egl_buffer *buffer) {
	if (buffer == NULL || buffer->egl == NULL ||
			buffer->surface == EGL_NO_SURFACE) {
		return false;
	}
	if (buffer->swap_interval_configured) {
		return true;
	}

	if (!eglSwapInterval(buffer->egl->display, 0)) {
		wlf_log(WLF_ERROR, "failed to disable EGL swap throttling: %s",
			wlf_egl_error_str(eglGetError()));
		return false;
	}

	buffer->swap_interval_configured = true;
	return true;
}
