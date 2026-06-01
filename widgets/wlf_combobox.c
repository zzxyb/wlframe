#include "wlf/widgets/wlf_combobox.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct wlf_color COLOR_FIELD = {0.97, 0.97, 0.98, 1.0};
static const struct wlf_color COLOR_FIELD_OPEN = {1.0, 1.0, 1.0, 1.0};
static const struct wlf_color COLOR_TEXT = {0.11, 0.11, 0.12, 1.0};
static const struct wlf_color COLOR_ARROW = {0.40, 0.40, 0.44, 1.0};

static void combobox_refresh_label(struct wlf_combobox *widget) {
	if (widget->base.selected_index >= 0 &&
			(size_t)widget->base.selected_index < widget->option_count) {
		wlf_scene_text_set_text(widget->label,
			widget->options[widget->base.selected_index].text);
	} else {
		wlf_scene_text_set_text(widget->label, "");
	}
}

static void combobox_clear_popup_labels(struct wlf_combobox *widget) {
	free(widget->popup_labels);
	widget->popup_labels = NULL;
}

static void combobox_rebuild_popup(struct wlf_combobox *widget) {
	combobox_clear_popup_labels(widget);

	if (widget->popup_tree == NULL || widget->option_count == 0) {
		return;
	}

	widget->popup_labels = calloc(widget->option_count, sizeof(*widget->popup_labels));
	if (widget->popup_labels == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate combobox popup labels");
		return;
	}

	for (size_t i = 0; i < widget->option_count; ++i) {
		widget->popup_labels[i] = wlf_scene_text_create(widget->popup_tree,
			widget->options[i].text, "SF Pro Text", 13.0f, &COLOR_TEXT);
		if (widget->popup_labels[i] == NULL) {
			continue;
		}

		wlf_scene_text_set_box(widget->popup_labels[i],
			widget->base.frame.width - 20.0, 22.0);
		wlf_scene_node_set_position(&widget->popup_labels[i]->node,
			10.0, 8.0 + (double)i * 24.0);
	}
}

static void combobox_layout(struct wlf_combobox *widget) {
	wlf_scene_radius_rect_set_size(widget->backend,
		widget->base.frame.width, widget->base.frame.height);
	wlf_scene_radius_rect_set_corner_radii(widget->backend, 10.0, 10.0, 10.0, 10.0);

	wlf_scene_text_set_box(widget->label,
		widget->base.frame.width - 28.0, widget->base.frame.height);
	wlf_scene_node_set_position(&widget->label->node, 10.0, 0.0);
	wlf_scene_text_set_horizontal_alignment(widget->label,
		WLF_TEXT_ALIGN_LEFT);
	wlf_scene_text_set_vertical_alignment(widget->label,
		WLF_TEXT_ALIGN_VCENTER);

	double mid_y = widget->base.frame.height / 2.0;
	double arrow_x = widget->base.frame.width - 16.0;
	wlf_scene_line_set_points(widget->arrow_a, arrow_x - 5.0, mid_y - 2.0,
		arrow_x, mid_y + 3.0);
	wlf_scene_line_set_points(widget->arrow_b, arrow_x, mid_y + 3.0,
		arrow_x + 5.0, mid_y - 2.0);

	if (widget->popup_bg != NULL) {
		double popup_height = widget->option_count > 0 ?
			(double)widget->option_count * 24.0 + 12.0 : 0.0;
		wlf_scene_node_set_position(&widget->popup_tree->base, 0.0,
			widget->base.frame.height + 6.0);
		wlf_scene_radius_rect_set_size(widget->popup_bg,
			widget->base.frame.width, popup_height);
		wlf_scene_radius_rect_set_corner_radii(widget->popup_bg, 10.0, 10.0, 10.0, 10.0);
	}
}

static void combobox_apply_style(struct wlf_combobox *widget) {
	struct wlf_color field_color = widget->base.open ? COLOR_FIELD_OPEN : COLOR_FIELD;
	wlf_scene_radius_rect_set_color(widget->backend, &field_color);
	wlf_scene_radius_rect_set_color(widget->popup_bg, &COLOR_FIELD_OPEN);
	wlf_scene_text_set_color(widget->label, &COLOR_TEXT);
	wlf_scene_line_set_color(widget->arrow_a, &COLOR_ARROW);
	wlf_scene_line_set_color(widget->arrow_b, &COLOR_ARROW);
	wlf_scene_node_set_enabled(&widget->popup_tree->base, widget->base.open);
}

static void combobox_destroy_impl(struct wlf_abstract_combobox *base) {
	struct wlf_combobox *widget = wlf_container_of(base, widget, base);

	free(widget->options);
	combobox_clear_popup_labels(widget);
	if (widget->base.tree != NULL) {
		wlf_scene_node_destroy(&widget->base.tree->base);
		widget->base.tree = NULL;
	}
	free(widget);
}

static void combobox_set_frame_impl(struct wlf_abstract_combobox *base,
		const struct wlf_frect *frame) {
	struct wlf_combobox *widget = wlf_container_of(base, widget, base);
	(void)frame;
	combobox_layout(widget);
	combobox_rebuild_popup(widget);
}

static void combobox_set_enabled_impl(struct wlf_abstract_combobox *base,
		bool enabled) {
	(void)base;
	(void)enabled;
}

static void combobox_set_state_impl(struct wlf_abstract_combobox *base,
		enum wlf_widget_state state) {
	(void)base;
	(void)state;
}

static void combobox_set_options_impl(struct wlf_abstract_combobox *base,
		const struct wlf_combobox_option *options, size_t option_count) {
	struct wlf_combobox *widget = wlf_container_of(base, widget, base);

	free(widget->options);
	widget->options = NULL;
	widget->option_count = 0;

	if (option_count > 0) {
		widget->options = calloc(option_count, sizeof(*widget->options));
		if (widget->options == NULL) {
			wlf_log_errno(WLF_ERROR, "failed to allocate combobox options");
			return;
		}

		memcpy(widget->options, options, option_count * sizeof(*widget->options));
		widget->option_count = option_count;
	}

	if (widget->base.selected_index >= (int)widget->option_count) {
		widget->base.selected_index = -1;
	}

	combobox_rebuild_popup(widget);
	combobox_layout(widget);
	combobox_refresh_label(widget);
}

static void combobox_set_selected_index_impl(struct wlf_abstract_combobox *base,
		int selected_index) {
	struct wlf_combobox *widget = wlf_container_of(base, widget, base);
	if (selected_index < -1 || selected_index >= (int)widget->option_count) {
		widget->base.selected_index = -1;
	}

	combobox_refresh_label(widget);
}

static void combobox_set_open_impl(struct wlf_abstract_combobox *base, bool open) {
	struct wlf_combobox *widget = wlf_container_of(base, widget, base);
	(void)open;
	combobox_apply_style(widget);
}

static const struct wlf_abstract_combobox_impl combobox_impl = {
	.name = "wlf_combobox",
	.destroy = combobox_destroy_impl,
	.set_frame = combobox_set_frame_impl,
	.set_enabled = combobox_set_enabled_impl,
	.set_state = combobox_set_state_impl,
	.set_options = combobox_set_options_impl,
	.set_selected_index = combobox_set_selected_index_impl,
	.set_open = combobox_set_open_impl,
};

struct wlf_combobox *wlf_combobox_create(struct wlf_scene_tree *parent,
		const struct wlf_frect *frame) {
	assert(frame != NULL);
	struct wlf_combobox *widget = calloc(1, sizeof(*widget));
	if (widget == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_combobox");
		return NULL;
	}

	wlf_abstract_combobox_init(&widget->base, &combobox_impl, parent, frame);
	if (widget->base.tree == NULL) {
		free(widget);
		return NULL;
	}

	widget->backend = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_FIELD);
	widget->label = wlf_scene_text_create(widget->base.tree, "",
		"SF Pro Text", 13.0f, &COLOR_TEXT);
	widget->arrow_a = wlf_scene_line_create(widget->base.tree, 0, 0, 0, 0,
		&COLOR_ARROW, 1.8);
	widget->arrow_b = wlf_scene_line_create(widget->base.tree, 0, 0, 0, 0,
		&COLOR_ARROW, 1.8);
	widget->popup_tree = wlf_scene_tree_create(&widget->base.tree->base);
	if (widget->popup_tree != NULL) {
		widget->popup_bg = wlf_scene_radius_rect_create(widget->popup_tree,
			frame->width, 0.0, &COLOR_FIELD_OPEN);
	}

	if (widget->backend == NULL || widget->label == NULL ||
			widget->arrow_a == NULL || widget->arrow_b == NULL ||
			widget->popup_tree == NULL || widget->popup_bg == NULL) {
		wlf_abstract_combobox_destroy(&widget->base);
		return NULL;
	}

	widget->base.selected_index = -1;
	combobox_layout(widget);
	combobox_apply_style(widget);
	return widget;
}

bool wlf_abstract_combobox_is_default(
		const struct wlf_abstract_combobox *widget) {
	return widget->impl == &combobox_impl;
}

struct wlf_combobox *wlf_combobox_from_abstract(
		struct wlf_abstract_combobox *widget) {
	assert(widget->impl == &combobox_impl);

	struct wlf_combobox *combobox = wlf_container_of(widget, combobox, base);
	return combobox;
}
