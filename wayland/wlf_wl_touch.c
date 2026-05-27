#include "wlf/wayland/wlf_wl_touch.h"
#include "wlf/types/wlf_touch.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <wayland-client-protocol.h>

static void touch_down(void *data, struct wl_touch *wl_touch,
		uint32_t serial, uint32_t time, struct wl_surface *surface,
		int32_t id, wl_fixed_t x, wl_fixed_t y) {
	struct wlf_wl_touch *touch = data;

	struct wlf_touch_down_event event = {
		.touch = &touch->base,
		.surface = surface,
		.time_msec = time,
		.touch_id = id,
		.x = wl_fixed_to_double(x),
		.y = wl_fixed_to_double(y),
	};
	wlf_signal_emit_mutable(&touch->base.events.down, &event);
}

static void touch_up(void *data, struct wl_touch *wl_touch,
		uint32_t serial, uint32_t time, int32_t id) {
	struct wlf_wl_touch *touch = data;

	struct wlf_touch_up_event event = {
		.touch = &touch->base,
		.time_msec = time,
		.touch_id = id,
	};
	wlf_signal_emit_mutable(&touch->base.events.up, &event);
}

static void touch_motion(void *data, struct wl_touch *wl_touch,
		uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y) {
	struct wlf_wl_touch *touch = data;

	struct wlf_touch_motion_event event = {
		.touch = &touch->base,
		.time_msec = time,
		.touch_id = id,
		.x = wl_fixed_to_double(x),
		.y = wl_fixed_to_double(y),
	};
	wlf_signal_emit_mutable(&touch->base.events.motion, &event);
}

static void touch_frame(void *data, struct wl_touch *wl_touch) {
	struct wlf_wl_touch *touch = data;
	wlf_signal_emit_mutable(&touch->base.events.frame, NULL);
}

static void touch_cancel(void *data, struct wl_touch *wl_touch) {
	struct wlf_wl_touch *touch = data;

	struct wlf_touch_cancel_event event = {
		.touch = &touch->base,
	};
	wlf_signal_emit_mutable(&touch->base.events.cancel, &event);
}

static void touch_shape(void *data, struct wl_touch *wl_touch,
		int32_t id, wl_fixed_t major, wl_fixed_t minor) {
	struct wlf_wl_touch *touch = data;

	struct wlf_touch_shape_event event = {
		.touch = &touch->base,
		.touch_id = id,
		.major = wl_fixed_to_double(major),
		.minor = wl_fixed_to_double(minor),
	};
	wlf_signal_emit_mutable(&touch->base.events.shape, &event);
}

static void touch_orientation(void *data, struct wl_touch *wl_touch,
		int32_t id, wl_fixed_t orientation) {
	struct wlf_wl_touch *touch = data;

	struct wlf_touch_orientation_event event = {
		.touch = &touch->base,
		.touch_id = id,
		.orientation = wl_fixed_to_double(orientation),
	};
	wlf_signal_emit_mutable(&touch->base.events.orientation, &event);
}

static const struct wl_touch_listener wl_touch_listener = {
	.down = touch_down,
	.up = touch_up,
	.motion = touch_motion,
	.frame = touch_frame,
	.cancel = touch_cancel,
	.shape = touch_shape,
	.orientation = touch_orientation,
};

static void touch_destroy(struct wlf_touch *base) {
	struct wlf_wl_touch *touch = wlf_wl_touch_from_touch(base);
	wl_touch_release(touch->touch);
	free(touch);
}

static const struct wlf_touch_impl touch_impl = {
	.name = "wl_touch",
	.destroy = touch_destroy,
};

struct wlf_touch *wlf_wl_touch_create(struct wl_seat *seat) {
	assert(seat != NULL);

	struct wlf_wl_touch *touch = calloc(1, sizeof(*touch));
	if (touch == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_touch");
		return NULL;
	}

	touch->touch = wl_seat_get_touch(seat);
	if (touch->touch == NULL) {
		wlf_log(WLF_ERROR, "Failed to get wl_touch from wl_seat");
		free(touch);
		return NULL;
	}

	wl_touch_add_listener(touch->touch, &wl_touch_listener, touch);
	wlf_touch_init(&touch->base, &touch_impl);

	return &touch->base;
}

bool wlf_touch_is_wayland(const struct wlf_touch *touch) {
	return touch->impl == &touch_impl;
}

struct wlf_wl_touch *wlf_wl_touch_from_touch(struct wlf_touch *touch) {
	assert(touch->impl == &touch_impl);

	struct wlf_wl_touch *wl_touch = wlf_container_of(touch, wl_touch, base);

	return wl_touch;
}
