#include "wlf/widgets/wlf_slider.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static const struct wlf_color COLOR_SLIDER_TRACK = {0.88, 0.88, 0.90, 1.0};
static const struct wlf_color COLOR_SLIDER_FILL = {0.20, 0.65, 0.98, 1.0};
static const struct wlf_color COLOR_SLIDER_THUMB = {1.0, 1.0, 1.0, 1.0};

static void slider_layout(struct wlf_slider *widget) {
	double thickness = widget->base.orientation == WLF_WIDGET_HORIZONTAL ?
		widget->base.frame.height * 0.26 : widget->base.frame.width * 0.26;
	if (thickness < 4.0) {
		thickness = 4.0;
	}

	double thumb_size = widget->base.orientation == WLF_WIDGET_HORIZONTAL ?
		widget->base.frame.height : widget->base.frame.width;
	if (thumb_size < 8.0) {
		thumb_size = 8.0;
	}

	if (widget->base.orientation == WLF_WIDGET_HORIZONTAL) {
		double track_y = (widget->base.frame.height - thickness) / 2.0;
		wlf_scene_node_set_position(&widget->track->base, 0.0, track_y);
		wlf_scene_radius_rect_set_size(widget->track, widget->base.frame.width, thickness);
		wlf_scene_radius_rect_set_size(widget->fill,
			widget->base.frame.width * widget->base.normalized_value, thickness);
		wlf_scene_node_set_position(&widget->fill->base, 0.0, track_y);
		wlf_scene_node_set_position(&widget->thumb->base,
			(widget->base.frame.width - thumb_size) * widget->base.normalized_value, 0.0);
		wlf_scene_radius_rect_set_size(widget->thumb, thumb_size, thumb_size);
		wlf_scene_radius_rect_set_corner_radii(widget->track, thickness / 2.0,
			thickness / 2.0, thickness / 2.0, thickness / 2.0);
		wlf_scene_radius_rect_set_corner_radii(widget->fill, thickness / 2.0,
			thickness / 2.0, thickness / 2.0, thickness / 2.0);
	} else {
		double track_x = (widget->base.frame.width - thickness) / 2.0;
		double fill_height = widget->base.frame.height * widget->base.normalized_value;
		wlf_scene_node_set_position(&widget->track->base, track_x, 0.0);
		wlf_scene_radius_rect_set_size(widget->track, thickness, widget->base.frame.height);
		wlf_scene_node_set_position(&widget->fill->base, track_x,
			widget->base.frame.height - fill_height);
		wlf_scene_radius_rect_set_size(widget->fill, thickness, fill_height);
		wlf_scene_node_set_position(&widget->thumb->base, 0.0,
			(widget->base.frame.height - thumb_size) * (1.0 - widget->base.normalized_value));
		wlf_scene_radius_rect_set_size(widget->thumb, thumb_size, thumb_size);
		wlf_scene_radius_rect_set_corner_radii(widget->track, thickness / 2.0,
			thickness / 2.0, thickness / 2.0, thickness / 2.0);
		wlf_scene_radius_rect_set_corner_radii(widget->fill, thickness / 2.0,
			thickness / 2.0, thickness / 2.0, thickness / 2.0);
	}

	wlf_scene_radius_rect_set_corner_radii(widget->thumb, thumb_size / 2.0,
		thumb_size / 2.0, thumb_size / 2.0, thumb_size / 2.0);
}

static void slider_destroy_impl(struct wlf_abstract_slider *base) {
	struct wlf_slider *widget = wlf_container_of(base, widget, base);

	if (widget->base.tree != NULL) {
		wlf_scene_node_destroy(&widget->base.tree->base);
		widget->base.tree = NULL;
	}
	free(widget);
}

static void slider_set_frame_impl(struct wlf_abstract_slider *base,
		const struct wlf_frect *frame) {
	struct wlf_slider *widget = wlf_container_of(base, widget, base);
	(void)frame;
	slider_layout(widget);
}

static void slider_set_enabled_impl(struct wlf_abstract_slider *base, bool enabled) {
	(void)base;
	(void)enabled;
}

static void slider_set_state_impl(struct wlf_abstract_slider *base,
		enum wlf_widget_state state) {
	(void)base;
	(void)state;
}

static void slider_set_range_impl(struct wlf_abstract_slider *base,
		double minimum, double maximum) {
	struct wlf_slider *widget = wlf_container_of(base, widget, base);
	(void)minimum;
	(void)maximum;
	slider_layout(widget);
}

static void slider_set_step_impl(struct wlf_abstract_slider *base, double step) {
	(void)base;
	(void)step;
}

static void slider_set_value_impl(struct wlf_abstract_slider *base, double value) {
	struct wlf_slider *widget = wlf_container_of(base, widget, base);
	(void)value;
	slider_layout(widget);
}

static void slider_set_normalized_value_impl(struct wlf_abstract_slider *base,
		double normalized_value) {
	struct wlf_slider *widget = wlf_container_of(base, widget, base);
	(void)normalized_value;
	slider_layout(widget);
}

static const struct wlf_abstract_slider_impl slider_impl = {
	.name = "wlf_slider",
	.destroy = slider_destroy_impl,
	.set_frame = slider_set_frame_impl,
	.set_enabled = slider_set_enabled_impl,
	.set_state = slider_set_state_impl,
	.set_range = slider_set_range_impl,
	.set_step = slider_set_step_impl,
	.set_value = slider_set_value_impl,
	.set_normalized_value = slider_set_normalized_value_impl,
};

struct wlf_slider *wlf_slider_create(struct wlf_scene_tree *parent,
		const struct wlf_frect *frame, enum wlf_widget_orientation orientation) {
	assert(frame != NULL);
	struct wlf_slider *widget = calloc(1, sizeof(*widget));
	if (widget == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_slider");
		return NULL;
	}

	wlf_abstract_slider_init(&widget->base, &slider_impl, parent, frame, orientation);
	if (widget->base.tree == NULL) {
		free(widget);
		return NULL;
	}

	widget->track = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_SLIDER_TRACK);
	widget->fill = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_SLIDER_FILL);
	widget->thumb = wlf_scene_radius_rect_create(widget->base.tree,
		frame->height, frame->height, &COLOR_SLIDER_THUMB);
	if (widget->track == NULL || widget->fill == NULL || widget->thumb == NULL) {
		wlf_abstract_slider_destroy(&widget->base);
		return NULL;
	}

	widget->base.minimum = 0.0;
	widget->base.maximum = 1.0;
	widget->base.value = 0.0;
	widget->base.step = 0.0;
	widget->base.normalized_value = 0.0;
	slider_layout(widget);
	return widget;
}

bool wlf_abstract_slider_is_default(const struct wlf_abstract_slider *widget) {
	return widget->impl == &slider_impl;
}

struct wlf_slider *wlf_slider_from_abstract(struct wlf_abstract_slider *widget) {
	assert(widget->impl == &slider_impl);

	struct wlf_slider *slider = wlf_container_of(widget, slider, base);
	return slider;
}
