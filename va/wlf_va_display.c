#include "wlf/va/wlf_va_display.h"
#include "wlf/va/wayland/va_display.h"

#include <stdlib.h>
#include <assert.h>

static bool validate_rect(const VARectangle *rect)
{
	return (rect && rect->x >= 0 && rect->y >= 0 && rect->width > 0 && rect->height > 0);
}

struct wlf_va_display *wlf_va_display_autocreate(struct wlf_backend *backend) {
	if (wlf_backend_is_wayland(backend)) {
		return wl_va_display_create(wlf_backend_wayland_from_backend(backend));
	}

	return NULL;
}

void wlf_va_display_init(struct wlf_va_display *buffer,
		const struct wlf_va_display_impl *impl, struct wlf_backend *backend) {
	assert(impl->destroy);

	*buffer = (struct wlf_va_display){
		.impl = impl,
	};

	buffer->backend = backend;

	wlf_signal_init(&buffer->events.destroy);
}

void wlf_va_display_destroy(struct wlf_va_display *display) {
	if (!display) {
		return;
	}

	wlf_signal_emit(&display->events.destroy, display);
	vaTerminate(display->display);
	if (display->impl && display->impl->destroy) {
		display->impl->destroy(display);
	} else {
		free(display);
	}
}

VAStatus wlf_va_display_put_surface(struct wlf_va_display *display,
		VASurfaceID va_surface, void *native_surface,
		const VARectangle *src_rect, const VARectangle *dst_rect) {
	if (display->display) {
		return VA_STATUS_ERROR_INVALID_DISPLAY;
	}

	if (va_surface == VA_INVALID_SURFACE) {
		return VA_STATUS_ERROR_INVALID_SURFACE;
	}

	if (!validate_rect(src_rect) || !validate_rect(dst_rect)) {
		return VA_STATUS_ERROR_INVALID_PARAMETER;
	}

	if (display->impl && display->impl->put_surface) {
		return display->impl->put_surface(display, va_surface, native_surface, src_rect, dst_rect);
	}

	return VA_STATUS_ERROR_OPERATION_FAILED;
}
