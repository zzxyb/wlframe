#include "wlf/buffer/xcb-egl/buffer.h"

#include "wlf/renderer/gles/egl.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <limits.h>
#include <stdlib.h>

static void egl_buffer_release_surface(struct wlf_xcb_egl_buffer *buffer) {
	if (buffer->surface != EGL_NO_SURFACE) {
		eglDestroySurface(buffer->egl->display, buffer->surface);
		buffer->surface = EGL_NO_SURFACE;
	}
}

static void egl_buffer_destroy(struct wlf_buffer *base) {
	struct wlf_xcb_egl_buffer *buffer = wlf_xcb_egl_buffer_from_buffer(base);
	wlf_buffer_finish(base);
	egl_buffer_release_surface(buffer);
	free(buffer);
}

static const struct wlf_buffer_impl egl_buffer_impl = {
	.destroy = egl_buffer_destroy,
};

struct wlf_xcb_egl_buffer *wlf_xcb_egl_buffer_create(struct wlf_egl *egl,
		xcb_window_t window, uint32_t width, uint32_t height,
		const struct wlf_render_format *format) {
	if (egl == NULL || !egl->xcb_platform || window == 0 || format == NULL ||
			width == 0 || height == 0 || width > INT_MAX || height > INT_MAX ||
			egl->procs.eglCreatePlatformWindowSurfaceEXT == NULL) {
		return NULL;
	}
	struct wlf_xcb_egl_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}
	wlf_buffer_init(&buffer->base, &egl_buffer_impl, width, height);
	buffer->egl = egl;
	buffer->config = wlf_egl_choose_config(egl, format);
	buffer->surface = EGL_NO_SURFACE;
	if (buffer->config != NULL) {
		/* EGL_EXT_platform_xcb takes a pointer to xcb_window_t. */
		buffer->surface = egl->procs.eglCreatePlatformWindowSurfaceEXT(
			egl->display, buffer->config, &window, NULL);
	}
	if (buffer->surface == EGL_NO_SURFACE) {
		wlf_log(WLF_ERROR, "Failed to create XCB EGL surface: %s",
			wlf_egl_error_str(eglGetError()));
		wlf_buffer_drop(&buffer->base);
		return NULL;
	}
	return buffer;
}

bool wlf_buffer_is_xcb_egl(const struct wlf_buffer *buffer) {
	return buffer != NULL && buffer->impl == &egl_buffer_impl;
}

struct wlf_xcb_egl_buffer *wlf_xcb_egl_buffer_from_buffer(struct wlf_buffer *buffer) {
	assert(buffer->impl == &egl_buffer_impl);

	struct wlf_xcb_egl_buffer *egl_buffer =
		wlf_container_of(buffer, egl_buffer, base);
	return egl_buffer;
}

bool wlf_xcb_egl_buffer_resize(struct wlf_xcb_egl_buffer *buffer, uint32_t width, uint32_t height) {
	if (buffer == NULL || width == 0 || height == 0 || width > INT_MAX || height > INT_MAX) {
		return false;
	}

	buffer->base.width = width;
	buffer->base.height = height;
	return true;
}

struct wlf_egl *wlf_xcb_egl_buffer_get_egl(const struct wlf_xcb_egl_buffer *buffer) {
	return buffer != NULL ? buffer->egl : NULL;
}

EGLSurface wlf_xcb_egl_buffer_get_surface(const struct wlf_xcb_egl_buffer *buffer) {
	return buffer != NULL ? buffer->surface : EGL_NO_SURFACE;
}

bool wlf_xcb_egl_buffer_configure_swap_interval(struct wlf_xcb_egl_buffer *buffer) {
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
