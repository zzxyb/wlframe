#include "wlf/wayland/wlf_wl_seat.h"
#include "wlf/wayland/wlf_wl_keyboard.h"
#include "wlf/wayland/wlf_wl_pointer.h"
#include "wlf/wayland/wlf_wl_surface.h"
#include "wlf/wayland/wlf_wl_touch.h"
#include "wlf/types/wlf_keyboard.h"
#include "wlf/types/wlf_pointer.h"
#include "wlf/types/wlf_touch.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/window/wlf_window.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

#define SEAT_FROM_LISTENER(listener, member) \
	wlf_container_of(listener, seat, listeners.member)

static void handle_pointer_window_destroy(struct wlf_listener *listener,
		void *data) {
	(void)data;
	struct wlf_wl_seat *seat =
		SEAT_FROM_LISTENER(listener, pointer_window_destroy);
	wlf_linked_list_remove(&listener->link);
	seat->pointer_window = NULL;
}

static void handle_keyboard_window_destroy(struct wlf_listener *listener,
		void *data) {
	(void)data;
	struct wlf_wl_seat *seat =
		SEAT_FROM_LISTENER(listener, keyboard_window_destroy);
	wlf_linked_list_remove(&listener->link);
	seat->keyboard_window = NULL;
}

static void handle_touch_window_destroy(struct wlf_listener *listener,
		void *data) {
	(void)data;
	struct wlf_wl_seat *seat =
		SEAT_FROM_LISTENER(listener, touch_window_destroy);
	wlf_linked_list_remove(&listener->link);
	seat->touch_window = NULL;
}

static void seat_set_pointer_window(struct wlf_wl_seat *seat,
		struct wlf_window *window) {
	if (seat->pointer_window == window) {
		return;
	}
	if (seat->pointer_window != NULL) {
		wlf_linked_list_remove(&seat->listeners.pointer_window_destroy.link);
	}
	seat->pointer_window = window;
	if (window != NULL) {
		seat->listeners.pointer_window_destroy.notify =
			handle_pointer_window_destroy;
		wlf_signal_add(&window->events.destroy,
			&seat->listeners.pointer_window_destroy);
	}
}

static void seat_set_keyboard_window(struct wlf_wl_seat *seat,
		struct wlf_window *window) {
	if (seat->keyboard_window == window) {
		return;
	}
	if (seat->keyboard_window != NULL) {
		wlf_linked_list_remove(&seat->listeners.keyboard_window_destroy.link);
	}
	seat->keyboard_window = window;
	if (window != NULL) {
		seat->listeners.keyboard_window_destroy.notify =
			handle_keyboard_window_destroy;
		wlf_signal_add(&window->events.destroy,
			&seat->listeners.keyboard_window_destroy);
	}
}

static void seat_set_touch_window(struct wlf_wl_seat *seat,
		struct wlf_window *window) {
	if (seat->touch_window == window) {
		return;
	}
	if (seat->touch_window != NULL) {
		wlf_linked_list_remove(&seat->listeners.touch_window_destroy.link);
	}
	seat->touch_window = window;
	if (window != NULL) {
		seat->listeners.touch_window_destroy.notify = handle_touch_window_destroy;
		wlf_signal_add(&window->events.destroy,
			&seat->listeners.touch_window_destroy);
	}
}

static void handle_pointer_enter(struct wlf_listener *listener, void *data) {
	struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, pointer_enter);
	struct wlf_pointer_enter_event *event = data;
	seat_set_pointer_window(seat,
		wlf_wl_surface_get_window(event->surface));
	if (seat->pointer_window != NULL) {
		wlf_window_pointer_enter(seat->pointer_window, event);
	}
}

static void handle_pointer_leave(struct wlf_listener *listener, void *data) {
	struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, pointer_leave);
	if (seat->pointer_window != NULL) {
		wlf_window_pointer_leave(seat->pointer_window, data);
	}
	seat_set_pointer_window(seat, NULL);
}

static void handle_pointer_motion(struct wlf_listener *listener, void *data) {
	struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, pointer_motion);
	if (seat->pointer_window != NULL) {
		wlf_window_pointer_motion(seat->pointer_window, data);
	}
}

static void handle_pointer_button(struct wlf_listener *listener, void *data) {
	struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, pointer_button);
	if (seat->pointer_window != NULL) {
		wlf_window_pointer_button(seat->pointer_window, data);
	}
}

static void handle_pointer_axis(struct wlf_listener *listener, void *data) {
	struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, pointer_axis);
	if (seat->pointer_window != NULL) {
		wlf_window_pointer_axis(seat->pointer_window, data);
	}
}

static void handle_pointer_frame(struct wlf_listener *listener, void *data) {
	struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, pointer_frame);
	if (seat->pointer_window != NULL) {
		wlf_window_pointer_frame(seat->pointer_window, data);
	}
}

static void handle_keyboard_enter(struct wlf_listener *listener, void *data) {
	struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, keyboard_enter);
	struct wlf_keyboard_enter_event *event = data;
	seat_set_keyboard_window(seat, event->window);
	if (seat->keyboard_window != NULL) {
		wlf_window_keyboard_enter(seat->keyboard_window, event);
	}
}

static void handle_keyboard_leave(struct wlf_listener *listener, void *data) {
	struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, keyboard_leave);
	if (seat->keyboard_window != NULL) {
		wlf_window_keyboard_leave(seat->keyboard_window, data);
	}
	seat_set_keyboard_window(seat, NULL);
}

#define FORWARD_KEYBOARD(name) \
	static void handle_keyboard_##name(struct wlf_listener *listener, void *data) { \
		struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, keyboard_##name); \
		if (seat->keyboard_window != NULL) { \
			wlf_window_keyboard_##name(seat->keyboard_window, data); \
		} \
	}

FORWARD_KEYBOARD(keymap)
FORWARD_KEYBOARD(key)
FORWARD_KEYBOARD(modifiers)
FORWARD_KEYBOARD(repeat_info)

static void handle_touch_down(struct wlf_listener *listener, void *data) {
	struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, touch_down);
	struct wlf_touch_down_event *event = data;
	seat_set_touch_window(seat,
		wlf_wl_surface_get_window(event->surface));
	if (seat->touch_window != NULL) {
		wlf_window_touch_down(seat->touch_window, event);
	}
}

#define FORWARD_TOUCH(name) \
	static void handle_touch_##name(struct wlf_listener *listener, void *data) { \
		struct wlf_wl_seat *seat = SEAT_FROM_LISTENER(listener, touch_##name); \
		if (seat->touch_window != NULL) { \
			wlf_window_touch_##name(seat->touch_window, data); \
		} \
	}

FORWARD_TOUCH(up)
FORWARD_TOUCH(motion)
FORWARD_TOUCH(cancel)
FORWARD_TOUCH(frame)
FORWARD_TOUCH(shape)
FORWARD_TOUCH(orientation)

static void seat_destroy_pointer(struct wlf_wl_seat *seat) {
	if (seat->pointer == NULL) {
		return;
	}
	wlf_linked_list_remove(&seat->listeners.pointer_frame.link);
	wlf_linked_list_remove(&seat->listeners.pointer_axis.link);
	wlf_linked_list_remove(&seat->listeners.pointer_button.link);
	wlf_linked_list_remove(&seat->listeners.pointer_motion.link);
	wlf_linked_list_remove(&seat->listeners.pointer_leave.link);
	wlf_linked_list_remove(&seat->listeners.pointer_enter.link);
	wlf_pointer_destroy(seat->pointer);
	seat->pointer = NULL;
	seat_set_pointer_window(seat, NULL);
}

static void seat_destroy_keyboard(struct wlf_wl_seat *seat) {
	if (seat->keyboard == NULL) {
		return;
	}
	wlf_linked_list_remove(&seat->listeners.keyboard_repeat_info.link);
	wlf_linked_list_remove(&seat->listeners.keyboard_modifiers.link);
	wlf_linked_list_remove(&seat->listeners.keyboard_key.link);
	wlf_linked_list_remove(&seat->listeners.keyboard_leave.link);
	wlf_linked_list_remove(&seat->listeners.keyboard_enter.link);
	wlf_linked_list_remove(&seat->listeners.keyboard_keymap.link);
	wlf_keyboard_destroy(seat->keyboard);
	seat->keyboard = NULL;
	seat_set_keyboard_window(seat, NULL);
}

static void seat_destroy_touch(struct wlf_wl_seat *seat) {
	if (seat->touch == NULL) {
		return;
	}
	wlf_linked_list_remove(&seat->listeners.touch_orientation.link);
	wlf_linked_list_remove(&seat->listeners.touch_shape.link);
	wlf_linked_list_remove(&seat->listeners.touch_frame.link);
	wlf_linked_list_remove(&seat->listeners.touch_cancel.link);
	wlf_linked_list_remove(&seat->listeners.touch_motion.link);
	wlf_linked_list_remove(&seat->listeners.touch_up.link);
	wlf_linked_list_remove(&seat->listeners.touch_down.link);
	wlf_touch_destroy(seat->touch);
	seat->touch = NULL;
	seat_set_touch_window(seat, NULL);
}

static bool seat_create_pointer(struct wlf_wl_seat *seat) {
	seat->pointer = wlf_wl_pointer_create(seat->wl_seat);
	if (seat->pointer == NULL) {
		return false;
	}
	seat->listeners.pointer_enter.notify = handle_pointer_enter;
	seat->listeners.pointer_leave.notify = handle_pointer_leave;
	seat->listeners.pointer_motion.notify = handle_pointer_motion;
	seat->listeners.pointer_button.notify = handle_pointer_button;
	seat->listeners.pointer_axis.notify = handle_pointer_axis;
	seat->listeners.pointer_frame.notify = handle_pointer_frame;
	wlf_signal_add(&seat->pointer->events.enter, &seat->listeners.pointer_enter);
	wlf_signal_add(&seat->pointer->events.leave, &seat->listeners.pointer_leave);
	wlf_signal_add(&seat->pointer->events.motion_absolute, &seat->listeners.pointer_motion);
	wlf_signal_add(&seat->pointer->events.button, &seat->listeners.pointer_button);
	wlf_signal_add(&seat->pointer->events.axis, &seat->listeners.pointer_axis);
	wlf_signal_add(&seat->pointer->events.frame, &seat->listeners.pointer_frame);
	return true;
}

static bool seat_create_keyboard(struct wlf_wl_seat *seat) {
	struct wlf_wl_keyboard *keyboard = wlf_wl_keyboard_create(seat->wl_seat);
	if (keyboard == NULL) {
		return false;
	}
	seat->keyboard = &keyboard->base;
	seat->listeners.keyboard_keymap.notify = handle_keyboard_keymap;
	seat->listeners.keyboard_enter.notify = handle_keyboard_enter;
	seat->listeners.keyboard_leave.notify = handle_keyboard_leave;
	seat->listeners.keyboard_key.notify = handle_keyboard_key;
	seat->listeners.keyboard_modifiers.notify = handle_keyboard_modifiers;
	seat->listeners.keyboard_repeat_info.notify = handle_keyboard_repeat_info;
	wlf_signal_add(&seat->keyboard->events.keymap, &seat->listeners.keyboard_keymap);
	wlf_signal_add(&seat->keyboard->events.enter, &seat->listeners.keyboard_enter);
	wlf_signal_add(&seat->keyboard->events.leave, &seat->listeners.keyboard_leave);
	wlf_signal_add(&seat->keyboard->events.key, &seat->listeners.keyboard_key);
	wlf_signal_add(&seat->keyboard->events.modifiers, &seat->listeners.keyboard_modifiers);
	wlf_signal_add(&seat->keyboard->events.repeat_info, &seat->listeners.keyboard_repeat_info);
	return true;
}

static bool seat_create_touch(struct wlf_wl_seat *seat) {
	seat->touch = wlf_wl_touch_create(seat->wl_seat);
	if (seat->touch == NULL) {
		return false;
	}
	seat->listeners.touch_down.notify = handle_touch_down;
	seat->listeners.touch_up.notify = handle_touch_up;
	seat->listeners.touch_motion.notify = handle_touch_motion;
	seat->listeners.touch_cancel.notify = handle_touch_cancel;
	seat->listeners.touch_frame.notify = handle_touch_frame;
	seat->listeners.touch_shape.notify = handle_touch_shape;
	seat->listeners.touch_orientation.notify = handle_touch_orientation;
	wlf_signal_add(&seat->touch->events.down, &seat->listeners.touch_down);
	wlf_signal_add(&seat->touch->events.up, &seat->listeners.touch_up);
	wlf_signal_add(&seat->touch->events.motion, &seat->listeners.touch_motion);
	wlf_signal_add(&seat->touch->events.cancel, &seat->listeners.touch_cancel);
	wlf_signal_add(&seat->touch->events.frame, &seat->listeners.touch_frame);
	wlf_signal_add(&seat->touch->events.shape, &seat->listeners.touch_shape);
	wlf_signal_add(&seat->touch->events.orientation, &seat->listeners.touch_orientation);
	return true;
}

static void seat_handle_capabilities(void *data, struct wl_seat *base,
		uint32_t capabilities) {
	WLF_UNUSED(base);

	struct wlf_wl_seat *seat = data;
	uint32_t previous = seat->capabilities;
	seat->capabilities = capabilities;
	if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && seat->pointer == NULL) {
		(void)seat_create_pointer(seat);
	} else if (!(capabilities & WL_SEAT_CAPABILITY_POINTER) &&
			(previous & WL_SEAT_CAPABILITY_POINTER)) {
		seat_destroy_pointer(seat);
	}
	if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && seat->keyboard == NULL) {
		(void)seat_create_keyboard(seat);
	} else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) &&
			(previous & WL_SEAT_CAPABILITY_KEYBOARD)) {
		seat_destroy_keyboard(seat);
	}
	if ((capabilities & WL_SEAT_CAPABILITY_TOUCH) && seat->touch == NULL) {
		(void)seat_create_touch(seat);
	} else if (!(capabilities & WL_SEAT_CAPABILITY_TOUCH) &&
			(previous & WL_SEAT_CAPABILITY_TOUCH)) {
		seat_destroy_touch(seat);
	}
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
	seat_destroy_touch(seat);
	seat_destroy_keyboard(seat);
	seat_destroy_pointer(seat);
	assert(wlf_linked_list_empty(&seat->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&seat->events.capabilities.listener_list));
	assert(wlf_linked_list_empty(&seat->events.name.listener_list));

	if (seat->wl_seat != NULL) {
		wl_seat_release(seat->wl_seat);
	}

	free(seat->name);
	free(seat);
}
