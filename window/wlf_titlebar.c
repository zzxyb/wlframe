#include "wlf/window/wlf_titlebar.h"

#include "wlf/platform/wlf_backend.h"
#include "wlf/platform/wlf_theme.h"
#include "wlf/scene/wlf_event_node.h"
#include "wlf/scene/wlf_rect_node.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/scene/wlf_svg_node.h"
#include "wlf/scene/wlf_text_node.h"
#include "wlf/svg/wlf_svg.h"
#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wlf_window.h"

#include <stdlib.h>

enum {
	TITLEBAR_HORIZONTAL_PADDING = 5,
	TITLEBAR_BUTTON_GAP = 0,
	TITLEBAR_BUTTON_SIZE = 24,
	TITLEBAR_ICON_SIZE = 16,
	TITLEBAR_FONT_SIZE = 16,
	TITLEBAR_RESIZE_BORDER = 5,
};

static const struct wlf_color focused_background = {
	.r = 0xe0 / 255.0,
	.g = 0xdf / 255.0,
	.b = 0xde / 255.0,
	.a = 1.0,
};

static const struct wlf_color unfocused_background = {
	.r = 0xef / 255.0,
	.g = 0xf0 / 255.0,
	.b = 0xf1 / 255.0,
	.a = 1.0,
};

static const struct wlf_color focused_text = {
	.r = 0.0,
	.g = 0.0,
	.b = 0.0,
	.a = 1.0,
};
static const struct wlf_color unfocused_text = {
	.r = 0.0,
	.g = 0.0,
	.b = 0.0,
	.a = 0xe1 / 255.0,
};

static const struct wlf_color separator_focused = {
	.r = 0.0,
	.g = 0.0,
	.b = 0.0,
	.a = 0.16,
};

static const struct wlf_color separator_unfocused = {
	.r = 0.0,
	.g = 0.0,
	.b = 0.0,
	.a = 0.10,
};

static const struct wlf_color button_hover_light = {
	.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.09,
};

static const struct wlf_color button_hover_dark = {
	.r = 1.0, .g = 1.0, .b = 1.0, .a = 0.12,
};

static const struct wlf_color close_button_hover = {
	.r = 0xe8 / 255.0, .g = 0x11 / 255.0, .b = 0x23 / 255.0, .a = 0.88,
};

static const char minimize_icon_source[] =
	"<svg width='16' height='16' viewBox='0 0 16 16'>"
	"<path d='M3 8 L13 8' fill='none' stroke='#202124' "
		"stroke-width='1.5' stroke-linecap='round'/>"
	"</svg>";

static const char minimize_icon_source_dark[] =
	"<svg width='16' height='16' viewBox='0 0 16 16'>"
	"<path d='M3 8 L13 8' fill='none' stroke='#ffffff' "
		"stroke-width='1.5' stroke-linecap='round'/>"
	"</svg>";

static const char maximize_icon_source[] =
	"<svg width='16' height='16' viewBox='0 0 16 16'>"
	"<rect x='3.5' y='3.5' width='9' height='9' rx='1' fill='none' "
		"stroke='#202124' stroke-width='1.25'/>"
	"</svg>";

static const char maximize_icon_source_dark[] =
	"<svg width='16' height='16' viewBox='0 0 16 16'>"
	"<rect x='3.5' y='3.5' width='9' height='9' rx='1' fill='none' "
		"stroke='#ffffff' stroke-width='1.25'/>"
	"</svg>";

static const char close_icon_source[] =
	"<svg width='16' height='16' viewBox='0 0 16 16'>"
	"<path d='M4 4 L12 12 M12 4 L4 12' fill='none' stroke='#202124' "
		"stroke-width='1.5' stroke-linecap='round'/>"
	"</svg>";

static const char close_icon_source_dark[] =
	"<svg width='16' height='16' viewBox='0 0 16 16'>"
	"<path d='M4 4 L12 12 M12 4 L4 12' fill='none' stroke='#ffffff' "
		"stroke-width='1.5' stroke-linecap='round'/>"
	"</svg>";

static const char window_icon_source[] =
	"<svg width='16' height='16' viewBox='0 0 16 16'>"
	"<rect x='1.5' y='1.5' width='13' height='13' rx='3' fill='#3584e4'/>"
	"<rect x='4' y='4' width='8' height='2' rx='1' fill='#ffffff'/>"
	"<rect x='4' y='7' width='5' height='5' rx='1' fill='#ffffff'/>"
	"</svg>";

static struct wlf_svg_node *create_svg_icon(struct wlf_scene_node *parent,
		const char *source, int x, int y) {
	if (source == NULL) {
		return NULL;
	}
	char *input = strdup(source);
	if (input == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to duplicate titlebar icon SVG");
		return NULL;
	}
	struct wlf_svg_image *image = wlf_svg_parse(input, "px", 96.0f);
	free(input);
	if (image == NULL) {
		return NULL;
	}
	struct wlf_svg_node *icon = wlf_svg_node_create(parent, x, y, image);
	if (icon == NULL) {
		wlf_svg_destroy(image);
	}
	return icon;
}

static void update_button_hover(struct wlf_titlebar_button *button) {
	struct wlf_color color = {0};
	if (button->hovered) {
		if (button->type == WLF_TITLEBAR_BUTTON_CLOSE) {
			color = close_button_hover;
		} else {
			struct wlf_theme *theme =
				button->titlebar->window->state.backend->theme;
			color = theme != NULL &&
				theme->appearance == WLF_THEME_APPEARANCE_DARK ?
				button_hover_dark : button_hover_light;
		}
	}
	button->background->color = color;
	wlf_scene_node_update(&button->background->base, NULL);
}

static void handle_button_pointer_enter(struct wlf_listener *listener,
		void *data) {
	struct wlf_titlebar_button *button =
		wlf_container_of(listener, button, listeners.pointer_enter);
	struct wlf_titlebar_button *buttons[] = {
		&button->titlebar->minimize_button,
		&button->titlebar->maximize_button,
		&button->titlebar->close_button,
	};
	for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
		if (buttons[i] == button) {
			continue;
		}
		if (buttons[i]->event_node->pointer_inside) {
			wlf_event_node_notify_pointer_leave(buttons[i]->event_node, data);
		} else if (buttons[i]->hovered) {
			buttons[i]->hovered = false;
			update_button_hover(buttons[i]);
		}
	}
	button->hovered = true;
	update_button_hover(button);
}

static void handle_button_pointer_leave(struct wlf_listener *listener,
		void *data) {
	(void)data;
	struct wlf_titlebar_button *button =
		wlf_container_of(listener, button, listeners.pointer_leave);
	button->hovered = false;
	update_button_hover(button);
}

static void handle_button_pointer_button(struct wlf_listener *listener,
		void *data) {
	struct wlf_titlebar_button *button =
		wlf_container_of(listener, button, listeners.pointer_button);
	const struct wlf_pointer_button_event *event = data;
	if (event->button != WLF_POINTER_BUTTON_LEFT ||
			event->state != WLF_POINTER_BUTTON_STATE_RELEASED ||
			!button->hovered) {
		return;
	}

	struct wlf_window *window = button->titlebar->window;
	switch (button->type) {
	case WLF_TITLEBAR_BUTTON_MINIMIZE:
		wlf_window_set_state(window,
			window->state.state | WLF_WINDOW_MINIMIZED);
		break;
	case WLF_TITLEBAR_BUTTON_MAXIMIZE:
		if (window->state.state & WLF_WINDOW_MAXIMIZED) {
			wlf_window_set_state(window,
				window->state.state & ~WLF_WINDOW_MAXIMIZED);
		} else {
			wlf_window_set_state(window,
				(window->state.state & ~WLF_WINDOW_MINIMIZED) |
				WLF_WINDOW_MAXIMIZED);
		}
		break;
	case WLF_TITLEBAR_BUTTON_CLOSE:
		wlf_window_close(window);
		break;
	}
}

static bool create_button(struct wlf_titlebar *titlebar,
		struct wlf_titlebar_button *button,
		enum wlf_titlebar_button_type type, const char *icon_source) {
	button->titlebar = titlebar;
	button->type = type;
	struct wlf_scene_node *parent = &titlebar->tree->base;
	button->tree = wlf_scene_tree_create(parent);
	if (button->tree == NULL) {
		return false;
	}
	button->visible = true;

	struct wlf_color transparent = {0};
	button->background = wlf_rect_node_create(&button->tree->base,
		0, 0, TITLEBAR_BUTTON_SIZE, TITLEBAR_BUTTON_SIZE, &transparent);
	if (button->background == NULL) {
		return false;
	}

	double icon_offset =
		(TITLEBAR_BUTTON_SIZE - TITLEBAR_ICON_SIZE) / 2.0;
	button->icon = create_svg_icon(&button->tree->base, icon_source,
		(int)icon_offset, (int)icon_offset);
	if (button->icon == NULL) {
		return false;
	}
	button->event_node = wlf_event_node_create(&button->tree->base,
		0, 0, TITLEBAR_BUTTON_SIZE, TITLEBAR_BUTTON_SIZE);
	if (button->event_node == NULL) {
		return false;
	}
	button->event_node->base.state.focus_policy = NO_FOCUS;
	button->listeners.pointer_enter.notify = handle_button_pointer_enter;
	button->listeners.pointer_leave.notify = handle_button_pointer_leave;
	button->listeners.pointer_button.notify = handle_button_pointer_button;
	wlf_signal_add(&button->event_node->events.pointer_enter,
		&button->listeners.pointer_enter);
	wlf_signal_add(&button->event_node->events.pointer_leave,
		&button->listeners.pointer_leave);
	wlf_signal_add(&button->event_node->events.pointer_button,
		&button->listeners.pointer_button);
	return true;
}

static void remove_button_listeners(struct wlf_titlebar_button *button) {
	if (button->event_node == NULL) {
		return;
	}
	wlf_linked_list_remove(&button->listeners.pointer_button.link);
	wlf_linked_list_remove(&button->listeners.pointer_leave.link);
	wlf_linked_list_remove(&button->listeners.pointer_enter.link);
}

static enum wlf_cursor_shape resize_cursor_shape(
		enum wlf_window_resize_edge edge) {
	switch (edge) {
	case WLF_WINDOW_RESIZE_EDGE_TOP: return WLF_CURSOR_SHAPE_N_RESIZE;
	case WLF_WINDOW_RESIZE_EDGE_BOTTOM: return WLF_CURSOR_SHAPE_S_RESIZE;
	case WLF_WINDOW_RESIZE_EDGE_LEFT: return WLF_CURSOR_SHAPE_W_RESIZE;
	case WLF_WINDOW_RESIZE_EDGE_RIGHT: return WLF_CURSOR_SHAPE_E_RESIZE;
	case WLF_WINDOW_RESIZE_EDGE_TOP_LEFT: return WLF_CURSOR_SHAPE_NW_RESIZE;
	case WLF_WINDOW_RESIZE_EDGE_TOP_RIGHT: return WLF_CURSOR_SHAPE_NE_RESIZE;
	case WLF_WINDOW_RESIZE_EDGE_BOTTOM_LEFT: return WLF_CURSOR_SHAPE_SW_RESIZE;
	case WLF_WINDOW_RESIZE_EDGE_BOTTOM_RIGHT: return WLF_CURSOR_SHAPE_SE_RESIZE;
	default: return WLF_CURSOR_SHAPE_DEFAULT;
	}
}

static void handle_resize_pointer_button(struct wlf_listener *listener,
		void *data) {
	struct wlf_titlebar_resize_handle *handle =
		wlf_container_of(listener, handle, pointer_button);
	const struct wlf_pointer_button_event *event = data;
	if (event->button == WLF_POINTER_BUTTON_LEFT &&
			event->state == WLF_POINTER_BUTTON_STATE_PRESSED) {
		wlf_window_begin_resize(handle->titlebar->window, event->pointer,
			event->serial, (enum wlf_window_resize_edge)handle->edge);
	}
}

static bool create_resize_handles(struct wlf_titlebar *titlebar) {
	static const enum wlf_window_resize_edge edges[] = {
		WLF_WINDOW_RESIZE_EDGE_TOP,
		WLF_WINDOW_RESIZE_EDGE_BOTTOM,
		WLF_WINDOW_RESIZE_EDGE_LEFT,
		WLF_WINDOW_RESIZE_EDGE_RIGHT,
		WLF_WINDOW_RESIZE_EDGE_TOP_LEFT,
		WLF_WINDOW_RESIZE_EDGE_TOP_RIGHT,
		WLF_WINDOW_RESIZE_EDGE_BOTTOM_LEFT,
		WLF_WINDOW_RESIZE_EDGE_BOTTOM_RIGHT,
	};
	for (size_t i = 0; i < WLF_TITLEBAR_RESIZE_HANDLE_COUNT; ++i) {
		struct wlf_titlebar_resize_handle *handle =
			&titlebar->resize_handles[i];
		handle->titlebar = titlebar;
		handle->edge = (uint32_t)edges[i];
		handle->event_node = wlf_event_node_create(&titlebar->tree->base,
			0, 0, 0, 0);
		if (handle->event_node == NULL) {
			return false;
		}
		handle->event_node->base.state.focus_policy = NO_FOCUS;
		wlf_event_node_set_cursor_shape(handle->event_node,
			resize_cursor_shape(edges[i]));
		handle->pointer_button.notify = handle_resize_pointer_button;
		wlf_signal_add(&handle->event_node->events.pointer_button,
			&handle->pointer_button);
	}
	return true;
}

static void remove_resize_handle_listeners(struct wlf_titlebar *titlebar) {
	for (size_t i = 0; i < WLF_TITLEBAR_RESIZE_HANDLE_COUNT; ++i) {
		struct wlf_titlebar_resize_handle *handle =
			&titlebar->resize_handles[i];
		if (handle->event_node != NULL) {
			wlf_linked_list_remove(&handle->pointer_button.link);
		}
	}
}

static void set_event_node_box(struct wlf_event_node *node,
		int x, int y, uint32_t width, uint32_t height) {
	wlf_scene_node_set_position(&node->base, x, y);
	pixman_region32_t region;
	pixman_region32_init_rect(&region, 0, 0, width, height);
	wlf_event_node_set_input_region(node, &region);
	pixman_region32_fini(&region);
}

static bool replace_button_icon(struct wlf_titlebar_button *button,
		const char *svg_source) {
	int offset = (TITLEBAR_BUTTON_SIZE - TITLEBAR_ICON_SIZE) / 2;
	struct wlf_svg_node *icon = create_svg_icon(&button->tree->base,
		svg_source, offset, offset);
	if (icon == NULL) {
		return false;
	}
	wlf_scene_node_destroy(&button->icon->base);
	button->icon = icon;
	wlf_scene_node_raise_to_top(&button->event_node->base);
	return true;
}

static void update_default_button_icons(struct wlf_titlebar *titlebar) {
	struct wlf_theme *theme = titlebar->window->state.backend->theme;
	bool dark = theme != NULL &&
		theme->appearance == WLF_THEME_APPEARANCE_DARK;
	if (!titlebar->minimize_button.custom_icon) {
		(void)replace_button_icon(&titlebar->minimize_button, dark ?
			minimize_icon_source_dark : minimize_icon_source);
	}
	if (!titlebar->maximize_button.custom_icon) {
		(void)replace_button_icon(&titlebar->maximize_button, dark ?
			maximize_icon_source_dark : maximize_icon_source);
	}
	if (!titlebar->close_button.custom_icon) {
		(void)replace_button_icon(&titlebar->close_button, dark ?
			close_icon_source_dark : close_icon_source);
	}
}

static void arrange_button(struct wlf_titlebar_button *button,
		int button_x) {
	int y = (WLF_TITLEBAR_HEIGHT - TITLEBAR_BUTTON_SIZE) / 2;
	wlf_scene_node_set_position(&button->tree->base, button_x, y);
}

static void set_button_icon_opacity(struct wlf_titlebar_button *button,
		float opacity) {
	struct wlf_scene_node *child;
	wlf_linked_list_for_each(child, &button->icon->children, link) {
		wlf_scene_node_set_opacity(child, opacity);
	}
}

struct wlf_titlebar_button *wlf_titlebar_get_button(
		struct wlf_titlebar *titlebar,
		enum wlf_titlebar_button_type type) {
	if (titlebar == NULL) {
		return NULL;
	}
	switch (type) {
	case WLF_TITLEBAR_BUTTON_MINIMIZE:
		return &titlebar->minimize_button;
	case WLF_TITLEBAR_BUTTON_MAXIMIZE:
		return &titlebar->maximize_button;
	case WLF_TITLEBAR_BUTTON_CLOSE:
		return &titlebar->close_button;
	default:
		return NULL;
	}
}

void wlf_titlebar_arrange(struct wlf_titlebar *titlebar) {
	int width = titlebar->window->state.geometry.width;
	if (width < 0) {
		width = 0;
	}
	titlebar->background->base.state.width = width;
	titlebar->background->base.state.height = WLF_TITLEBAR_HEIGHT;
	wlf_scene_node_update(&titlebar->background->base, NULL);
	titlebar->separator->base.state.width = width;
	titlebar->separator->base.state.height = 1;
	wlf_scene_node_set_position(&titlebar->separator->base,
		0, WLF_TITLEBAR_HEIGHT - 1);
	wlf_scene_node_update(&titlebar->separator->base, NULL);
	pixman_region32_t move_region;
	pixman_region32_init_rect(&move_region, 0, 0,
		(uint32_t)width, WLF_TITLEBAR_HEIGHT);
	wlf_event_node_set_input_region(titlebar->move_event_node, &move_region);
	pixman_region32_fini(&move_region);
	int height = titlebar->window->state.geometry.height;
	if (height < 0) {
		height = 0;
	}
	uint32_t middle_width = width > TITLEBAR_RESIZE_BORDER * 2 ?
		(uint32_t)(width - TITLEBAR_RESIZE_BORDER * 2) : 0;
	uint32_t middle_height = height > TITLEBAR_RESIZE_BORDER * 2 ?
		(uint32_t)(height - TITLEBAR_RESIZE_BORDER * 2) : 0;
	struct wlf_titlebar_resize_handle *handles = titlebar->resize_handles;
	set_event_node_box(handles[0].event_node, TITLEBAR_RESIZE_BORDER, 0,
		middle_width, TITLEBAR_RESIZE_BORDER);
	set_event_node_box(handles[1].event_node, TITLEBAR_RESIZE_BORDER,
		height > TITLEBAR_RESIZE_BORDER ? height - TITLEBAR_RESIZE_BORDER : 0,
		middle_width, height > 0 ? TITLEBAR_RESIZE_BORDER : 0);
	set_event_node_box(handles[2].event_node, 0, TITLEBAR_RESIZE_BORDER,
		TITLEBAR_RESIZE_BORDER, middle_height);
	set_event_node_box(handles[3].event_node,
		width > TITLEBAR_RESIZE_BORDER ? width - TITLEBAR_RESIZE_BORDER : 0,
		TITLEBAR_RESIZE_BORDER, width > 0 ? TITLEBAR_RESIZE_BORDER : 0,
		middle_height);
	set_event_node_box(handles[4].event_node, 0, 0,
		TITLEBAR_RESIZE_BORDER, TITLEBAR_RESIZE_BORDER);
	set_event_node_box(handles[5].event_node,
		width > TITLEBAR_RESIZE_BORDER ? width - TITLEBAR_RESIZE_BORDER : 0, 0,
		width > 0 ? TITLEBAR_RESIZE_BORDER : 0, TITLEBAR_RESIZE_BORDER);
	set_event_node_box(handles[6].event_node, 0,
		height > TITLEBAR_RESIZE_BORDER ? height - TITLEBAR_RESIZE_BORDER : 0,
		TITLEBAR_RESIZE_BORDER, height > 0 ? TITLEBAR_RESIZE_BORDER : 0);
	set_event_node_box(handles[7].event_node,
		width > TITLEBAR_RESIZE_BORDER ? width - TITLEBAR_RESIZE_BORDER : 0,
		height > TITLEBAR_RESIZE_BORDER ? height - TITLEBAR_RESIZE_BORDER : 0,
		width > 0 ? TITLEBAR_RESIZE_BORDER : 0,
		height > 0 ? TITLEBAR_RESIZE_BORDER : 0);
	bool resize_enabled =
		(titlebar->window->state.flags & WLF_WINDOW_FLAG_RESIZABLE) &&
		!(titlebar->window->state.state & WLF_WINDOW_MAXIMIZED);
	for (size_t i = 0; i < WLF_TITLEBAR_RESIZE_HANDLE_COUNT; ++i) {
		wlf_scene_node_set_enabled(&handles[i].event_node->base,
			resize_enabled);
	}

	int controls_x = width - TITLEBAR_HORIZONTAL_PADDING;
	struct wlf_titlebar_button *buttons[] = {
		&titlebar->close_button,
		&titlebar->maximize_button,
		&titlebar->minimize_button,
	};
	for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
		if (!buttons[i]->visible) {
			continue;
		}
		controls_x -= TITLEBAR_BUTTON_SIZE;
		arrange_button(buttons[i], controls_x);
		controls_x -= TITLEBAR_BUTTON_GAP;
	}

	double title_area_width = controls_x - TITLEBAR_HORIZONTAL_PADDING;
	if (title_area_width < 0) {
		title_area_width = 0;
	}
	wlf_text_node_set_max_width(titlebar->title_text,
		(int)title_area_width);
	double title_x = (width - titlebar->title_text->natural_width) / 2.0;
	int content_left = TITLEBAR_HORIZONTAL_PADDING + TITLEBAR_ICON_SIZE +
		TITLEBAR_BUTTON_GAP;
	if (title_x < content_left) {
		title_x = content_left;
	}
	int title_y = (WLF_TITLEBAR_HEIGHT -
		(int)titlebar->title_text->base.state.height) / 2;
	wlf_scene_node_set_position(&titlebar->title_text->base,
		(int)title_x, title_y);
}

void wlf_titlebar_set_active(struct wlf_titlebar *titlebar, bool active) {
	struct wlf_theme *theme = titlebar->window->state.backend->theme;
	const struct wlf_color *background = active ?
		&focused_background : &unfocused_background;
	const struct wlf_color *text = active ? &focused_text : &unfocused_text;
	const struct wlf_color *separator = active ?
		&separator_focused : &separator_unfocused;
	if (theme != NULL) {
		background = &theme->palette[active ?
			WLF_THEME_COLOR_TITLEBAR_ACTIVE :
			WLF_THEME_COLOR_TITLEBAR_INACTIVE];
		text = &theme->palette[active ?
			WLF_THEME_COLOR_TITLEBAR_TEXT_ACTIVE :
			WLF_THEME_COLOR_TITLEBAR_TEXT_INACTIVE];
		separator = &theme->palette[WLF_THEME_COLOR_TITLEBAR_SEPARATOR];
	}
	titlebar->background->color = *background;
	wlf_text_node_set_color(titlebar->title_text, text);
	titlebar->separator->color = *separator;
	float icon_opacity = active ? 0.88f : 0.62f;
	set_button_icon_opacity(&titlebar->minimize_button, icon_opacity);
	set_button_icon_opacity(&titlebar->maximize_button, icon_opacity);
	set_button_icon_opacity(&titlebar->close_button, icon_opacity);
	update_button_hover(&titlebar->minimize_button);
	update_button_hover(&titlebar->maximize_button);
	update_button_hover(&titlebar->close_button);
	wlf_scene_node_update(&titlebar->background->base, NULL);
	wlf_scene_node_update(&titlebar->separator->base, NULL);
}

bool wlf_titlebar_set_icon(struct wlf_titlebar *titlebar,
		const char *svg_source) {
	struct wlf_svg_node *icon = create_svg_icon(&titlebar->content->base,
		svg_source, TITLEBAR_HORIZONTAL_PADDING,
		(WLF_TITLEBAR_HEIGHT - TITLEBAR_ICON_SIZE) / 2);
	if (icon == NULL) {
		return false;
	}
	wlf_scene_node_destroy(&titlebar->window_icon->base);
	titlebar->window_icon = icon;
	wlf_titlebar_arrange(titlebar);
	return true;
}

void wlf_titlebar_set_button_visible(struct wlf_titlebar *titlebar,
		enum wlf_titlebar_button_type type, bool visible) {
	struct wlf_titlebar_button *button = wlf_titlebar_get_button(titlebar, type);
	if (button == NULL || button->visible == visible) {
		return;
	}
	if (!visible) {
		if (titlebar->window->pointer_event_node == button->event_node) {
			struct wlf_event_pointer_focus_event event = {
				.window = titlebar->window,
				.x = titlebar->window->pointer_x,
				.y = titlebar->window->pointer_y,
			};
			wlf_event_node_notify_pointer_leave(button->event_node, &event);
			titlebar->window->pointer_event_node = NULL;
		}
		button->hovered = false;
		update_button_hover(button);
		if (titlebar->window->keyboard_event_node == button->event_node) {
			titlebar->window->keyboard_event_node = NULL;
		}
		if (titlebar->window->touch_event_node == button->event_node) {
			titlebar->window->touch_event_node = NULL;
		}
	}
	button->visible = visible;
	wlf_scene_node_set_enabled(&button->tree->base, visible);
	wlf_titlebar_arrange(titlebar);
}

bool wlf_titlebar_set_button_icon(struct wlf_titlebar *titlebar,
		enum wlf_titlebar_button_type type, const char *svg_source) {
	struct wlf_titlebar_button *button = wlf_titlebar_get_button(titlebar, type);
	if (button == NULL) {
		return false;
	}
	if (!replace_button_icon(button, svg_source)) {
		return false;
	}
	button->custom_icon = true;
	wlf_titlebar_set_active(titlebar, titlebar->window->state.focused);
	return true;
}

void wlf_titlebar_set_title(struct wlf_titlebar *titlebar,
		const char *title) {
	wlf_text_node_set_text(titlebar->title_text, title != NULL ? title : "");
	wlf_titlebar_arrange(titlebar);
}

struct wlf_scene_tree *wlf_titlebar_get_content_tree(
		struct wlf_titlebar *titlebar) {
	return titlebar != NULL ? titlebar->content : NULL;
}

static void handle_resize(struct wlf_listener *listener, void *data) {
	(void)data;
	struct wlf_titlebar *titlebar =
		wlf_container_of(listener, titlebar, listeners.resize);
	wlf_titlebar_arrange(titlebar);
}

static void handle_focus_in(struct wlf_listener *listener, void *data) {
	(void)data;
	struct wlf_titlebar *titlebar =
		wlf_container_of(listener, titlebar, listeners.focus_in);
	wlf_titlebar_set_active(titlebar, true);
}

static void handle_focus_out(struct wlf_listener *listener, void *data) {
	(void)data;
	struct wlf_titlebar *titlebar =
		wlf_container_of(listener, titlebar, listeners.focus_out);
	wlf_titlebar_set_active(titlebar, false);
}

static void handle_theme_changed(struct wlf_listener *listener, void *data) {
	(void)data;
	struct wlf_titlebar *titlebar =
		wlf_container_of(listener, titlebar, listeners.theme_changed);
	update_default_button_icons(titlebar);
	wlf_titlebar_set_active(titlebar, titlebar->window->state.focused);
}

static void handle_titlebar_pointer_button(struct wlf_listener *listener,
		void *data) {
	struct wlf_titlebar *titlebar =
		wlf_container_of(listener, titlebar, listeners.pointer_button);
	const struct wlf_pointer_button_event *event = data;
	if (event->button == WLF_POINTER_BUTTON_LEFT &&
			event->state == WLF_POINTER_BUTTON_STATE_PRESSED) {
		(void)wlf_pointer_set_cursor_shape(event->pointer,
			WLF_CURSOR_SHAPE_GRABBING);
		wlf_window_begin_move(titlebar->window, event->pointer, event->serial);
	}
}

struct wlf_titlebar *wlf_titlebar_create(struct wlf_scene_node *parent,
		struct wlf_window *window) {
	struct wlf_titlebar *titlebar = calloc(1, sizeof(*titlebar));
	if (titlebar == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_titlebar");
		return NULL;
	}
	titlebar->window = window;
	titlebar->tree = wlf_scene_tree_create(parent);
	if (titlebar->tree == NULL) {
		free(titlebar);
		return NULL;
	}

	titlebar->background = wlf_rect_node_create(&titlebar->tree->base,
		0, 0, window->state.geometry.width, WLF_TITLEBAR_HEIGHT,
		&unfocused_background);
	titlebar->separator = wlf_rect_node_create(&titlebar->tree->base,
		0, WLF_TITLEBAR_HEIGHT - 1, window->state.geometry.width, 1,
		&separator_unfocused);
	titlebar->content = wlf_scene_tree_create(&titlebar->tree->base);
	if (titlebar->content == NULL) {
		wlf_scene_node_destroy(&titlebar->tree->base);
		free(titlebar);
		return NULL;
	}
	titlebar->title_text = wlf_text_node_create(&titlebar->content->base,
		0, 0, window->state.title, "sans-serif", TITLEBAR_FONT_SIZE,
		&unfocused_text);
	titlebar->window_icon = create_svg_icon(&titlebar->content->base,
		window_icon_source, TITLEBAR_HORIZONTAL_PADDING,
		(WLF_TITLEBAR_HEIGHT - TITLEBAR_ICON_SIZE) / 2);
	titlebar->move_event_node = wlf_event_node_create(&titlebar->tree->base,
		0, 0, window->state.geometry.width, WLF_TITLEBAR_HEIGHT);
	if (titlebar->move_event_node != NULL) {
		titlebar->move_event_node->base.state.focus_policy = NO_FOCUS;
		wlf_event_node_set_cursor_shape(titlebar->move_event_node,
			WLF_CURSOR_SHAPE_GRAB);
		titlebar->listeners.pointer_button.notify =
			handle_titlebar_pointer_button;
		wlf_signal_add(&titlebar->move_event_node->events.pointer_button,
			&titlebar->listeners.pointer_button);
	}
	if (titlebar->background == NULL || titlebar->separator == NULL ||
			titlebar->window_icon == NULL ||
			titlebar->title_text == NULL ||
			titlebar->move_event_node == NULL ||
			!create_button(titlebar, &titlebar->minimize_button,
				WLF_TITLEBAR_BUTTON_MINIMIZE, minimize_icon_source) ||
			!create_button(titlebar, &titlebar->maximize_button,
				WLF_TITLEBAR_BUTTON_MAXIMIZE, maximize_icon_source) ||
			!create_button(titlebar, &titlebar->close_button,
				WLF_TITLEBAR_BUTTON_CLOSE, close_icon_source) ||
			!create_resize_handles(titlebar)) {
		remove_resize_handle_listeners(titlebar);
		remove_button_listeners(&titlebar->close_button);
		remove_button_listeners(&titlebar->maximize_button);
		remove_button_listeners(&titlebar->minimize_button);
		if (titlebar->move_event_node != NULL) {
			wlf_linked_list_remove(&titlebar->listeners.pointer_button.link);
		}
		wlf_scene_node_destroy(&titlebar->tree->base);
		free(titlebar);
		return NULL;
	}

	titlebar->listeners.resize.notify = handle_resize;
	titlebar->listeners.focus_in.notify = handle_focus_in;
	titlebar->listeners.focus_out.notify = handle_focus_out;
	wlf_signal_add(&window->events.resize, &titlebar->listeners.resize);
	wlf_signal_add(&window->events.focus_in, &titlebar->listeners.focus_in);
	wlf_signal_add(&window->events.focus_out, &titlebar->listeners.focus_out);
	if (window->state.backend->theme != NULL) {
		titlebar->listeners.theme_changed.notify = handle_theme_changed;
		wlf_signal_add(&window->state.backend->theme->events.theme_changed,
			&titlebar->listeners.theme_changed);
		titlebar->theme_listener_attached = true;
	}
	update_default_button_icons(titlebar);
	wlf_titlebar_arrange(titlebar);
	wlf_titlebar_set_active(titlebar, window->state.focused);
	return titlebar;
}

void wlf_titlebar_destroy(struct wlf_titlebar *titlebar) {
	if (titlebar == NULL) {
		return;
	}
	if (titlebar->theme_listener_attached) {
		wlf_linked_list_remove(&titlebar->listeners.theme_changed.link);
	}
	remove_button_listeners(&titlebar->close_button);
	remove_button_listeners(&titlebar->maximize_button);
	remove_button_listeners(&titlebar->minimize_button);
	remove_resize_handle_listeners(titlebar);
	wlf_linked_list_remove(&titlebar->listeners.pointer_button.link);
	wlf_linked_list_remove(&titlebar->listeners.focus_out.link);
	wlf_linked_list_remove(&titlebar->listeners.focus_in.link);
	wlf_linked_list_remove(&titlebar->listeners.resize.link);
	wlf_scene_node_destroy(&titlebar->tree->base);
	free(titlebar);
}
