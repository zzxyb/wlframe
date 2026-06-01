#include "wlf/widgets/wlf_checkbox.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static const struct wlf_color COLOR_CHECKBOX_FILL = {1.0, 1.0, 1.0, 1.0};
static const struct wlf_color COLOR_CHECKBOX_FILL_HOVERED = {0.97, 0.97, 0.98, 1.0};
static const struct wlf_color COLOR_CHECKBOX_ACCENT = {0.04, 0.48, 0.98, 1.0};
static const struct wlf_color COLOR_CHECKBOX_MARK = {1.0, 1.0, 1.0, 1.0};
static const struct wlf_color COLOR_CHECKBOX_DISABLED = {0.88, 0.88, 0.90, 1.0};

static void checkbox_layout(struct wlf_checkbox *widget) {
	double size = widget->base.frame.width < widget->base.frame.height ?
		widget->base.frame.width : widget->base.frame.height;
	double inset = size * 0.22;

	wlf_scene_radius_rect_set_size(widget->backend, widget->base.frame.width,
		widget->base.frame.height);
	wlf_scene_radius_rect_set_corner_radii(widget->backend, 5.0, 5.0, 5.0, 5.0);

	wlf_scene_line_set_points(widget->mark_a,
		inset, widget->base.frame.height * 0.55,
		widget->base.frame.width * 0.42, widget->base.frame.height - inset);
	wlf_scene_line_set_points(widget->mark_b,
		widget->base.frame.width * 0.42, widget->base.frame.height - inset,
		widget->base.frame.width - inset, inset);
}

static void checkbox_apply_style(struct wlf_checkbox *widget) {
	struct wlf_color fill = COLOR_CHECKBOX_FILL;
	struct wlf_color mark = WLF_COLOR_TRANSPARENT;

	if (!widget->base.enabled || widget->base.state == WLF_WIDGET_STATE_DISABLED) {
		fill = COLOR_CHECKBOX_DISABLED;
	} else if (widget->base.checked || widget->base.indeterminate) {
		fill = COLOR_CHECKBOX_ACCENT;
		mark = COLOR_CHECKBOX_MARK;
	} else if (widget->base.state == WLF_WIDGET_STATE_HOVERED) {
		fill = COLOR_CHECKBOX_FILL_HOVERED;
	}

	wlf_scene_radius_rect_set_color(widget->backend, &fill);
	wlf_scene_line_set_color(widget->mark_a, &mark);
	wlf_scene_line_set_color(widget->mark_b, &mark);
}

static void checkbox_destroy_impl(struct wlf_abstract_checkbox *base) {
	struct wlf_checkbox *widget = wlf_container_of(base, widget, base);

	if (widget->base.tree != NULL) {
		wlf_scene_node_destroy(&widget->base.tree->base);
		widget->base.tree = NULL;
	}

	free(widget);
}

static void checkbox_set_frame_impl(struct wlf_abstract_checkbox *base,
		const struct wlf_frect *frame) {
	struct wlf_checkbox *widget = wlf_container_of(base, widget, base);
	checkbox_layout(widget);
	(void)frame;
}

static void checkbox_set_enabled_impl(struct wlf_abstract_checkbox *base,
		bool enabled) {
	struct wlf_checkbox *widget = wlf_container_of(base, widget, base);
	(void)enabled;
	checkbox_apply_style(widget);
}

static void checkbox_set_state_impl(struct wlf_abstract_checkbox *base,
		enum wlf_widget_state state) {
	struct wlf_checkbox *widget = wlf_container_of(base, widget, base);
	(void)state;
	checkbox_apply_style(widget);
}

static void checkbox_set_checked_impl(struct wlf_abstract_checkbox *base,
		bool checked) {
	struct wlf_checkbox *widget = wlf_container_of(base, widget, base);
	(void)checked;
	checkbox_apply_style(widget);
}

static void checkbox_set_indeterminate_impl(struct wlf_abstract_checkbox *base,
		bool indeterminate) {
	struct wlf_checkbox *widget = wlf_container_of(base, widget, base);
	(void)indeterminate;
	checkbox_apply_style(widget);
}

static void checkbox_toggle_impl(struct wlf_abstract_checkbox *base) {
	struct wlf_checkbox *widget = wlf_container_of(base, widget, base);
	widget->base.indeterminate = false;
	widget->base.checked = !widget->base.checked;
	checkbox_apply_style(widget);
}

static const struct wlf_abstract_checkbox_impl checkbox_impl = {
	.name = "wlf_checkbox",
	.destroy = checkbox_destroy_impl,
	.set_frame = checkbox_set_frame_impl,
	.set_enabled = checkbox_set_enabled_impl,
	.set_state = checkbox_set_state_impl,
	.set_checked = checkbox_set_checked_impl,
	.set_indeterminate = checkbox_set_indeterminate_impl,
	.toggle = checkbox_toggle_impl,
};

struct wlf_checkbox *wlf_checkbox_create(struct wlf_scene_tree *parent,
		const struct wlf_frect *frame, bool checked) {
	assert(frame != NULL);

	struct wlf_checkbox *widget = calloc(1, sizeof(*widget));
	if (widget == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_checkbox");
		return NULL;
	}

	wlf_abstract_checkbox_init(&widget->base, &checkbox_impl, parent, frame);
	if (widget->base.tree == NULL) {
		free(widget);
		return NULL;
	}

	widget->backend = wlf_scene_radius_rect_create(widget->base.tree,
		frame->width, frame->height, &COLOR_CHECKBOX_FILL);
	widget->mark_a = wlf_scene_line_create(widget->base.tree, 0, 0, 0, 0,
		&COLOR_CHECKBOX_MARK, 2.2);
	widget->mark_b = wlf_scene_line_create(widget->base.tree, 0, 0, 0, 0,
		&COLOR_CHECKBOX_MARK, 2.2);
	if (widget->backend == NULL || widget->mark_a == NULL || widget->mark_b == NULL) {
		wlf_abstract_checkbox_destroy(&widget->base);
		return NULL;
	}

	widget->base.checked = checked;
	checkbox_layout(widget);
	checkbox_apply_style(widget);
	return widget;
}

bool wlf_abstract_checkbox_is_default(
		const struct wlf_abstract_checkbox *widget) {
	return widget->impl == &checkbox_impl;
}

struct wlf_checkbox *wlf_checkbox_from_abstract(
		struct wlf_abstract_checkbox *widget) {
	assert(widget->impl == &checkbox_impl);

	struct wlf_checkbox *checkbox = wlf_container_of(widget, checkbox, base);
	return checkbox;
}
