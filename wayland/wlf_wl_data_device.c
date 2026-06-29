#include "wlf/wayland/wlf_wl_data_device.h"
#include "wlf/wayland/wlf_wl_data_device_manager.h"
#include "wlf/wayland/wlf_wl_data_offer.h"
#include "wlf/wayland/wlf_wl_data_source.h"
#include "wlf/wayland/wlf_wl_seat.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void data_device_handle_data_offer(void *data,
		struct wl_data_device *base, struct wl_data_offer *id) {
	(void)base;
	struct wlf_wl_data_device *device = data;

	struct wlf_wl_data_offer *offer = wlf_wl_data_offer_wrap(id);
	if (offer == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&device->events.data_offer, offer);
}

static void data_device_handle_enter(void *data,
		struct wl_data_device *base, uint32_t serial,
		struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y,
		struct wl_data_offer *id) {
	(void)base;
	struct wlf_wl_data_device *device = data;

	device->drag_offer = id ? wl_data_offer_get_user_data(id) : NULL;

	struct wlf_wl_data_device_enter_event event = {
		.device = device,
		.serial = serial,
		.surface = surface,
		.x = wl_fixed_to_double(x),
		.y = wl_fixed_to_double(y),
		.offer = device->drag_offer,
	};
	wlf_signal_emit_mutable(&device->events.enter, &event);
}

static void data_device_handle_leave(void *data,
		struct wl_data_device *base) {
	(void)base;
	struct wlf_wl_data_device *device = data;
	device->drag_offer = NULL;
	wlf_signal_emit_mutable(&device->events.leave, device);
}

static void data_device_handle_motion(void *data,
		struct wl_data_device *base, uint32_t time,
		wl_fixed_t x, wl_fixed_t y) {
	(void)base;
	struct wlf_wl_data_device *device = data;
	struct wlf_wl_data_device_motion_event event = {
		.device = device,
		.time_msec = time,
		.x = wl_fixed_to_double(x),
		.y = wl_fixed_to_double(y),
	};
	wlf_signal_emit_mutable(&device->events.motion, &event);
}

static void data_device_handle_drop(void *data,
		struct wl_data_device *base) {
	(void)base;
	struct wlf_wl_data_device *device = data;
	wlf_signal_emit_mutable(&device->events.drop, device);
}

static void data_device_handle_selection(void *data,
		struct wl_data_device *base, struct wl_data_offer *id) {
	(void)base;
	struct wlf_wl_data_device *device = data;

	device->selection_offer = id ? wl_data_offer_get_user_data(id) : NULL;

	struct wlf_wl_data_device_selection_event event = {
		.device = device,
		.offer = device->selection_offer,
	};
	wlf_signal_emit_mutable(&device->events.selection, &event);
}

static const struct wl_data_device_listener wl_data_device_listener = {
	.data_offer = data_device_handle_data_offer,
	.enter = data_device_handle_enter,
	.leave = data_device_handle_leave,
	.motion = data_device_handle_motion,
	.drop = data_device_handle_drop,
	.selection = data_device_handle_selection,
};

struct wlf_wl_data_device *wlf_wl_data_device_create(
		struct wlf_wl_data_device_manager *manager,
		struct wlf_wl_seat *seat) {
	assert(manager != NULL);
	assert(seat != NULL);

	struct wlf_wl_data_device *device = calloc(1, sizeof(*device));
	if (device == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_data_device");
		return NULL;
	}

	device->wl_data_device = wl_data_device_manager_get_data_device(
		manager->wl_data_device_manager, seat->wl_seat);
	if (device->wl_data_device == NULL) {
		wlf_log(WLF_ERROR, "wl_data_device_manager_get_data_device failed");
		free(device);
		return NULL;
	}

	wlf_signal_init(&device->events.destroy);
	wlf_signal_init(&device->events.data_offer);
	wlf_signal_init(&device->events.enter);
	wlf_signal_init(&device->events.leave);
	wlf_signal_init(&device->events.motion);
	wlf_signal_init(&device->events.drop);
	wlf_signal_init(&device->events.selection);

	wl_data_device_add_listener(device->wl_data_device,
		&wl_data_device_listener, device);

	return device;
}

void wlf_wl_data_device_destroy(struct wlf_wl_data_device *device) {
	if (device == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&device->events.destroy, device);
	wl_data_device_release(device->wl_data_device);
	free(device);
}

void wlf_wl_data_device_start_drag(struct wlf_wl_data_device *device,
		struct wlf_wl_data_source *source,
		struct wl_surface *origin,
		struct wl_surface *icon,
		uint32_t serial) {
	assert(device != NULL);
	struct wl_data_source *wl_src = source ? source->wl_data_source : NULL;
	wl_data_device_start_drag(device->wl_data_device, wl_src,
		origin, icon, serial);
}

void wlf_wl_data_device_set_selection(struct wlf_wl_data_device *device,
		struct wlf_wl_data_source *source, uint32_t serial) {
	assert(device != NULL);
	struct wl_data_source *wl_src = source ? source->wl_data_source : NULL;
	wl_data_device_set_selection(device->wl_data_device, wl_src, serial);
}
