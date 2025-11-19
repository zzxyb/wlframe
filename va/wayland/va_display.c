#include "wlf/va/wayland/va_display.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <wayland-client-protocol.h>

#include <va/va.h>
#include <va/va_wayland.h>

static void wl_va_display_destroy(struct wlf_va_display *display) {
	struct wlf_wl_va_display *wl_display = wlf_wl_va_display_from_va_display(display);
	wlf_signal_emit(&display->events.destroy, display);

	if (display->display != NULL) {
		vaTerminate(display->display);
	}

	free(wl_display);
}

static VAStatus wl_va_display_put_surface(struct wlf_va_display *display,
		VASurfaceID va_surface, void *native_surface,
		const VARectangle *src_rect, const VARectangle *dst_rect) {
	VAStatus va_status;
	struct wl_buffer *buffer;
	struct wl_surface *surface = native_surface;
	va_status = vaGetSurfaceBufferWl(display->display, va_surface, VA_FRAME_PICTURE, &buffer);
	if (va_status != VA_STATUS_SUCCESS) {
		return va_status;
	}

	wl_surface_attach(surface, buffer, 0, 0);
	wl_surface_damage(
		surface,
		dst_rect->x, dst_rect->y, dst_rect->width, dst_rect->height
	);

	wl_surface_commit(surface);
	struct wlf_backend_wayland *wl_backend = wlf_backend_wayland_from_backend(display->backend);
	wl_display_flush(wl_backend->display->base);

	return VA_STATUS_SUCCESS;
}

static const struct wlf_va_display_impl wl_va_display_impl = {
	.name = "wayland",
	.destroy = wl_va_display_destroy,
	.put_surface = wl_va_display_put_surface,
};

struct wlf_va_display *wl_va_display_create(struct wlf_backend_wayland *backend) {
	VAStatus va_status;
	int major_version, minor_version;
	const char *driver;
	struct wlf_wl_va_display *display = calloc(1, sizeof(*display));
	if (display == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_wl_va_display");
		return NULL;
	}

	display->base.display = vaGetDisplayWl(backend->display->base);
	if (display->base.display == NULL) {
		goto error;
	}

	va_status = vaInitialize(display->base.display, &major_version, &minor_version);
	wlf_log(WLF_INFO, "wlframe: VA-API version: %d.%d", major_version, minor_version);
	if (va_status != VA_STATUS_SUCCESS) {
		goto error;
	}

	driver = vaQueryVendorString(display->base.display);
	wlf_log(WLF_INFO, "wlframe: Driver version: %s\n", driver ? driver : "<unknown>");

	wlf_va_display_init(&display->base, &wl_va_display_impl, &backend->base);

	return &display->base;

error:
	wlf_va_display_destroy(&display->base);

	return NULL;
}

bool wlf_va_display_is_wayland(const struct wlf_va_display *display) {
	return (display && display->impl == &wl_va_display_impl);
}

struct wlf_wl_va_display *wlf_wl_va_display_from_va_display(struct wlf_va_display *display) {
	assert(display && display->impl == &wl_va_display_impl);
	struct wlf_wl_va_display *wl_display = wlf_container_of(display, wl_display, base);

	return wl_display;
}
