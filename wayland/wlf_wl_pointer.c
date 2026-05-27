#include "wlf/wayland/wlf_wl_pointer.h"
#include "wlf/types/wlf_pointer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <wayland-client-protocol.h>

static void pointer_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface,
		wl_fixed_t surface_x, wl_fixed_t surface_y) {
	struct wlf_wl_pointer *pointer = data;
	pointer->focus_surface = surface;
	pointer->enter_serial = serial;
}

static void pointer_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	struct wlf_wl_pointer *pointer = data;
	pointer->focus_surface = NULL;
	pointer->enter_serial = 0;
}

static void pointer_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	struct wlf_wl_pointer *pointer = data;

	struct wlf_pointer_motion_absolute_event event = {
		.pointer = &pointer->base,
		.time_msec = time,
		.x = wl_fixed_to_double(surface_x),
		.y = wl_fixed_to_double(surface_y),
	};
	wlf_signal_emit_mutable(&pointer->base.events.motion_absolute, &event);
}

static void pointer_button(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	struct wlf_wl_pointer *pointer = data;

	enum wlf_pointer_button_state btn_state;
	if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
		btn_state = WLF_POINTER_BUTTON_STATE_PRESSED;
	} else {
		btn_state = WLF_POINTER_BUTTON_STATE_RELEASED;
	}

	struct wlf_pointer_button_event event = {
		.pointer = &pointer->base,
		.time_msec = time,
		.button = button,
		.state = btn_state,
	};
	wlf_signal_emit_mutable(&pointer->base.events.button, &event);
}

static void pointer_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	struct wlf_wl_pointer *pointer = data;

	uint32_t idx = (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
		? WLF_POINTER_AXIS_VERTICAL_SCROLL
		: WLF_POINTER_AXIS_HORIZONTAL_SCROLL;

	pointer->frame_axes[idx].valid = true;
	pointer->frame_axes[idx].time_msec = time;
	pointer->frame_axes[idx].value += wl_fixed_to_double(value);
}

static void pointer_frame(void *data, struct wl_pointer *wl_pointer) {
	struct wlf_wl_pointer *pointer = data;

	for (uint32_t i = 0; i < 2; i++) {
		struct wlf_wl_pointer_axis_frame *af = &pointer->frame_axes[i];
		if (!af->valid && !af->stop) {
			continue;
		}

		struct wlf_pointer_axis_event event = {
			.pointer = &pointer->base,
			.time_msec = af->time_msec,
			.orientation = (enum wlf_pointer_axis)i,
			.delta = af->stop ? 0.0 : af->value,
			.delta_discrete = 0,
			.relative_direction = af->has_direction
				? af->direction
				: WLF_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL,
		};

		if (pointer->frame_has_source) {
			event.source = pointer->frame_source;
		}

		if (af->has_value120) {
			event.delta_discrete = af->value120;
		}

		wlf_signal_emit_mutable(&pointer->base.events.axis, &event);
	}

	wlf_signal_emit_mutable(&pointer->base.events.frame, NULL);

	/* Reset accumulated frame state. */
	memset(pointer->frame_axes, 0, sizeof(pointer->frame_axes));
	pointer->frame_has_source = false;
}

static void pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
		uint32_t axis_source) {
	struct wlf_wl_pointer *pointer = data;

	pointer->frame_has_source = true;
	switch (axis_source) {
	case WL_POINTER_AXIS_SOURCE_WHEEL:
		pointer->frame_source = WLF_POINTER_AXIS_SOURCE_WHEEL;
		break;
	case WL_POINTER_AXIS_SOURCE_FINGER:
		pointer->frame_source = WLF_POINTER_AXIS_SOURCE_FINGER;
		break;
	case WL_POINTER_AXIS_SOURCE_CONTINUOUS:
		pointer->frame_source = WLF_POINTER_AXIS_SOURCE_CONTINUOUS;
		break;
	case WL_POINTER_AXIS_SOURCE_WHEEL_TILT:
		pointer->frame_source = WLF_POINTER_AXIS_SOURCE_WHEEL_TILT;
		break;
	default:
		pointer->frame_has_source = false;
		break;
	}
}

static void pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis) {
	struct wlf_wl_pointer *pointer = data;

	uint32_t idx = (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
		? WLF_POINTER_AXIS_VERTICAL_SCROLL
		: WLF_POINTER_AXIS_HORIZONTAL_SCROLL;

	pointer->frame_axes[idx].stop = true;
	pointer->frame_axes[idx].time_msec = time;
}

static void pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
		uint32_t axis, int32_t discrete) {
	struct wlf_wl_pointer *pointer = data;

	uint32_t idx = (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
		? WLF_POINTER_AXIS_VERTICAL_SCROLL
		: WLF_POINTER_AXIS_HORIZONTAL_SCROLL;

	/* axis_discrete is deprecated in v8; value120 takes precedence if present. */
	if (!pointer->frame_axes[idx].has_value120) {
		pointer->frame_axes[idx].value120 = discrete * 120;
	}
}

static void pointer_axis_value120(void *data, struct wl_pointer *wl_pointer,
		uint32_t axis, int32_t value120) {
	struct wlf_wl_pointer *pointer = data;

	uint32_t idx = (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
		? WLF_POINTER_AXIS_VERTICAL_SCROLL
		: WLF_POINTER_AXIS_HORIZONTAL_SCROLL;

	pointer->frame_axes[idx].has_value120 = true;
	pointer->frame_axes[idx].value120 = value120;
}

static void pointer_axis_relative_direction(void *data, struct wl_pointer *wl_pointer,
		uint32_t axis, uint32_t direction) {
	struct wlf_wl_pointer *pointer = data;

	uint32_t idx = (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
		? WLF_POINTER_AXIS_VERTICAL_SCROLL
		: WLF_POINTER_AXIS_HORIZONTAL_SCROLL;

	pointer->frame_axes[idx].has_direction = true;
	pointer->frame_axes[idx].direction =
		(direction == WL_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED)
		? WLF_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED
		: WLF_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL;
}

static const struct wl_pointer_listener wl_pointer_listener = {
	.enter = pointer_enter,
	.leave = pointer_leave,
	.motion = pointer_motion,
	.button = pointer_button,
	.axis = pointer_axis,
	.frame = pointer_frame,
	.axis_source = pointer_axis_source,
	.axis_stop = pointer_axis_stop,
	.axis_discrete = pointer_axis_discrete,
	.axis_value120 = pointer_axis_value120,
	.axis_relative_direction = pointer_axis_relative_direction,
};

static void pointer_destroy(struct wlf_pointer *base) {
	struct wlf_wl_pointer *pointer = wlf_wl_pointer_from_pointer(base);
	wl_pointer_release(pointer->pointer);
	free(pointer);
}

static const struct wlf_pointer_impl pointer_impl = {
	.name = "wl_pointer",
	.destroy = pointer_destroy,
};

struct wlf_pointer *wlf_wl_pointer_create(struct wl_seat *seat) {
	assert(seat != NULL);

	struct wlf_wl_pointer *pointer = calloc(1, sizeof(*pointer));
	if (pointer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_pointer");
		return NULL;
	}

	pointer->pointer = wl_seat_get_pointer(seat);
	if (pointer->pointer == NULL) {
		wlf_log(WLF_ERROR, "Failed to get wl_pointer from wl_seat");
		free(pointer);
		return NULL;
	}

	wl_pointer_add_listener(pointer->pointer, &wl_pointer_listener, pointer);
	wlf_pointer_init(&pointer->base, &pointer_impl);

	return &pointer->base;
}

bool wlf_pointer_is_wayland(const struct wlf_pointer *pointer) {
	return pointer->impl == &pointer_impl;
}

struct wlf_wl_pointer *wlf_wl_pointer_from_pointer(struct wlf_pointer *pointer) {
	assert(pointer->impl == &pointer_impl);

	struct wlf_wl_pointer *wl_pointer = wlf_container_of(pointer, wl_pointer, base);
	return wl_pointer;
}
