#include "wlf/va/wlf_va_display.h"
#include "wlf/va/wayland/va_display.h"

#include <stdlib.h>
#include <assert.h>

struct wlf_va_display *wlf_va_display_autocreate(struct wlf_backend *backend) {
	// if (wlf_backend_is_wayland(backend)) {
	// 	return wl_va_display_create(wlf_backend_wayland_from_backend(backend));
	// }

	return NULL;
}

void wlf_va_display_init(struct wlf_va_display *display,
		const struct wlf_va_display_impl *impl, struct wlf_backend *backend) {
	assert(impl->destroy);

	*display = (struct wlf_va_display){
		.impl = impl,
	};

	display->backend = backend;

	wlf_signal_init(&display->events.destroy);
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
