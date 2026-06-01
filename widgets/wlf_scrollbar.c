#include "wlf/widgets/wlf_scrollbar.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static const struct wlf_color COLOR_TRACK = {0.86, 0.86, 0.88, 0.45};
static const struct wlf_color COLOR_THUMB = {0.45, 0.45, 0.49, 0.65};
static const struct wlf_color COLOR_THUMB_DISABLED = {0.72, 0.72, 0.74, 0.45};

static double scrollbar_visible_fraction(struct wlf_scrollbar *widget) {
	double range = widget->base.maximum - widget->base.minimum;
	if (range <= 0.0) {
		return 1.0;
	}

	double fraction = widget->base.page_size / (range + widget->base.page_size);
	if (fraction < 0.08) {
		fraction = 0.08;
	} else if (fraction > 1.0) {
		fraction = 1.0;
	}
	return fraction;
}

static void scrollbar_layout(struct wlf_scrollbar *widget) {
	double radius = widget->base.orientation == WLF_WIDGET_HORIZONTAL ?
		widget->base.frame.height / 2.0 : widget->base.frame.width / 2.0;
	double fraction = scrollbar_visible_fraction(widget);

	wlf_scene_radius_rect_set_size(widget->track, widget->base.frame.width,
		widget->base.frame.height);
	wlf_scene_radius_rect_set_corner_radii(widget->track, radius, radius,
		radius, radius);

	if (widget->base.orientation == WLF_WIDGET_HORIZONTAL) {
		double thumb_width = widget->base.frame.width * fraction;
		double travel = widget->base.frame.width - thumb_width;
		wlf_scene_radius_rect_set_size(widget->thumb, thumb_width,
			widget->base.frame.height);
		wlf_scene_node_set_position(&widget->thumb->base,
			travel * widget->base.normalized_value, 0.0);
		wlf_scene_radius_rect_set_corner_radii(widget->thumb, radius, radius,
			radius, radius);
	} else {
		double thumb_height = widget->base.frame.height * fraction;
		double travel = widget->base.frame.height - thumb_height;
		wlf_scene_radius_rect_set_size(widget->thumb, widget->base.frame.width,
			thumb_height);
		wlf_scene_node_set_position(&widget->thumb->base, 0.0,
			travel * widget->base.normalized_value);
		wlf_scene_radius_rect_set_corner_radii(widget->thumb, radius, radius,
			radius, radius);
	}
}

static void scrollbar_apply_style(struct wlf_scrollbar *widget) {
	struct wlf_color thumb = widget->base.enabled ? COLOR_THUMB :
		COLOR_THUMB_DISABLED;
	wlf_scene_radius_rect_set_color(widget->track, &COLOR_TRACK);
	wlf_scene_radius_rect_set_color(widget->thumb, &thumb);
}

static void scrollbar_destroy_impl(struct wlf_abstract_scrollbar *base) {
	struct wlf_scrollbar *widget = wlf_container_of(base, widget, base);

	if (widget->base.tree != NULL) {
		wlf_scene_node_destroy(&widget->base.tree->base);
		widget->base.tree = NULL;
	}

	free(widget);
}

static void scrollbar_set_frame_impl(struct wlf_abstract_scrollbar *base,
		const struct wlf_frect *frame) {
	struct wlf_scrollbar *widget = wlf_container_of(base, widget, base);
	(void)frame;
	scrollbar_layout(widget);
}

static void scrollbar_set_enabled_impl(struct wlf_abstract_scrollbar *base,
		bool enabled) {
	struct wlf_scrollbar *widget = wlf_container_of(base, widget, base);
	(void)enabled;
	scrollbar_apply_style(widget);
}

static void scrollbar_set_state_impl(struct wlf_abstract_scrollbar *base,
		enum wlf_widget_state state) {
	struct wlf_scrollbar *widget = wlf_container_of(base, widget, base);
	(void)state;
	scrollbar_apply_style(widget);
}

static void scrollbar_set_range_impl(struct wlf_abstract_scrollbar *base,
		double minimum, double maximum) {
	struct wlf_scrollbar *widget = wlf_container_of(base, widget, base);
	(void)minimum;
	(void)maximum;
	scrollbar_layout(widget);
}

static void scrollbar_set_page_size_impl(struct wlf_abstract_scrollbar *base,
		double page_size) {
	struct wlf_scrollbar *widget = wlf_container_of(base, widget, base);
	(void)page_size;
	scrollbar_layout(widget);
}

static void scrollbar_set_value_impl(struct wlf_abstract_scrollbar *base,
		double value) {
	struct wlf_scrollbar *widget = wlf_container_of(base, widget, base);
	(void)value;
	scrollbar_layout(widget);
}

static void scrollbar_set_normalized_value_impl(
		struct wlf_abstract_scrollbar *base, double normalized_value) {
	struct wlf_scrollbar *widget = wlf_container_of(base, widget, base);
	(void)normalized_value;
	scrollbar_layout(widget);
}

static const struct wlf_abstract_scrollbar_impl scrollbar_impl = {
	.name = "wlf_scrollbar",
	.destroy = scrollbar_destroy_impl,
	.set_frame = scrollbar_set_frame_impl,
	.set_enabled = scrollbar_set_enabled_impl,
	.set_state = scrollbar_set_state_impl,
	.set_range = scrollbar_set_range_impl,
	.set_page_size = scrollbar_set_page_size_impl,
	.set_value = scrollbar_set_value_impl,
	.set_normalized_value = scrollbar_set_normalized_value_impl,
};

struct wlf_scrollbar *wlf_scrollbar_create(struct wlf_scene_tree *parent,
		const struct wlf_frect *frame,
		enum wlf_widget_orientation orientation) {
	assert(frame != NULL);

	struct wlf_scrollbar *widget = calloc(1, sizeof(*widget));
	if (widget == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_scrollbar");
		return NULL;
	}

	wlf_abstract_scrollbar_init(&widget->base, &scrollbar_impl, parent, frame,
		orientation);
	if (widget->base.tree == NULL) {
		free(widget);
		return NULL;
	}

	widget->track = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_TRACK);
	widget->thumb = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_THUMB);
	if (widget->track == NULL || widget->thumb == NULL) {
		wlf_abstract_scrollbar_destroy(&widget->base);
		return NULL;
	}

	scrollbar_layout(widget);
	scrollbar_apply_style(widget);
	return widget;
}

bool wlf_abstract_scrollbar_is_default(
		const struct wlf_abstract_scrollbar *widget) {
	return widget->impl == &scrollbar_impl;
}

struct wlf_scrollbar *wlf_scrollbar_from_abstract(
		struct wlf_abstract_scrollbar *widget) {
	assert(widget->impl == &scrollbar_impl);

	struct wlf_scrollbar *scrollbar = wlf_container_of(widget, scrollbar, base);
	return scrollbar;
}
