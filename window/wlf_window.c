#include "wlf/window/wlf_window.h"
#include "wlf/config.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/scene/wlf_event_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/window/wlf_titlebar.h"
#include "wlf/platform/wlf_theme.h"
#include "wlf/swapchain/wlf_swapchain.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/window/wayland/xdg_toplevel_window.h"
#elif WLF_HAS_WINDOWS_PLATFORM
#include "wlf/window/windows/win32_window.h"
#endif

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

struct wlf_window_touch_point {
	struct wlf_window_touch_point *next;
	struct wlf_event_node *event_node;
	int32_t touch_id;
	bool active;
	bool frame_pending;
};

static struct wlf_window_touch_point *window_touch_point_find(
		struct wlf_window *window, int32_t touch_id) {
	for (struct wlf_window_touch_point *point = window->touch_points;
			point != NULL; point = point->next) {
		if (point->touch_id == touch_id) {
			return point;
		}
	}
	return NULL;
}

static void window_touch_point_remove(struct wlf_window *window,
		int32_t touch_id) {
	struct wlf_window_touch_point **link = &window->touch_points;
	while (*link != NULL) {
		struct wlf_window_touch_point *point = *link;
		if (point->touch_id == touch_id) {
			*link = point->next;
			free(point);
			return;
		}
		link = &point->next;
	}
}

static void window_touch_points_finish(struct wlf_window *window) {
	while (window->touch_points != NULL) {
		struct wlf_window_touch_point *point = window->touch_points;
		window->touch_points = point->next;
		free(point);
	}
}

static uint32_t get_render_format(bool has_alpha) {
	if (has_alpha) {
		return WLF_FORMAT_ARGB8888;
	}

	return WLF_FORMAT_XRGB8888;
}

static void handle_theme_changed(struct wlf_listener *listener, void *data) {
	struct wlf_window *window =
		wlf_container_of(listener, window, theme_changed);
	struct wlf_theme *theme = data;
	if (!window->uses_theme_background) {
		return;
	}
	window->state.background_color =
		theme->palette[WLF_THEME_COLOR_WINDOW_BACKGROUND];
	if (window->impl->set_background_color != NULL) {
		window->impl->set_background_color(window,
			&window->state.background_color);
	}
	if (window->scene != NULL) {
		wlf_scene_damage_whole(window->scene);
	}
}

void wlf_window_init(struct wlf_window *window, enum wlf_window_type type,
		const struct wlf_window_impl *impl, struct wlf_backend *backend,
		uint32_t width, uint32_t height) {
	assert(impl->destroy);
	assert(backend != NULL);
	(void)wlf_backend_init_theme(backend);

	*window = (struct wlf_window){
		.impl = impl,
		.state.type = type,
		.state.opacity = 1.0f,
		.state.scale = 1.0,
		.state.background_color = WLF_COLOR_BLACK,
		.state.backend = backend,
		.state.server_side_decorated =
			wlf_backend_supports_server_side_decorations(backend),
		.state.geometry = {
			.width = (int)width,
			.height = (int)height,
		},
		.features = {
			.enable_set_position = impl->set_position != NULL,
			.enable_set_min_size = impl->set_min_size != NULL,
			.enable_set_max_size = impl->set_max_size != NULL,
		},
	};
	if (type == WLF_WINDOW_TYPE_TOPLEVEL || type == WLF_WINDOW_TYPE_DIALOG) {
		window->state.flags = WLF_WINDOW_FLAG_RESIZABLE |
			WLF_WINDOW_FLAG_DECORATED;
	}

	wlf_render_format_init(&window->state.format, get_render_format(false));
	window->uses_theme_background = true;
	if (backend->theme != NULL) {
		window->state.background_color = backend->theme->palette[
			WLF_THEME_COLOR_WINDOW_BACKGROUND];
		window->theme_changed.notify = handle_theme_changed;
		wlf_signal_add(&backend->theme->events.theme_changed,
			&window->theme_changed);
		window->theme_listener_attached = true;
	}

	wlf_signal_init(&window->events.destroy);
	wlf_signal_init(&window->events.expose);
	wlf_signal_init(&window->events.resize);
	wlf_signal_init(&window->events.move);
	wlf_signal_init(&window->events.close);
	wlf_signal_init(&window->events.focus_in);
	wlf_signal_init(&window->events.focus_out);
	wlf_signal_init(&window->events.scale);
	wlf_signal_init(&window->events.show);
	wlf_signal_init(&window->events.hide);
	struct wlf_signal *input_signals[] = {
		&window->events.pointer_enter, &window->events.pointer_leave,
		&window->events.pointer_motion, &window->events.pointer_button,
		&window->events.pointer_axis, &window->events.pointer_frame,
		&window->events.keyboard_enter, &window->events.keyboard_leave,
		&window->events.keyboard_keymap, &window->events.keyboard_key,
		&window->events.keyboard_modifiers,
		&window->events.keyboard_repeat_info, &window->events.tablet,
		&window->events.touch_down, &window->events.touch_up,
		&window->events.touch_motion, &window->events.touch_cancel,
		&window->events.touch_frame, &window->events.touch_shape,
		&window->events.touch_orientation,
	};
	for (size_t i = 0; i < sizeof(input_signals) / sizeof(input_signals[0]); ++i) {
		wlf_signal_init(input_signals[i]);
	}
}

struct wlf_window *wlf_window_create_toplevel(struct wlf_backend *backend,
		uint32_t width, uint32_t height) {
	if (backend == NULL || width == 0 || height == 0) {
		return NULL;
	}
#if WLF_HAS_LINUX_PLATFORM
	return wlf_xdg_toplevel_window_create_from_backend(backend, width, height);
#elif WLF_HAS_WINDOWS_PLATFORM
	return wlf_win32_window_create_from_backend(backend, width, height);
#else
	wlf_log(WLF_ERROR, "Native toplevel windows are not implemented on this platform");
	return NULL;
#endif
}

void wlf_window_destroy(struct wlf_window *window) {
	if (window == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&window->events.destroy, window);
	if (window->theme_listener_attached) {
		wlf_linked_list_remove(&window->theme_changed.link);
		window->theme_listener_attached = false;
	}
	if (window->scene != NULL) {
		wlf_scene_destroy(window->scene);
	}
	if (window->state.swapchain != NULL) {
		wlf_swapchain_destroy(window->state.swapchain);
		window->state.swapchain = NULL;
	}
	window_touch_points_finish(window);
	free(window->state.title);
	wlf_render_format_finish(&window->state.format);
	if (window->impl->destroy) {
		window->impl->destroy(window);
	} else {
		free(window);
	}
}

void wlf_window_close(struct wlf_window *window) {
	if (window->impl->close) {
		window->impl->close(window);
	}

	window->state.visible = false;
	wlf_signal_emit_mutable(&window->events.close, window);
}

void wlf_window_show(struct wlf_window *window) {
	if (window->impl->show) {
		window->impl->show(window);
	}

	window->state.visible = true;
	wlf_signal_emit_mutable(&window->events.show, window);
}

void wlf_window_hide(struct wlf_window *window) {
	if (window->impl->hide) {
		window->impl->hide(window);
	}

	window->state.visible = false;
	wlf_signal_emit_mutable(&window->events.hide, window);
}

void wlf_window_set_title(struct wlf_window *window, const char *title) {
	char *new_title = strdup(title);
	if (new_title == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to duplicate window title");
		return;
	}

	free(window->state.title);
	window->state.title = new_title;

	if (window->impl->set_title) {
		window->impl->set_title(window, new_title);
	}
	if (window->scene != NULL && window->scene->titlebar != NULL) {
		wlf_titlebar_set_title(window->scene->titlebar, new_title);
	}
}

struct wlf_titlebar *wlf_window_get_titlebar(struct wlf_window *window) {
	return window != NULL && window->scene != NULL ?
		window->scene->titlebar : NULL;
}

void wlf_window_set_geometry(struct wlf_window *window,
		const struct wlf_rect *geometry) {
	bool resized = window->state.geometry.width != geometry->width ||
		window->state.geometry.height != geometry->height;
	window->state.geometry = *geometry;
	if (window->impl->set_geometry) {
		window->impl->set_geometry(window, &window->state.geometry);
	}
	if (resized) {
		wlf_signal_emit_mutable(&window->events.resize, window);
	}
}

void wlf_window_set_size(struct wlf_window *window, int width, int height) {
	bool resized = window->state.geometry.width != width ||
		window->state.geometry.height != height;
	window->state.geometry.width = width;
	window->state.geometry.height = height;

	if (window->impl->set_size) {
		window->impl->set_size(window, width, height);
	}
	if (resized) {
		wlf_signal_emit_mutable(&window->events.resize, window);
	}
}

void wlf_window_set_min_size(struct wlf_window *window, int width, int height) {
	window->state.min_size.width = width;
	window->state.min_size.height = height;

	if (window->impl->set_min_size) {
		window->impl->set_min_size(window, width, height);
	}
}

void wlf_window_set_max_size(struct wlf_window *window, int width, int height) {
	window->state.max_size.width = width;
	window->state.max_size.height = height;

	if (window->impl->set_max_size) {
		window->impl->set_max_size(window, width, height);
	}
}

void wlf_window_set_position(struct wlf_window *window, int x, int y) {
	if (window->impl->set_position) {
		window->impl->set_position(window, x, y);
	}

	window->state.geometry.x = x;
	window->state.geometry.y = y;
	wlf_signal_emit_mutable(&window->events.move, window);
}

void wlf_window_begin_move(struct wlf_window *window,
		struct wlf_pointer *pointer, uint32_t serial) {
	if (window == NULL || pointer == NULL || serial == 0 ||
			window->impl->begin_move == NULL) {
		return;
	}
	window->impl->begin_move(window, pointer, serial);
}

void wlf_window_begin_resize(struct wlf_window *window,
		struct wlf_pointer *pointer, uint32_t serial,
		enum wlf_window_resize_edge edge) {
	if (window == NULL || pointer == NULL || serial == 0 ||
			edge == WLF_WINDOW_RESIZE_EDGE_NONE ||
			!(window->state.flags & WLF_WINDOW_FLAG_RESIZABLE) ||
			window->impl->begin_resize == NULL) {
		return;
	}
	window->impl->begin_resize(window, pointer, serial, edge);
}

uint32_t wlf_window_scale_length(const struct wlf_window *window,
		uint32_t logical_length) {
	assert(window != NULL);
	double scaled = ceil(logical_length * window->state.scale);
	return scaled >= INT_MAX ? INT_MAX : (uint32_t)scaled;
}

void wlf_window_set_scale(struct wlf_window *window, double scale) {
	assert(window != NULL);
	if (!isfinite(scale) || scale <= 0.0) {
		wlf_log(WLF_ERROR, "Ignoring invalid window scale %.2f", scale);
		return;
	}
	if (window->state.scale == scale) {
		return;
	}

	window->state.scale = scale;
	wlf_signal_emit_mutable(&window->events.scale, window);
	if (window->scene != NULL) {
		wlf_scene_damage_whole(window->scene);
	}
}

void wlf_window_set_state(struct wlf_window *window,
		enum wlf_window_state_flags state) {
	if (window->impl->set_state) {
		window->impl->set_state(window, state);
	}

	window->state.state = state;
	if (window->scene != NULL) {
		bool client_side = !window->state.server_side_decorated &&
			!(state & WLF_WINDOW_FULLSCREEN);
		if (!wlf_scene_set_client_side_decorated(window->scene, client_side)) {
			wlf_log(WLF_ERROR, "Failed to update client-side decoration state");
		}
		if (window->scene->titlebar != NULL) {
			wlf_titlebar_arrange(window->scene->titlebar);
		}
	}
}

void wlf_window_set_flags(struct wlf_window *window, uint32_t flags) {
	window->state.flags = flags;

	if (window->impl->set_flags) {
		window->impl->set_flags(window, flags);
	}
	if (window->scene != NULL && window->scene->titlebar != NULL) {
		wlf_titlebar_arrange(window->scene->titlebar);
	}
}

void wlf_window_set_input_region(struct wlf_window *window,
		const pixman_region32_t *region) {
	if (window->impl->set_input_region) {
		window->impl->set_input_region(window, region);
	}
}

void wlf_window_set_opaque_region(struct wlf_window *window,
		const pixman_region32_t *region) {
	if (window->impl->set_opaque_region) {
		window->impl->set_opaque_region(window, region);
	}
}

void wlf_window_set_opacity(struct wlf_window *window, float opacity) {
	window->state.opacity = opacity;

	if (window->impl->set_opacity) {
		window->impl->set_opacity(window, opacity);
	}
}

void wlf_window_set_mask(struct wlf_window *window,
		const pixman_region32_t *mask) {
	if (window->impl->set_mask) {
		window->impl->set_mask(window, mask);
	}
}

void wlf_window_set_background_color(struct wlf_window *window,
		const struct wlf_color *color) {
	window->uses_theme_background = false;
	window->state.background_color = *color;

	if (window->impl->set_background_color) {
		window->impl->set_background_color(window, &window->state.background_color);
	}
	if (window->scene != NULL) {
		wlf_scene_damage_whole(window->scene);
	}
}

void *wlf_window_native_handle(struct wlf_window *window) {
	if (window == NULL || window->impl->native_handle == NULL) {
		return NULL;
	}

	return window->impl->native_handle(window);
}

void wlf_window_init_renderer(struct wlf_window *window, struct wlf_renderer *renderer) {
	if (window->state.swapchain != NULL) {
		wlf_swapchain_destroy(window->state.swapchain);
		window->state.swapchain = NULL;
	}

	window->state.renderer = renderer;
	if (renderer == NULL || window->state.geometry.width <= 0 ||
			window->state.geometry.height <= 0) {
		return;
	}

	int width = (int)wlf_window_scale_length(window,
		(uint32_t)window->state.geometry.width);
	int height = (int)wlf_window_scale_length(window,
		(uint32_t)window->state.geometry.height);
	window->state.swapchain = wlf_swapchain_auto_create(window,
		width, height, &window->state.format);
	if (window->state.swapchain == NULL) {
		wlf_log(WLF_ERROR, "Failed to create window swapchain");
	}
}

void wlf_window_schedule_frame(struct wlf_window *window) {
	if (window == NULL) {
		return;
	}

	if (window->impl->schedule_frame != NULL) {
		window->impl->schedule_frame(window);
		return;
	}

	/* Backends without explicit frame callbacks render on the expose signal. */
	wlf_signal_emit_mutable(&window->events.expose, window);
}

void wlf_window_arm_frame(struct wlf_window *window) {
	if (window != NULL && window->impl->arm_frame != NULL) {
		window->impl->arm_frame(window);
	}
}

static struct wlf_event_node *window_event_node_at(struct wlf_window *window,
		double x, double y) {
	if (window == NULL || window->scene == NULL) {
		return NULL;
	}
	return wlf_event_node_at(&window->scene->root->base, x, y);
}

static void window_update_pointer_node(struct wlf_window *window,
		struct wlf_pointer *pointer, double x, double y) {
	struct wlf_event_node *next = window_event_node_at(window, x, y);
	if (next == window->pointer_event_node) {
		return;
	}
	struct wlf_event_pointer_focus_event event = {
		.window = window,
		.pointer = pointer,
		.x = x,
		.y = y,
	};
	if (window->pointer_event_node != NULL) {
		wlf_event_node_notify_pointer_leave(
			window->pointer_event_node, &event);
	}
	window->pointer_event_node = next;
	(void)wlf_pointer_set_cursor_shape(pointer, next != NULL ?
		next->cursor_shape : WLF_CURSOR_SHAPE_DEFAULT);
	if (next != NULL) {
		wlf_event_node_notify_pointer_enter(next, &event);
	}
}

void wlf_window_pointer_enter(struct wlf_window *window,
		const struct wlf_pointer_enter_event *event) {
	window->pointer_x = event->x;
	window->pointer_y = event->y;
	(void)wlf_pointer_set_cursor_shape(event->pointer,
		WLF_CURSOR_SHAPE_DEFAULT);
	wlf_signal_emit_mutable(&window->events.pointer_enter, (void *)event);
	window_update_pointer_node(window, event->pointer, event->x, event->y);
}

void wlf_window_pointer_leave(struct wlf_window *window,
		const struct wlf_pointer_leave_event *event) {
	wlf_signal_emit_mutable(&window->events.pointer_leave, (void *)event);
	if (window->pointer_event_node != NULL) {
		struct wlf_event_pointer_focus_event focus = {
			.window = window,
			.pointer = event->pointer,
			.x = window->pointer_x,
			.y = window->pointer_y,
		};
		wlf_event_node_notify_pointer_leave(
			window->pointer_event_node, &focus);
		window->pointer_event_node = NULL;
	}
}

void wlf_window_pointer_motion(struct wlf_window *window,
		const struct wlf_pointer_motion_absolute_event *event) {
	window->pointer_x = event->x;
	window->pointer_y = event->y;
	wlf_signal_emit_mutable(&window->events.pointer_motion, (void *)event);
	window_update_pointer_node(window, event->pointer, event->x, event->y);
	if (window->pointer_event_node != NULL) {
		wlf_signal_emit_mutable(
			&window->pointer_event_node->events.pointer_motion, (void *)event);
	}
}

void wlf_window_pointer_button(struct wlf_window *window,
		const struct wlf_pointer_button_event *event) {
	wlf_signal_emit_mutable(&window->events.pointer_button, (void *)event);
	struct wlf_event_node *target = window->pointer_event_node;
	if (event->state == WLF_POINTER_BUTTON_STATE_PRESSED &&
			window->pointer_grab_event_node == NULL) {
		window->pointer_grab_event_node = target;
	} else if (event->state == WLF_POINTER_BUTTON_STATE_RELEASED &&
			window->pointer_grab_event_node != NULL) {
		target = window->pointer_grab_event_node;
		/* Clear before dispatch: a close callback may destroy the window. */
		window->pointer_grab_event_node = NULL;
	}
	if (target != NULL) {
		if (event->state == WLF_POINTER_BUTTON_STATE_PRESSED &&
				target->base.state.focus_policy == CLICK_FOCUS) {
			window->keyboard_event_node = target;
		}
		wlf_signal_emit_mutable(
			&target->events.pointer_button, (void *)event);
	}
}

void wlf_window_pointer_axis(struct wlf_window *window,
		const struct wlf_pointer_axis_event *event) {
	wlf_signal_emit_mutable(&window->events.pointer_axis, (void *)event);
	if (window->pointer_event_node != NULL) {
		wlf_signal_emit_mutable(
			&window->pointer_event_node->events.pointer_axis, (void *)event);
	}
}

void wlf_window_pointer_frame(struct wlf_window *window, void *event) {
	wlf_signal_emit_mutable(&window->events.pointer_frame, event);
	if (window->pointer_event_node != NULL) {
		wlf_signal_emit_mutable(
			&window->pointer_event_node->events.pointer_frame, event);
	}
}

void wlf_window_keyboard_enter(struct wlf_window *window,
		const struct wlf_keyboard_enter_event *event) {
	if (window->keyboard_event_node == NULL) {
		window->keyboard_event_node = window->pointer_event_node;
	}
	wlf_signal_emit_mutable(&window->events.keyboard_enter, (void *)event);
	if (window->keyboard_event_node != NULL) {
		wlf_signal_emit_mutable(
			&window->keyboard_event_node->events.keyboard_enter, (void *)event);
	}
}

void wlf_window_keyboard_leave(struct wlf_window *window,
		const struct wlf_keyboard_leave_event *event) {
	wlf_signal_emit_mutable(&window->events.keyboard_leave, (void *)event);
	if (window->keyboard_event_node != NULL) {
		wlf_signal_emit_mutable(
			&window->keyboard_event_node->events.keyboard_leave, (void *)event);
		window->keyboard_event_node = NULL;
	}
}

void wlf_window_keyboard_keymap(struct wlf_window *window,
		const struct wlf_keyboard_keymap_event *event) {
	wlf_signal_emit_mutable(&window->events.keyboard_keymap, (void *)event);
	if (window->keyboard_event_node != NULL) {
		wlf_signal_emit_mutable(
			&window->keyboard_event_node->events.keyboard_keymap, (void *)event);
	}
}

void wlf_window_keyboard_key(struct wlf_window *window,
		const struct wlf_keyboard_key_event *event) {
	wlf_signal_emit_mutable(&window->events.keyboard_key, (void *)event);
	if (window->keyboard_event_node != NULL) {
		wlf_signal_emit_mutable(
			&window->keyboard_event_node->events.keyboard_key, (void *)event);
	}
}

void wlf_window_keyboard_modifiers(struct wlf_window *window,
		const struct wlf_keyboard_modifiers_event *event) {
	wlf_signal_emit_mutable(&window->events.keyboard_modifiers, (void *)event);
	if (window->keyboard_event_node != NULL) {
		wlf_signal_emit_mutable(
			&window->keyboard_event_node->events.keyboard_modifiers, (void *)event);
	}
}

void wlf_window_keyboard_repeat_info(struct wlf_window *window,
		const struct wlf_keyboard_repeat_info_event *event) {
	wlf_signal_emit_mutable(&window->events.keyboard_repeat_info, (void *)event);
	if (window->keyboard_event_node != NULL) {
		wlf_signal_emit_mutable(
			&window->keyboard_event_node->events.keyboard_repeat_info,
			(void *)event);
	}
}

void wlf_window_touch_down(struct wlf_window *window,
		const struct wlf_touch_down_event *event) {
	wlf_signal_emit_mutable(&window->events.touch_down, (void *)event);
	window_touch_point_remove(window, event->touch_id);
	struct wlf_event_node *node = window_event_node_at(window, event->x, event->y);
	if (node == NULL) {
		return;
	}
	struct wlf_window_touch_point *point = calloc(1, sizeof(*point));
	if (point == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to track window touch point");
		return;
	}
	point->event_node = node;
	point->touch_id = event->touch_id;
	point->active = true;
	point->frame_pending = true;
	point->next = window->touch_points;
	window->touch_points = point;
	wlf_signal_emit_mutable(&node->events.touch_down, (void *)event);
}

void wlf_window_touch_motion(struct wlf_window *window,
		const struct wlf_touch_motion_event *event) {
	wlf_signal_emit_mutable(&window->events.touch_motion, (void *)event);
	struct wlf_window_touch_point *point =
		window_touch_point_find(window, event->touch_id);
	if (point != NULL && point->active && point->event_node != NULL) {
		point->frame_pending = true;
		wlf_signal_emit_mutable(&point->event_node->events.touch_motion,
			(void *)event);
	}
}

void wlf_window_touch_up(struct wlf_window *window,
		const struct wlf_touch_up_event *event) {
	wlf_signal_emit_mutable(&window->events.touch_up, (void *)event);
	struct wlf_window_touch_point *point =
		window_touch_point_find(window, event->touch_id);
	if (point != NULL && point->active && point->event_node != NULL) {
		point->active = false;
		point->frame_pending = true;
		wlf_signal_emit_mutable(&point->event_node->events.touch_up,
			(void *)event);
	}
}

void wlf_window_touch_cancel(struct wlf_window *window,
		const struct wlf_touch_cancel_event *event) {
	wlf_signal_emit_mutable(&window->events.touch_cancel, (void *)event);
	for (struct wlf_window_touch_point *point = window->touch_points;
			point != NULL; point = point->next) {
		if (point->event_node == NULL) {
			continue;
		}
		bool already_notified = false;
		for (struct wlf_window_touch_point *previous = window->touch_points;
				previous != point; previous = previous->next) {
			if (previous->event_node == point->event_node) {
				already_notified = true;
				break;
			}
		}
		if (!already_notified) {
			wlf_signal_emit_mutable(&point->event_node->events.touch_cancel,
				(void *)event);
		}
	}
	window_touch_points_finish(window);
}

void wlf_window_touch_shape(struct wlf_window *window,
		const struct wlf_touch_shape_event *event) {
	wlf_signal_emit_mutable(&window->events.touch_shape, (void *)event);
	struct wlf_window_touch_point *point =
		window_touch_point_find(window, event->touch_id);
	if (point != NULL && point->active && point->event_node != NULL) {
		point->frame_pending = true;
		wlf_signal_emit_mutable(&point->event_node->events.touch_shape,
			(void *)event);
	}
}

void wlf_window_touch_orientation(struct wlf_window *window,
		const struct wlf_touch_orientation_event *event) {
	wlf_signal_emit_mutable(&window->events.touch_orientation, (void *)event);
	struct wlf_window_touch_point *point =
		window_touch_point_find(window, event->touch_id);
	if (point != NULL && point->active && point->event_node != NULL) {
		point->frame_pending = true;
		wlf_signal_emit_mutable(&point->event_node->events.touch_orientation,
			(void *)event);
	}
}

void wlf_window_touch_frame(struct wlf_window *window, void *event) {
	wlf_signal_emit_mutable(&window->events.touch_frame, event);
	for (struct wlf_window_touch_point *point = window->touch_points;
			point != NULL; point = point->next) {
		if (point->frame_pending && point->event_node != NULL) {
			bool already_notified = false;
			for (struct wlf_window_touch_point *previous = window->touch_points;
					previous != point; previous = previous->next) {
				if (previous->frame_pending &&
						previous->event_node == point->event_node) {
					already_notified = true;
					break;
				}
			}
			if (!already_notified) {
				wlf_signal_emit_mutable(
					&point->event_node->events.touch_frame, event);
			}
		}
	}
	struct wlf_window_touch_point **link = &window->touch_points;
	while (*link != NULL) {
		struct wlf_window_touch_point *point = *link;
		point->frame_pending = false;
		if (!point->active) {
			*link = point->next;
			free(point);
		} else {
			link = &point->next;
		}
	}
}

void wlf_window_forget_touch_event_node(struct wlf_window *window,
		struct wlf_event_node *node) {
	if (window == NULL || node == NULL) {
		return;
	}
	struct wlf_window_touch_point **link = &window->touch_points;
	while (*link != NULL) {
		struct wlf_window_touch_point *point = *link;
		if (point->event_node == node) {
			*link = point->next;
			free(point);
		} else {
			link = &point->next;
		}
	}
}

void wlf_window_tablet_event(struct wlf_window *window, void *event,
		double x, double y) {
	wlf_signal_emit_mutable(&window->events.tablet, event);
	struct wlf_event_node *node = window_event_node_at(window, x, y);
	if (node != NULL) {
		wlf_signal_emit_mutable(&node->events.tablet, event);
	}
}
