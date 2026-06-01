#include "wlf/widgets/wlf_menu.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static const struct wlf_color COLOR_MENU_BG = {0.98, 0.98, 0.99, 0.96};
static const struct wlf_color COLOR_ITEM_BG = {1.0, 1.0, 1.0, 0.0};
static const struct wlf_color COLOR_ITEM_SELECTED = {0.20, 0.65, 0.98, 1.0};
static const struct wlf_color COLOR_TEXT = {0.11, 0.11, 0.12, 1.0};
static const struct wlf_color COLOR_TEXT_SELECTED = {1.0, 1.0, 1.0, 1.0};

static void menu_clear_items(struct wlf_menu *widget) {
	for (size_t i = 0; i < widget->item_count; ++i) {
		if (widget->item_backgrounds != NULL &&
				widget->item_backgrounds[i] != NULL) {
			wlf_scene_node_destroy(&widget->item_backgrounds[i]->base);
		}
		if (widget->labels != NULL && widget->labels[i] != NULL) {
			wlf_scene_node_destroy(&widget->labels[i]->node);
		}
	}

	free(widget->items);
	free(widget->item_backgrounds);
	free(widget->labels);
	widget->items = NULL;
	widget->item_backgrounds = NULL;
	widget->labels = NULL;
	widget->item_count = 0;
}

static double menu_item_height(const struct wlf_menu *widget) {
	return widget->item_count > 0 ?
		widget->base.frame.height / (double)widget->item_count :
		widget->base.frame.height;
}

static void menu_layout(struct wlf_menu *widget) {
	wlf_scene_radius_rect_set_size(widget->background,
		widget->base.frame.width, widget->base.frame.height);
	wlf_scene_radius_rect_set_corner_radii(widget->background,
		12.0, 12.0, 12.0, 12.0);

	double item_height = menu_item_height(widget);
	for (size_t i = 0; i < widget->item_count; ++i) {
		double y = (double)i * item_height;
		wlf_scene_node_set_position(&widget->item_backgrounds[i]->base,
			6.0, y + 4.0);
		wlf_scene_radius_rect_set_size(widget->item_backgrounds[i],
			widget->base.frame.width - 12.0, item_height - 8.0);
		wlf_scene_radius_rect_set_corner_radii(widget->item_backgrounds[i],
			7.0, 7.0, 7.0, 7.0);

		wlf_scene_node_set_position(&widget->labels[i]->node, 12.0, y);
		wlf_scene_text_set_box(widget->labels[i],
			widget->base.frame.width - 24.0, item_height);
		wlf_scene_text_set_vertical_alignment(widget->labels[i],
			WLF_TEXT_ALIGN_VCENTER);
		wlf_scene_text_set_horizontal_alignment(widget->labels[i],
			WLF_TEXT_ALIGN_LEFT);
	}
}

static void menu_apply_style(struct wlf_menu *widget) {
	wlf_scene_radius_rect_set_color(widget->background, &COLOR_MENU_BG);
	wlf_scene_node_set_enabled(&widget->base.tree->base,
		widget->base.enabled && widget->base.open);

	for (size_t i = 0; i < widget->item_count; ++i) {
		bool selected = widget->base.selected_index == (int)i;
		wlf_scene_radius_rect_set_color(widget->item_backgrounds[i],
			selected ? &COLOR_ITEM_SELECTED : &COLOR_ITEM_BG);
		wlf_scene_text_set_color(widget->labels[i],
			selected ? &COLOR_TEXT_SELECTED : &COLOR_TEXT);
	}
}

static void menu_destroy_impl(struct wlf_abstract_menu *base) {
	struct wlf_menu *widget = wlf_container_of(base, widget, base);

	menu_clear_items(widget);
	if (widget->base.tree != NULL) {
		wlf_scene_node_destroy(&widget->base.tree->base);
		widget->base.tree = NULL;
	}

	free(widget);
}

static void menu_set_frame_impl(struct wlf_abstract_menu *base,
		const struct wlf_frect *frame) {
	struct wlf_menu *widget = wlf_container_of(base, widget, base);
	(void)frame;
	menu_layout(widget);
}

static void menu_set_enabled_impl(struct wlf_abstract_menu *base,
		bool enabled) {
	struct wlf_menu *widget = wlf_container_of(base, widget, base);
	(void)enabled;
	menu_apply_style(widget);
}

static void menu_set_state_impl(struct wlf_abstract_menu *base,
		enum wlf_widget_state state) {
	struct wlf_menu *widget = wlf_container_of(base, widget, base);
	(void)state;
	menu_apply_style(widget);
}

static void menu_set_items_impl(struct wlf_abstract_menu *base,
		const struct wlf_menu_item *items, size_t item_count) {
	struct wlf_menu *widget = wlf_container_of(base, widget, base);

	menu_clear_items(widget);
	if (item_count == 0) {
		menu_layout(widget);
		return;
	}

	widget->items = calloc(item_count, sizeof(*widget->items));
	widget->item_backgrounds = calloc(item_count, sizeof(*widget->item_backgrounds));
	widget->labels = calloc(item_count, sizeof(*widget->labels));
	if (widget->items == NULL || widget->item_backgrounds == NULL ||
			widget->labels == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate menu items");
		menu_clear_items(widget);
		return;
	}

	memcpy(widget->items, items, item_count * sizeof(*widget->items));
	widget->item_count = item_count;
	for (size_t i = 0; i < item_count; ++i) {
		widget->item_backgrounds[i] =
			wlf_scene_radius_rect_create(widget->base.tree,
				0.0, 0.0, &COLOR_ITEM_BG);
		widget->labels[i] = wlf_scene_text_create(widget->base.tree,
			widget->items[i].text, "SF Pro Text", 13.0f, &COLOR_TEXT);
		if (widget->item_backgrounds[i] == NULL || widget->labels[i] == NULL) {
			wlf_log(WLF_ERROR, "failed to create menu item nodes");
			menu_clear_items(widget);
			return;
		}
	}

	if (widget->base.selected_index >= (int)widget->item_count) {
		widget->base.selected_index = -1;
	}
	menu_layout(widget);
	menu_apply_style(widget);
}

static void menu_set_selected_index_impl(struct wlf_abstract_menu *base,
		int selected_index) {
	struct wlf_menu *widget = wlf_container_of(base, widget, base);
	if (selected_index < -1 || selected_index >= (int)widget->item_count) {
		widget->base.selected_index = -1;
	}
	menu_apply_style(widget);
}

static void menu_set_open_impl(struct wlf_abstract_menu *base, bool open) {
	struct wlf_menu *widget = wlf_container_of(base, widget, base);
	(void)open;
	menu_apply_style(widget);
}

static const struct wlf_abstract_menu_impl menu_impl = {
	.name = "wlf_menu",
	.destroy = menu_destroy_impl,
	.set_frame = menu_set_frame_impl,
	.set_enabled = menu_set_enabled_impl,
	.set_state = menu_set_state_impl,
	.set_items = menu_set_items_impl,
	.set_selected_index = menu_set_selected_index_impl,
	.set_open = menu_set_open_impl,
};

struct wlf_menu *wlf_menu_create(struct wlf_scene_tree *parent,
		const struct wlf_frect *frame) {
	assert(frame != NULL);

	struct wlf_menu *widget = calloc(1, sizeof(*widget));
	if (widget == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_menu");
		return NULL;
	}

	wlf_abstract_menu_init(&widget->base, &menu_impl, parent, frame);
	if (widget->base.tree == NULL) {
		free(widget);
		return NULL;
	}

	widget->background = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_MENU_BG);
	if (widget->background == NULL) {
		wlf_abstract_menu_destroy(&widget->base);
		return NULL;
	}

	menu_layout(widget);
	menu_apply_style(widget);
	return widget;
}

bool wlf_abstract_menu_is_default(const struct wlf_abstract_menu *widget) {
	return widget->impl == &menu_impl;
}

struct wlf_menu *wlf_menu_from_abstract(struct wlf_abstract_menu *widget) {
	assert(widget->impl == &menu_impl);

	struct wlf_menu *menu = wlf_container_of(widget, menu, base);
	return menu;
}
