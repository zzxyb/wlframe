#include "wlf/wayland/wlf_wl_seat.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void seat_handle_capabilities(void *data, struct wl_seat *base,
		uint32_t capabilities) {
	WLF_UNUSED(base);

	struct wlf_wl_seat *seat = data;
	seat->capabilities = capabilities;
	wlf_signal_emit_mutable(&seat->events.capabilities, seat);
}

static void seat_handle_name(void *data, struct wl_seat *base,
		const char *name) {
	WLF_UNUSED(base);

	struct wlf_wl_seat *seat = data;
	free(seat->name);
	seat->name = strdup(name);
	wlf_signal_emit_mutable(&seat->events.name, seat);
}

static const struct wl_seat_listener wl_seat_listener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

struct wlf_wl_seat *wlf_wl_seat_create(struct wl_registry *wl_registry,
		uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wl_seat *seat = calloc(1, sizeof(*seat));
	if (seat == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_seat");
		return NULL;
	}

	uint32_t bind_version = version;
	if (version > (uint32_t)wl_seat_interface.version) {
		bind_version = (uint32_t)wl_seat_interface.version;
	}

	seat->wl_seat = wl_registry_bind(wl_registry, name,
		&wl_seat_interface, bind_version);
	if (seat->wl_seat == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind wl_seat");
		free(seat);
		return NULL;
	}

	wlf_signal_init(&seat->events.destroy);
	wlf_signal_init(&seat->events.capabilities);
	wlf_signal_init(&seat->events.name);
	seat->version = bind_version;
	wl_seat_add_listener(seat->wl_seat, &wl_seat_listener, seat);

	return seat;
}

void wlf_wl_seat_destroy(struct wlf_wl_seat *seat) {
	if (seat == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&seat->events.destroy, seat);
	assert(wlf_linked_list_empty(&seat->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&seat->events.capabilities.listener_list));
	assert(wlf_linked_list_empty(&seat->events.name.listener_list));

	if (seat->wl_seat != NULL) {
		wl_seat_release(seat->wl_seat);
	}

	free(seat->name);
	free(seat);
}
