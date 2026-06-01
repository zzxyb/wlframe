#include "wlf/widgets/wlf_switch.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static const struct wlf_color COLOR_TRACK_OFF = {0.84, 0.84, 0.86, 1.0};
static const struct wlf_color COLOR_TRACK_ON = {0.20, 0.65, 0.98, 1.0};
static const struct wlf_color COLOR_TRACK_DISABLED = {0.90, 0.90, 0.92, 1.0};
static const struct wlf_color COLOR_THUMB = {1.0, 1.0, 1.0, 1.0};

static void switch_layout(struct wlf_switch *widget) {
	double thumb_size = widget->base.frame.height - 4.0;
	if (thumb_size < 0.0) {
		thumb_size = 0.0;
	}

	double travel = widget->base.frame.width - thumb_size - 4.0;
	if (travel < 0.0) {
		travel = 0.0;
	}

	double thumb_x = 2.0 + travel * widget->base.position;

	wlf_scene_radius_rect_set_size(widget->track, widget->base.frame.width,
		widget->base.frame.height);
	wlf_scene_radius_rect_set_corner_radii(widget->track,
		widget->base.frame.height / 2.0, widget->base.frame.height / 2.0,
		widget->base.frame.height / 2.0, widget->base.frame.height / 2.0);

	wlf_scene_radius_rect_set_size(widget->thumb, thumb_size, thumb_size);
	wlf_scene_radius_rect_set_corner_radii(widget->thumb,
		thumb_size / 2.0, thumb_size / 2.0, thumb_size / 2.0, thumb_size / 2.0);
	wlf_scene_node_set_position(&widget->thumb->base, thumb_x, 2.0);
}

static void switch_apply_style(struct wlf_switch *widget) {
	struct wlf_color track_color = widget->base.checked ? COLOR_TRACK_ON : COLOR_TRACK_OFF;
	if (!widget->base.enabled) {
		track_color = COLOR_TRACK_DISABLED;
	}

	wlf_scene_radius_rect_set_color(widget->track, &track_color);
	wlf_scene_radius_rect_set_color(widget->thumb, &COLOR_THUMB);
}

static void switch_destroy_impl(struct wlf_abstract_switch *base) {
	struct wlf_switch *widget = wlf_container_of(base, widget, base);

	if (widget->base.tree != NULL) {
		wlf_scene_node_destroy(&widget->base.tree->base);
		widget->base.tree = NULL;
	}

	free(widget);
}

static void switch_set_frame_impl(struct wlf_abstract_switch *base,
		const struct wlf_frect *frame) {
	struct wlf_switch *widget = wlf_container_of(base, widget, base);
	(void)frame;
	switch_layout(widget);
}

static void switch_set_enabled_impl(struct wlf_abstract_switch *base, bool enabled) {
	struct wlf_switch *widget = wlf_container_of(base, widget, base);
	(void)enabled;
	switch_apply_style(widget);
}

static void switch_set_state_impl(struct wlf_abstract_switch *base,
		enum wlf_widget_state state) {
	struct wlf_switch *widget = wlf_container_of(base, widget, base);
	(void)state;
	switch_apply_style(widget);
}

static void switch_set_checked_impl(struct wlf_abstract_switch *base, bool checked) {
	struct wlf_switch *widget = wlf_container_of(base, widget, base);
	widget->base.position = checked ? 1.0 : 0.0;
	switch_layout(widget);
	switch_apply_style(widget);
}

static void switch_set_position_impl(struct wlf_abstract_switch *base, double position) {
	struct wlf_switch *widget = wlf_container_of(base, widget, base);
	(void)position;
	switch_layout(widget);
}

static void switch_toggle_impl(struct wlf_abstract_switch *base) {
	struct wlf_switch *widget = wlf_container_of(base, widget, base);
	widget->base.checked = !widget->base.checked;
	widget->base.position = widget->base.checked ? 1.0 : 0.0;
	switch_layout(widget);
	switch_apply_style(widget);
}

static const struct wlf_abstract_switch_impl switch_impl = {
	.name = "wlf_switch",
	.destroy = switch_destroy_impl,
	.set_frame = switch_set_frame_impl,
	.set_enabled = switch_set_enabled_impl,
	.set_state = switch_set_state_impl,
	.set_checked = switch_set_checked_impl,
	.set_position = switch_set_position_impl,
	.toggle = switch_toggle_impl,
};

struct wlf_switch *wlf_switch_create(struct wlf_scene_tree *parent,
		const struct wlf_frect *frame, bool checked) {
	assert(frame != NULL);
	struct wlf_switch *widget = calloc(1, sizeof(*widget));
	if (widget == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_switch");
		return NULL;
	}

	wlf_abstract_switch_init(&widget->base, &switch_impl, parent, frame);
	if (widget->base.tree == NULL) {
		free(widget);
		return NULL;
	}

	widget->track = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_TRACK_OFF);
	widget->thumb = wlf_scene_radius_rect_create(widget->base.tree,
		frame->height - 4.0, frame->height - 4.0, &COLOR_THUMB);
	if (widget->track == NULL || widget->thumb == NULL) {
		wlf_abstract_switch_destroy(&widget->base);
		return NULL;
	}

	widget->base.checked = checked;
	widget->base.position = checked ? 1.0 : 0.0;
	switch_layout(widget);
	switch_apply_style(widget);
	return widget;
}

bool wlf_abstract_switch_is_default(const struct wlf_abstract_switch *widget) {
	return widget->impl == &switch_impl;
}

struct wlf_switch *wlf_switch_from_abstract(struct wlf_abstract_switch *widget) {
	assert(widget->impl == &switch_impl);

	struct wlf_switch *sw = wlf_container_of(widget, sw, base);
	return sw;
}
