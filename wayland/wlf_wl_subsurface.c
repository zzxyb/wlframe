#include "wlf/wayland/wlf_wl_subsurface.h"
#include "wlf/wayland/wlf_wl_surface.h"

#include <assert.h>
#include <stdlib.h>

#include <wayland-client-protocol.h>

void wlf_wl_subsurface_destroy(struct wlf_wl_subsurface *subsurface) {
	if (subsurface == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&subsurface->events.destroy, subsurface);
	assert(wlf_linked_list_empty(&subsurface->events.destroy.listener_list));

	if (subsurface->wl_subsurface != NULL) {
		wl_subsurface_destroy(subsurface->wl_subsurface);
	}

	free(subsurface);
}

void wlf_wl_subsurface_set_position(struct wlf_wl_subsurface *subsurface,
		int32_t x, int32_t y) {
	assert(subsurface != NULL);

	wl_subsurface_set_position(subsurface->wl_subsurface, x, y);
}

void wlf_wl_subsurface_place_above(struct wlf_wl_subsurface *subsurface,
		struct wlf_wl_surface *sibling) {
	assert(subsurface != NULL);
	assert(sibling != NULL);

	wl_subsurface_place_above(subsurface->wl_subsurface, sibling->wl_surface);
}

void wlf_wl_subsurface_place_below(struct wlf_wl_subsurface *subsurface,
		struct wlf_wl_surface *sibling) {
	assert(subsurface != NULL);
	assert(sibling != NULL);

	wl_subsurface_place_below(subsurface->wl_subsurface, sibling->wl_surface);
}

void wlf_wl_subsurface_set_sync(struct wlf_wl_subsurface *subsurface) {
	assert(subsurface != NULL);

	wl_subsurface_set_sync(subsurface->wl_subsurface);
}

void wlf_wl_subsurface_set_desync(struct wlf_wl_subsurface *subsurface) {
	assert(subsurface != NULL);

	wl_subsurface_set_desync(subsurface->wl_subsurface);
}
