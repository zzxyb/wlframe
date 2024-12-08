#include "wlf/wayland/wlf_wl_pointer.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>

#include <wayland-client-protocol.h>

static const struct wl_pointer_listener pointer_listener = {
	// .enter = pointer_handle_enter,
	// .leave = pointer_handle_leave,
	// .motion = pointer_handle_motion,
	// .button = pointer_handle_button,
	// .axis = pointer_handle_axis,
	// .frame = pointer_handle_frame,
	// .axis_source = pointer_handle_axis_source,
	// .axis_stop = pointer_handle_axis_stop,
	// .axis_discrete = pointer_handle_axis_discrete,
	// .axis_value120 = pointer_handle_axis_value120,
	// .axis_relative_direction = pointer_handle_axis_relative_direction,
};

struct wlf_wl_pointer *create_wlf_wl_pointer(struct wl_pointer *pointer,
		struct wlf_wl_seat *seat) {
	struct wlf_wl_pointer *wlf_pointer = malloc(sizeof(struct wlf_wl_pointer));
	if (wlf_pointer == NULL) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_wl_pointer failed!");
		return NULL;
	}

	wlf_pointer->base = NULL;
	wlf_pointer->wl_pointer = pointer;
	wlf_pointer->seat = seat;
	wlf_pointer->axis_source = WL_POINTER_AXIS_SOURCE_WHEEL;
	wlf_pointer->axis_discrete = 0;
	wlf_pointer->fingers = 0;
	wlf_signal_init(&wlf_pointer->events.destroy);

	wl_pointer_add_listener(pointer, &pointer_listener, wlf_pointer);

	return wlf_pointer;
}

void wlf_wl_pointer_destroy(struct wlf_wl_pointer *pointer) {
	if (pointer == NULL) {
		return;
	}

	wlf_signal_emit(&pointer->events.destroy, pointer);

	if (pointer->wl_pointer) {
		wl_pointer_destroy(pointer->wl_pointer);
		pointer->wl_pointer = NULL;
	}

	free(pointer);
}
