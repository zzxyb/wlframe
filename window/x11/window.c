#include "wlf/window/x11/window.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb_icccm.h>

static struct wlf_x11_window *find_window(struct wlf_x11_backend *backend,
		xcb_window_t id) {
	struct wlf_x11_window *window;
	wlf_linked_list_for_each(window, &backend->windows, link) {
		if (window->window == id) {
			return window;
		}
	}
	return NULL;
}

static void window_destroy(struct wlf_window *base) {
	struct wlf_x11_window *window = wlf_x11_window_from_window(base);
	wlf_linked_list_remove(&window->link);
	if (window->window != XCB_WINDOW_NONE) {
		xcb_destroy_window(window->backend->xcb, window->window);
	}
	if (window->colormap != XCB_COLORMAP_NONE) {
		xcb_free_colormap(window->backend->xcb, window->colormap);
	}
	xcb_flush(window->backend->xcb);
	free(window);
}

static void window_show(struct wlf_window *base) {
	struct wlf_x11_window *window = wlf_x11_window_from_window(base);
	xcb_map_window(window->backend->xcb, window->window);
	window->frame_pending = true;
	xcb_flush(window->backend->xcb);
}

static void window_hide(struct wlf_window *base) {
	struct wlf_x11_window *window = wlf_x11_window_from_window(base);
	xcb_unmap_window(window->backend->xcb, window->window);
	xcb_flush(window->backend->xcb);
}

static void window_set_title(struct wlf_window *base, const char *title) {
	struct wlf_x11_window *window = wlf_x11_window_from_window(base);
	xcb_change_property(window->backend->xcb, XCB_PROP_MODE_REPLACE,
		window->window, window->backend->atoms.net_wm_name,
		window->backend->atoms.utf8_string, 8, strlen(title), title);
	xcb_change_property(window->backend->xcb, XCB_PROP_MODE_REPLACE,
		window->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);
}

static void window_set_size(struct wlf_window *base, int width, int height) {
	if (width <= 0 || height <= 0 || width > UINT16_MAX || height > UINT16_MAX) {
		return;
	}
	struct wlf_x11_window *window = wlf_x11_window_from_window(base);
	uint32_t values[] = { width, height };
	xcb_configure_window(window->backend->xcb, window->window,
		XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
}

static void window_set_position(struct wlf_window *base, int x, int y) {
	struct wlf_x11_window *window = wlf_x11_window_from_window(base);
	uint32_t values[] = { x, y };
	xcb_configure_window(window->backend->xcb, window->window,
		XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
}

static void window_set_geometry(struct wlf_window *base, const struct wlf_rect *rect) {
	window_set_position(base, rect->x, rect->y);
	window_set_size(base, rect->width, rect->height);
}

static void update_size_hints(struct wlf_window *base, int width, int height) {
	struct wlf_x11_window *window = wlf_x11_window_from_window(base);
	xcb_size_hints_t hints = {0};
	if (base->state.min_size.width > 0 && base->state.min_size.height > 0) {
		xcb_icccm_size_hints_set_min_size(&hints,
			base->state.min_size.width, base->state.min_size.height);
	}
	if (base->state.max_size.width > 0 && base->state.max_size.height > 0) {
		xcb_icccm_size_hints_set_max_size(&hints,
			base->state.max_size.width, base->state.max_size.height);
	}
	if (!(base->state.flags & WLF_WINDOW_FLAG_RESIZABLE)) {
		xcb_icccm_size_hints_set_min_size(&hints,
			base->state.geometry.width, base->state.geometry.height);
		xcb_icccm_size_hints_set_max_size(&hints,
			base->state.geometry.width, base->state.geometry.height);
	}
	xcb_icccm_set_wm_normal_hints(window->backend->xcb, window->window, &hints);
}

static void window_set_flags(struct wlf_window *base, uint32_t flags) {
	struct wlf_x11_window *window = wlf_x11_window_from_window(base);
	uint32_t hints[] = { 2, 0, !!(flags & WLF_WINDOW_FLAG_DECORATED), 0, 0 };
	xcb_change_property(window->backend->xcb, XCB_PROP_MODE_REPLACE,
		window->window, window->backend->atoms.motif_wm_hints,
		window->backend->atoms.motif_wm_hints, 32, 5, hints);
	base->state.server_side_decorated = !!(flags & WLF_WINDOW_FLAG_DECORATED);
	update_size_hints(base, 0, 0);
}

static void send_state(struct wlf_x11_window *window, bool enabled,
		xcb_atom_t first, xcb_atom_t second) {
	xcb_client_message_event_t event = {
		.response_type = XCB_CLIENT_MESSAGE,
		.format = 32,
		.window = window->window,
		.type = window->backend->atoms.net_wm_state,
		.data.data32 = { enabled ? 1 : 0, first, second, 1, 0 },
	};
	xcb_send_event(window->backend->xcb, false, window->backend->screen->root,
		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
		(const char *)&event);
}

static void window_set_state(struct wlf_window *base, enum wlf_window_state_flags state) {
	struct wlf_x11_window *window = wlf_x11_window_from_window(base);
	if (!base->state.visible) {
		xcb_atom_t atoms[3];
		size_t count = 0;
		if (state & WLF_WINDOW_FULLSCREEN) {
			atoms[count++] = window->backend->atoms.net_wm_state_fullscreen;
		}
		if (state & WLF_WINDOW_MAXIMIZED) {
			atoms[count++] = window->backend->atoms.net_wm_state_maximized_vert;
			atoms[count++] = window->backend->atoms.net_wm_state_maximized_horz;
		}
		xcb_change_property(window->backend->xcb, XCB_PROP_MODE_REPLACE,
			window->window, window->backend->atoms.net_wm_state,
			XCB_ATOM_ATOM, 32, count, atoms);
		return;
	}
	send_state(window, state & WLF_WINDOW_FULLSCREEN,
		window->backend->atoms.net_wm_state_fullscreen, XCB_ATOM_NONE);
	send_state(window, state & WLF_WINDOW_MAXIMIZED,
		window->backend->atoms.net_wm_state_maximized_vert,
		window->backend->atoms.net_wm_state_maximized_horz);
}

static void *window_native_handle(struct wlf_window *base) {
	return (void *)(uintptr_t)wlf_x11_window_from_window(base)->window;
}

static void window_schedule_frame(struct wlf_window *base) {
	wlf_x11_window_from_window(base)->frame_pending = true;
}

static const struct wlf_window_impl window_impl = {
	.destroy = window_destroy,
	.close = window_hide,
	.show = window_show,
	.hide = window_hide,
	.set_title = window_set_title,
	.set_geometry = window_set_geometry,
	.set_size = window_set_size,
	.set_position = window_set_position,
	.set_min_size = update_size_hints,
	.set_max_size = update_size_hints,
	.set_flags = window_set_flags,
	.set_state = window_set_state,
	.native_handle = window_native_handle,
	.arm_frame = window_schedule_frame,
	.schedule_frame = window_schedule_frame,
};

struct wlf_x11_window *wlf_x11_window_create_from_backend(
		struct wlf_backend *base, uint32_t width, uint32_t height) {
	if (!wlf_backend_is_x11(base) || width == 0 || height == 0 ||
			width > INT16_MAX || height > INT16_MAX) {
		return NULL;
	}
	struct wlf_x11_backend *backend = wlf_x11_backend_from_backend(base);
	struct wlf_x11_window *window = calloc(1, sizeof(*window));
	if (window == NULL) {
		return NULL;
	}
	wlf_window_init(&window->base, WLF_WINDOW_TYPE_TOPLEVEL,
		&window_impl, base, width, height);
	window->backend = backend;
	window->depth = backend->screen->root_depth;
	wlf_linked_list_insert(&backend->windows, &window->link);
	window->window = xcb_generate_id(backend->xcb);
	uint32_t values[] = { 0, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
		XCB_EVENT_MASK_FOCUS_CHANGE | XCB_EVENT_MASK_ENTER_WINDOW |
		XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_POINTER_MOTION |
		XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE };
	xcb_void_cookie_t cookie = xcb_create_window_checked(backend->xcb,
		window->depth, window->window, backend->screen->root, 0, 0,
		width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		backend->screen->root_visual, XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, values);
	xcb_generic_error_t *error = xcb_request_check(backend->xcb, cookie);
	if (error != NULL) {
		wlf_log(WLF_ERROR, "Failed to create X11 window: error %u", error->error_code);
		free(error);
		window->window = XCB_WINDOW_NONE;
		wlf_window_destroy(&window->base);
		return NULL;
	}
	xcb_icccm_set_wm_protocols(backend->xcb, window->window,
		backend->atoms.wm_protocols, 1, &backend->atoms.wm_delete_window);
	window_set_flags(&window->base, window->base.state.flags);
	return window;
}

bool wlf_window_is_x11(const struct wlf_window *window) {
	return window != NULL && window->impl == &window_impl;
}

struct wlf_x11_window *wlf_x11_window_from_window(struct wlf_window *base) {
	assert(wlf_window_is_x11(base));
	struct wlf_x11_window *window = wlf_container_of(base, window, base);
	return window;
}

void wlf_x11_window_handle_event(struct wlf_x11_backend *backend,
		xcb_generic_event_t *event) {
	uint8_t type = event->response_type & 0x7f;
	xcb_window_t id;
	switch (type) {
	case XCB_EXPOSE: id = ((xcb_expose_event_t *)event)->window; break;
	case XCB_CONFIGURE_NOTIFY: id = ((xcb_configure_notify_event_t *)event)->window; break;
	case XCB_CLIENT_MESSAGE: id = ((xcb_client_message_event_t *)event)->window; break;
	case XCB_DESTROY_NOTIFY: id = ((xcb_destroy_notify_event_t *)event)->window; break;
	case XCB_FOCUS_IN:
	case XCB_FOCUS_OUT: id = ((xcb_focus_in_event_t *)event)->event; break;
	case XCB_ENTER_NOTIFY:
	case XCB_LEAVE_NOTIFY: id = ((xcb_enter_notify_event_t *)event)->event; break;
	case XCB_MOTION_NOTIFY:
	case XCB_BUTTON_PRESS:
	case XCB_BUTTON_RELEASE:
	case XCB_KEY_PRESS:
	case XCB_KEY_RELEASE: id = ((xcb_key_press_event_t *)event)->event; break;
	case 0:
		wlf_log(WLF_ERROR, "X11 protocol error %u", ((xcb_generic_error_t *)event)->error_code);
		return;
	default: return;
	}
	struct wlf_x11_window *window = find_window(backend, id);
	if (window == NULL) {
		return;
	}
	struct wlf_window *base = &window->base;
	switch (type) {
	case XCB_EXPOSE:
		if (base->scene != NULL) {
			wlf_scene_damage_whole(base->scene);
		}
		window->frame_pending = true;
		break;
	case XCB_CONFIGURE_NOTIFY: {
		xcb_configure_notify_event_t *ev = (void *)event;
		bool resized = base->state.geometry.width != ev->width ||
			base->state.geometry.height != ev->height;
		base->state.geometry = (struct wlf_rect){ ev->x, ev->y, ev->width, ev->height };
		if (resized) {
			wlf_signal_emit_mutable(&base->events.resize, NULL);
		}
		break;
	}
	case XCB_CLIENT_MESSAGE: {
		xcb_client_message_event_t *ev = (void *)event;
		if (ev->type == backend->atoms.wm_protocols && ev->format == 32 &&
				ev->data.data32[0] == backend->atoms.wm_delete_window) {
			wlf_window_close(base);
		}
		break;
	}
	case XCB_DESTROY_NOTIFY:
		window->window = XCB_WINDOW_NONE;
		wlf_window_close(base);
		break;
	case XCB_FOCUS_IN:
	case XCB_FOCUS_OUT:
		base->state.focused = type == XCB_FOCUS_IN;
		wlf_signal_emit_mutable(type == XCB_FOCUS_IN ?
			&base->events.focus_in : &base->events.focus_out, NULL);
		break;
	case XCB_ENTER_NOTIFY: {
		xcb_enter_notify_event_t *ev = (void *)event;
		struct wlf_pointer_enter_event enter = { .serial = ev->sequence,
			.surface = window_native_handle(base), .x = ev->event_x, .y = ev->event_y };
		wlf_window_pointer_enter(base, &enter);
		break;
	}
	case XCB_LEAVE_NOTIFY: {
		struct wlf_pointer_leave_event leave = { .surface = window_native_handle(base) };
		wlf_window_pointer_leave(base, &leave);
		break;
	}
	case XCB_MOTION_NOTIFY: {
		xcb_motion_notify_event_t *ev = (void *)event;
		struct wlf_pointer_motion_absolute_event motion = { .time_msec = ev->time,
			.surface = window_native_handle(base), .x = ev->event_x, .y = ev->event_y };
		wlf_window_pointer_motion(base, &motion);
		break;
	}
	case XCB_BUTTON_PRESS:
	case XCB_BUTTON_RELEASE: {
		xcb_button_press_event_t *ev = (void *)event;
		if (ev->detail >= 4 && ev->detail <= 7) {
			if (type == XCB_BUTTON_PRESS) {
				int sign = (ev->detail == 4 || ev->detail == 6) ? -1 : 1;
				struct wlf_pointer_axis_event axis = { .time_msec = ev->time,
					.orientation = ev->detail < 6 ? WLF_POINTER_AXIS_VERTICAL_SCROLL :
						WLF_POINTER_AXIS_HORIZONTAL_SCROLL,
					.delta = sign * 15, .delta_discrete = sign * 120 };
				wlf_window_pointer_axis(base, &axis);
			}
		} else {
			uint32_t button = ev->detail == 1 ? WLF_POINTER_BUTTON_LEFT :
				ev->detail == 2 ? WLF_POINTER_BUTTON_MIDDLE :
				ev->detail == 3 ? WLF_POINTER_BUTTON_RIGHT : 0x110 + ev->detail - 1;
			struct wlf_pointer_button_event press = { .time_msec = ev->time,
				.serial = ev->sequence, .button = button,
				.state = type == XCB_BUTTON_PRESS ? WLF_POINTER_BUTTON_STATE_PRESSED :
					WLF_POINTER_BUTTON_STATE_RELEASED };
			wlf_window_pointer_button(base, &press);
		}
		break;
	}
	case XCB_KEY_PRESS:
	case XCB_KEY_RELEASE: {
		xcb_key_press_event_t *ev = (void *)event;
		if (ev->detail >= 8) {
			struct wlf_keyboard_key_event key = { .serial = ev->sequence,
				.time_msec = ev->time, .key = ev->detail - 8,
				.state = type == XCB_KEY_PRESS ? WLF_KEYBOARD_KEY_STATE_PRESSED :
					WLF_KEYBOARD_KEY_STATE_RELEASED };
			wlf_window_keyboard_key(base, &key);
		}
		break;
	}
	}
}
