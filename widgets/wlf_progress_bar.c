#include "wlf/widgets/wlf_progress_bar.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static const struct wlf_color COLOR_TRACK = {0.90, 0.90, 0.92, 1.0};
static const struct wlf_color COLOR_FILL = {0.20, 0.65, 0.98, 1.0};

static void progress_layout(struct wlf_progress_bar *widget) {
	wlf_scene_radius_rect_set_size(widget->track,
		widget->base.frame.width, widget->base.frame.height);
	wlf_scene_radius_rect_set_corner_radii(widget->track, widget->base.frame.height / 2.0,
		widget->base.frame.height / 2.0, widget->base.frame.height / 2.0, widget->base.frame.height / 2.0);

	if (widget->base.orientation == WLF_WIDGET_HORIZONTAL) {
		wlf_scene_radius_rect_set_size(widget->fill,
			widget->base.frame.width * widget->base.normalized_value, widget->base.frame.height);
	} else {
		double fill_height = widget->base.frame.height * widget->base.normalized_value;
		wlf_scene_radius_rect_set_size(widget->fill, widget->base.frame.width, fill_height);
		wlf_scene_node_set_position(&widget->fill->base, 0.0,
			widget->base.frame.height - fill_height);
	}

	wlf_scene_radius_rect_set_corner_radii(widget->fill, widget->base.frame.height / 2.0,
		widget->base.frame.height / 2.0, widget->base.frame.height / 2.0, widget->base.frame.height / 2.0);
}

static void progress_destroy_impl(struct wlf_abstract_progress_bar *base) {
	struct wlf_progress_bar *widget = wlf_container_of(base, widget, base);

	if (widget->base.tree != NULL) {
		wlf_scene_node_destroy(&widget->base.tree->base);
		widget->base.tree = NULL;
	}
	free(widget);
}

static void progress_set_frame_impl(struct wlf_abstract_progress_bar *base,
		const struct wlf_frect *frame) {
	struct wlf_progress_bar *widget = wlf_container_of(base, widget, base);
	(void)frame;
	progress_layout(widget);
}

static void progress_set_enabled_impl(struct wlf_abstract_progress_bar *base,
		bool enabled) {
	(void)base;
	(void)enabled;
}

static void progress_set_state_impl(struct wlf_abstract_progress_bar *base,
		enum wlf_widget_state state) {
	(void)base;
	(void)state;
}

static void progress_set_range_impl(struct wlf_abstract_progress_bar *base,
		double minimum, double maximum) {
	struct wlf_progress_bar *widget = wlf_container_of(base, widget, base);
	(void)minimum;
	(void)maximum;
	progress_layout(widget);
}

static void progress_set_value_impl(struct wlf_abstract_progress_bar *base,
		double value) {
	struct wlf_progress_bar *widget = wlf_container_of(base, widget, base);
	(void)value;
	progress_layout(widget);
}

static const struct wlf_abstract_progress_bar_impl progress_impl = {
	.name = "wlf_progress_bar",
	.destroy = progress_destroy_impl,
	.set_frame = progress_set_frame_impl,
	.set_enabled = progress_set_enabled_impl,
	.set_state = progress_set_state_impl,
	.set_range = progress_set_range_impl,
	.set_value = progress_set_value_impl,
};

struct wlf_progress_bar *wlf_progress_bar_create(struct wlf_scene_tree *parent,
		const struct wlf_frect *frame, enum wlf_widget_orientation orientation) {
	assert(frame != NULL);
	struct wlf_progress_bar *widget = calloc(1, sizeof(*widget));
	if (widget == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_progress_bar");
		return NULL;
	}

	wlf_abstract_progress_bar_init(&widget->base, &progress_impl, parent, frame,
		orientation);
	if (widget->base.tree == NULL) {
		free(widget);
		return NULL;
	}

	widget->track = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_TRACK);
	widget->fill = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_FILL);
	if (widget->track == NULL || widget->fill == NULL) {
		wlf_abstract_progress_bar_destroy(&widget->base);
		return NULL;
	}

	widget->base.minimum = 0.0;
	widget->base.maximum = 1.0;
	widget->base.value = 0.0;
	widget->base.normalized_value = 0.0;
	progress_layout(widget);
	return widget;
}

bool wlf_abstract_progress_bar_is_default(
		const struct wlf_abstract_progress_bar *widget) {
	return widget->impl == &progress_impl;
}

struct wlf_progress_bar *wlf_progress_bar_from_abstract(
		struct wlf_abstract_progress_bar *widget) {
	assert(widget->impl == &progress_impl);

	struct wlf_progress_bar *progress = wlf_container_of(widget, progress, base);
	return progress;
}
