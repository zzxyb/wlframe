#include "wlf/window/wlf_titlebar.h"

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
	TITLEBAR_BUTTON_GAP = 2,
	TITLEBAR_BUTTON_SIZE = 24,
	TITLEBAR_ICON_SIZE = 16,
	TITLEBAR_FONT_SIZE = 16,
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

static const char minimize_icon_source[] =
	"<svg width='16' height='16' viewBox='0 0 16 16'>"
	"<path d='M3 8 L13 8' fill='none' stroke='#202124' "
		"stroke-width='1.5' stroke-linecap='round'/>"
	"</svg>";

static const char maximize_icon_source[] =
	"<svg width='16' height='16' viewBox='0 0 16 16'>"
	"<rect x='3.5' y='3.5' width='9' height='9' rx='1' fill='none' "
		"stroke='#202124' stroke-width='1.25'/>"
	"</svg>";

static const char close_icon_source[] =
	"<svg width='16' height='16' viewBox='0 0 16 16'>"
	"<path d='M4 4 L12 12 M12 4 L4 12' fill='none' stroke='#202124' "
		"stroke-width='1.5' stroke-linecap='round'/>"
	"</svg>";

static bool create_button(struct wlf_titlebar_button *button,
		struct wlf_scene_node *parent, const char *icon_source) {
	button->tree = wlf_scene_tree_create(parent);
	if (button->tree == NULL) {
		return false;
	}

	struct wlf_color transparent = {0};
	button->background = wlf_rect_node_create(&button->tree->base,
		0, 0, TITLEBAR_BUTTON_SIZE, TITLEBAR_BUTTON_SIZE, &transparent);
	if (button->background == NULL) {
		return false;
	}

	char *input = strdup(icon_source);
	if (input == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to duplicate titlebar icon SVG");
		return false;
	}
	struct wlf_svg_image *image = wlf_svg_parse(input, "px", 96.0f);
	free(input);
	if (image == NULL) {
		return false;
	}

	double icon_offset =
		(TITLEBAR_BUTTON_SIZE - TITLEBAR_ICON_SIZE) / 2.0;
	button->icon = wlf_svg_node_create(&button->tree->base,
		icon_offset, icon_offset, image);
	if (button->icon == NULL) {
		wlf_svg_destroy(image);
		return false;
	}
	return true;
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

void wlf_titlebar_arrange(struct wlf_titlebar *titlebar) {
	int width = titlebar->window->state.geometry.width;
	titlebar->background->base.state.width = width;
	titlebar->background->base.state.height = WLF_TITLEBAR_HEIGHT;
	wlf_scene_node_update(&titlebar->background->base, NULL);
	titlebar->separator->base.state.width = width;
	titlebar->separator->base.state.height = 1;
	wlf_scene_node_set_position(&titlebar->separator->base,
		0, WLF_TITLEBAR_HEIGHT - 1);
	wlf_scene_node_update(&titlebar->separator->base, NULL);

	int close_x = width - TITLEBAR_HORIZONTAL_PADDING -
		TITLEBAR_BUTTON_SIZE;
	int maximize_x = close_x - TITLEBAR_BUTTON_GAP -
		TITLEBAR_BUTTON_SIZE;
	int minimize_x = maximize_x - TITLEBAR_BUTTON_GAP -
		TITLEBAR_BUTTON_SIZE;
	arrange_button(&titlebar->close_button, close_x);
	arrange_button(&titlebar->maximize_button, maximize_x);
	arrange_button(&titlebar->minimize_button, minimize_x);

	double title_area_width = minimize_x - TITLEBAR_HORIZONTAL_PADDING;
	if (title_area_width < 0) {
		title_area_width = 0;
	}
	wlf_text_node_set_max_width(titlebar->title_text,
		(int)title_area_width);
	double title_x = (width - titlebar->title_text->natural_width) / 2.0;
	if (title_x < TITLEBAR_HORIZONTAL_PADDING) {
		title_x = TITLEBAR_HORIZONTAL_PADDING;
	}
	int title_y = (WLF_TITLEBAR_HEIGHT -
		(int)titlebar->title_text->base.state.height) / 2;
	wlf_scene_node_set_position(&titlebar->title_text->base,
		(int)title_x, title_y);
}

void wlf_titlebar_set_active(struct wlf_titlebar *titlebar, bool active) {
	titlebar->background->color = active ?
		focused_background : unfocused_background;
	const struct wlf_color *text = active ? &focused_text : &unfocused_text;
	wlf_text_node_set_color(titlebar->title_text, text);
	titlebar->separator->color = active ?
		separator_focused : separator_unfocused;
	float icon_opacity = active ? 0.88f : 0.62f;
	set_button_icon_opacity(&titlebar->minimize_button, icon_opacity);
	set_button_icon_opacity(&titlebar->maximize_button, icon_opacity);
	set_button_icon_opacity(&titlebar->close_button, icon_opacity);
	wlf_scene_node_update(&titlebar->background->base, NULL);
	wlf_scene_node_update(&titlebar->separator->base, NULL);
}

void wlf_titlebar_set_title(struct wlf_titlebar *titlebar,
		const char *title) {
	wlf_text_node_set_text(titlebar->title_text, title != NULL ? title : "");
	wlf_titlebar_arrange(titlebar);
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
	titlebar->title_text = wlf_text_node_create(&titlebar->tree->base,
		0, 0, window->state.title, "sans-serif", TITLEBAR_FONT_SIZE,
		&unfocused_text);
	if (titlebar->background == NULL || titlebar->separator == NULL ||
			titlebar->title_text == NULL ||
			!create_button(&titlebar->minimize_button,
				&titlebar->tree->base, minimize_icon_source) ||
			!create_button(&titlebar->maximize_button,
				&titlebar->tree->base, maximize_icon_source) ||
			!create_button(&titlebar->close_button,
				&titlebar->tree->base, close_icon_source)) {
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
	wlf_titlebar_arrange(titlebar);
	wlf_titlebar_set_active(titlebar, window->state.focused);
	return titlebar;
}

void wlf_titlebar_destroy(struct wlf_titlebar *titlebar) {
	if (titlebar == NULL) {
		return;
	}
	wlf_linked_list_remove(&titlebar->listeners.focus_out.link);
	wlf_linked_list_remove(&titlebar->listeners.focus_in.link);
	wlf_linked_list_remove(&titlebar->listeners.resize.link);
	wlf_scene_node_destroy(&titlebar->tree->base);
	free(titlebar);
}
