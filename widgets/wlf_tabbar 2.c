#include "wlf/widgets/wlf_tabbar.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static const struct wlf_color COLOR_BG = {0.90, 0.90, 0.92, 1.0};
static const struct wlf_color COLOR_SEGMENT = {1.0, 1.0, 1.0, 0.0};
static const struct wlf_color COLOR_SEGMENT_SELECTED = {1.0, 1.0, 1.0, 1.0};
static const struct wlf_color COLOR_TEXT = {0.12, 0.12, 0.13, 1.0};
static const struct wlf_color COLOR_TEXT_DISABLED = {0.58, 0.58, 0.60, 1.0};

static void tabbar_clear_items(struct wlf_tabbar *widget) {
	for (size_t i = 0; i < widget->item_count; ++i) {
		if (widget->segments != NULL && widget->segments[i] != NULL) {
			wlf_scene_node_destroy(&widget->segments[i]->base);
		}
		if (widget->labels != NULL && widget->labels[i] != NULL) {
			wlf_scene_node_destroy(&widget->labels[i]->node);
		}
	}

	free(widget->items);
	free(widget->segments);
	free(widget->labels);
	widget->items = NULL;
	widget->segments = NULL;
	widget->labels = NULL;
	widget->item_count = 0;
}

static double tab_width(const struct wlf_tabbar *widget) {
	return widget->item_count > 0 ?
		widget->base.frame.width / (double)widget->item_count :
		widget->base.frame.width;
}

static void tabbar_layout(struct wlf_tabbar *widget) {
	wlf_scene_radius_rect_set_size(widget->background,
		widget->base.frame.width, widget->base.frame.height);
	wlf_scene_radius_rect_set_corner_radii(widget->background,
		10.0, 10.0, 10.0, 10.0);

	double width = tab_width(widget);
	for (size_t i = 0; i < widget->item_count; ++i) {
		double x = (double)i * width;
		wlf_scene_node_set_position(&widget->segments[i]->base, x + 3.0, 3.0);
		wlf_scene_radius_rect_set_size(widget->segments[i],
			width - 6.0, widget->base.frame.height - 6.0);
		wlf_scene_radius_rect_set_corner_radii(widget->segments[i],
			8.0, 8.0, 8.0, 8.0);

		wlf_scene_node_set_position(&widget->labels[i]->node, x, 0.0);
		wlf_scene_text_set_box(widget->labels[i],
			width, widget->base.frame.height);
		wlf_scene_text_set_vertical_alignment(widget->labels[i],
			WLF_TEXT_ALIGN_VCENTER);
		wlf_scene_text_set_horizontal_alignment(widget->labels[i],
			WLF_TEXT_ALIGN_CENTER);
	}
}

static void tabbar_apply_style(struct wlf_tabbar *widget) {
	wlf_scene_radius_rect_set_color(widget->background, &COLOR_BG);

	for (size_t i = 0; i < widget->item_count; ++i) {
		bool selected = widget->base.selected_index == (int)i;
		wlf_scene_radius_rect_set_color(widget->segments[i],
			selected ? &COLOR_SEGMENT_SELECTED : &COLOR_SEGMENT);
		wlf_scene_text_set_color(widget->labels[i],
			widget->items[i].enabled ? &COLOR_TEXT : &COLOR_TEXT_DISABLED);
	}
}

static void tabbar_destroy_impl(struct wlf_abstract_tabbar *base) {
	struct wlf_tabbar *widget = wlf_container_of(base, widget, base);

	tabbar_clear_items(widget);
	if (widget->base.tree != NULL) {
		wlf_scene_node_destroy(&widget->base.tree->base);
		widget->base.tree = NULL;
	}

	free(widget);
}

static void tabbar_set_frame_impl(struct wlf_abstract_tabbar *base,
		const struct wlf_frect *frame) {
	struct wlf_tabbar *widget = wlf_container_of(base, widget, base);
	(void)frame;
	tabbar_layout(widget);
}

static void tabbar_set_enabled_impl(struct wlf_abstract_tabbar *base,
		bool enabled) {
	struct wlf_tabbar *widget = wlf_container_of(base, widget, base);
	(void)enabled;
	tabbar_apply_style(widget);
}

static void tabbar_set_state_impl(struct wlf_abstract_tabbar *base,
		enum wlf_widget_state state) {
	struct wlf_tabbar *widget = wlf_container_of(base, widget, base);
	(void)state;
	tabbar_apply_style(widget);
}

static void tabbar_set_items_impl(struct wlf_abstract_tabbar *base,
		const struct wlf_tabbar_item *items, size_t item_count) {
	struct wlf_tabbar *widget = wlf_container_of(base, widget, base);

	tabbar_clear_items(widget);
	if (item_count == 0) {
		tabbar_layout(widget);
		return;
	}

	widget->items = calloc(item_count, sizeof(*widget->items));
	widget->segments = calloc(item_count, sizeof(*widget->segments));
	widget->labels = calloc(item_count, sizeof(*widget->labels));
	if (widget->items == NULL || widget->segments == NULL ||
			widget->labels == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate tabbar items");
		tabbar_clear_items(widget);
		return;
	}

	memcpy(widget->items, items, item_count * sizeof(*widget->items));
	widget->item_count = item_count;
	for (size_t i = 0; i < item_count; ++i) {
		widget->segments[i] = wlf_scene_radius_rect_create(widget->base.tree,
			0.0, 0.0, &COLOR_SEGMENT);
		widget->labels[i] = wlf_scene_text_create(widget->base.tree,
			widget->items[i].text, "SF Pro Text", 13.0f, &COLOR_TEXT);
		if (widget->segments[i] == NULL || widget->labels[i] == NULL) {
			wlf_log(WLF_ERROR, "failed to create tabbar item nodes");
			tabbar_clear_items(widget);
			return;
		}
	}

	if (widget->base.selected_index >= (int)widget->item_count) {
		widget->base.selected_index = -1;
	}
	tabbar_layout(widget);
	tabbar_apply_style(widget);
}

static void tabbar_set_selected_index_impl(struct wlf_abstract_tabbar *base,
		int selected_index) {
	struct wlf_tabbar *widget = wlf_container_of(base, widget, base);
	if (selected_index < -1 || selected_index >= (int)widget->item_count) {
		widget->base.selected_index = -1;
	}
	tabbar_apply_style(widget);
}

static const struct wlf_abstract_tabbar_impl tabbar_impl = {
	.name = "wlf_tabbar",
	.destroy = tabbar_destroy_impl,
	.set_frame = tabbar_set_frame_impl,
	.set_enabled = tabbar_set_enabled_impl,
	.set_state = tabbar_set_state_impl,
	.set_items = tabbar_set_items_impl,
	.set_selected_index = tabbar_set_selected_index_impl,
};

struct wlf_tabbar *wlf_tabbar_create(struct wlf_scene_tree *parent,
		const struct wlf_frect *frame) {
	assert(frame != NULL);

	struct wlf_tabbar *widget = calloc(1, sizeof(*widget));
	if (widget == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_tabbar");
		return NULL;
	}

	wlf_abstract_tabbar_init(&widget->base, &tabbar_impl, parent, frame);
	if (widget->base.tree == NULL) {
		free(widget);
		return NULL;
	}

	widget->background = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_BG);
	if (widget->background == NULL) {
		wlf_abstract_tabbar_destroy(&widget->base);
		return NULL;
	}

	tabbar_layout(widget);
	tabbar_apply_style(widget);
	return widget;
}

bool wlf_abstract_tabbar_is_default(
		const struct wlf_abstract_tabbar *widget) {
	return widget->impl == &tabbar_impl;
}

struct wlf_tabbar *wlf_tabbar_from_abstract(
		struct wlf_abstract_tabbar *widget) {
	assert(widget->impl == &tabbar_impl);

	struct wlf_tabbar *tabbar = wlf_container_of(widget, tabbar, base);
	return tabbar;
}
