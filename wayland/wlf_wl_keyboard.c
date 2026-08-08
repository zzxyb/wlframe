#include "wlf/wayland/wlf_wl_keyboard.h"
#include "wlf/wayland/wlf_wl_surface.h"
#include "wlf/types/wlf_keyboard.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static struct wlf_wl_keyboard *keyboard_from_data(void *data) {
	return (struct wlf_wl_keyboard *)data;
}

static void keyboard_handle_keymap(void *data, struct wl_keyboard *base,
		uint32_t format, int32_t fd, uint32_t size) {
	(void)base;
	struct wlf_wl_keyboard *keyboard = keyboard_from_data(data);
	struct wlf_keyboard_keymap_event event = {
		.keyboard = &keyboard->base,
		.format = (enum wlf_keyboard_keymap_format)format,
		.fd = fd,
		.size = size,
	};
	wlf_signal_emit_mutable(&keyboard->base.events.keymap, &event);
}

static void keyboard_handle_enter(void *data, struct wl_keyboard *base,
		uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
	(void)base;
	struct wlf_wl_keyboard *keyboard = keyboard_from_data(data);
	struct wlf_keyboard_enter_event event = {
		.keyboard = &keyboard->base,
		.serial = serial,
		.window = wlf_wl_surface_get_window(surface),
		.keys = keys->data,
		.keys_count = keys->size / sizeof(uint32_t),
	};
	wlf_signal_emit_mutable(&keyboard->base.events.enter, &event);
}

static void keyboard_handle_leave(void *data, struct wl_keyboard *base,
		uint32_t serial, struct wl_surface *surface) {
	(void)base;
	struct wlf_wl_keyboard *keyboard = keyboard_from_data(data);
	struct wlf_keyboard_leave_event event = {
		.keyboard = &keyboard->base,
		.serial = serial,
		.window = wlf_wl_surface_get_window(surface),
	};
	wlf_signal_emit_mutable(&keyboard->base.events.leave, &event);
}

static void keyboard_handle_key(void *data, struct wl_keyboard *base,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	(void)base;
	struct wlf_wl_keyboard *keyboard = keyboard_from_data(data);
	struct wlf_keyboard_key_event event = {
		.keyboard = &keyboard->base,
		.serial = serial,
		.time_msec = time,
		.key = key,
		.state = (enum wlf_keyboard_key_state)state,
	};
	wlf_signal_emit_mutable(&keyboard->base.events.key, &event);
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *base,
		uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
		uint32_t mods_locked, uint32_t group) {
	(void)base;
	struct wlf_wl_keyboard *keyboard = keyboard_from_data(data);
	struct wlf_keyboard_modifiers_event event = {
		.keyboard = &keyboard->base,
		.serial = serial,
		.mods_depressed = mods_depressed,
		.mods_latched = mods_latched,
		.mods_locked = mods_locked,
		.group = group,
	};
	wlf_signal_emit_mutable(&keyboard->base.events.modifiers, &event);
}

static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *base,
		int32_t rate, int32_t delay) {
	(void)base;
	struct wlf_wl_keyboard *keyboard = keyboard_from_data(data);
	struct wlf_keyboard_repeat_info_event event = {
		.keyboard = &keyboard->base,
		.rate = rate,
		.delay = delay,
	};
	wlf_signal_emit_mutable(&keyboard->base.events.repeat_info, &event);
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
	.keymap = keyboard_handle_keymap,
	.enter = keyboard_handle_enter,
	.leave = keyboard_handle_leave,
	.key = keyboard_handle_key,
	.modifiers = keyboard_handle_modifiers,
	.repeat_info = keyboard_handle_repeat_info,
};

static void keyboard_impl_destroy(struct wlf_keyboard *base) {
	struct wlf_wl_keyboard *keyboard = wlf_wl_keyboard_from_base(base);
	wl_keyboard_release(keyboard->wl_keyboard);
	free(keyboard);
}

static const struct wlf_keyboard_impl wlf_wl_keyboard_impl = {
	.name = "wl_keyboard",
	.destroy = keyboard_impl_destroy,
};

struct wlf_wl_keyboard *wlf_wl_keyboard_create(struct wl_seat *seat) {
	assert(seat != NULL);

	struct wlf_wl_keyboard *keyboard = calloc(1, sizeof(*keyboard));
	if (keyboard == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_keyboard");
		return NULL;
	}

	keyboard->wl_keyboard = wl_seat_get_keyboard(seat);
	if (keyboard->wl_keyboard == NULL) {
		wlf_log(WLF_ERROR, "wl_seat_get_keyboard failed");
		free(keyboard);
		return NULL;
	}

	wlf_keyboard_init(&keyboard->base, &wlf_wl_keyboard_impl);

	wl_keyboard_add_listener(keyboard->wl_keyboard, &wl_keyboard_listener, keyboard);

	return keyboard;
}

void wlf_wl_keyboard_destroy(struct wlf_wl_keyboard *keyboard) {
	if (keyboard == NULL) {
		return;
	}
	wlf_keyboard_destroy(&keyboard->base);
}

struct wlf_wl_keyboard *wlf_wl_keyboard_from_base(struct wlf_keyboard *base) {
	assert(base != NULL);
	assert(base->impl == &wlf_wl_keyboard_impl);
	return (struct wlf_wl_keyboard *)base;
}
