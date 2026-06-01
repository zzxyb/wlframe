#include "wlf/widgets/wlf_busy_indicator.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static const struct wlf_color COLOR_SPOKE_ACTIVE = {0.20, 0.20, 0.22, 1.0};
static const struct wlf_color COLOR_SPOKE_IDLE = {0.50, 0.50, 0.54, 0.35};
static const struct wlf_color COLOR_SPOKE_DISABLED = {0.76, 0.76, 0.78, 0.55};

static void busy_indicator_layout(struct wlf_busy_indicator *widget) {
	double size = widget->base.frame.width < widget->base.frame.height ?
		widget->base.frame.width : widget->base.frame.height;
	double center = size / 2.0;
	double inner = size * 0.24;
	double outer = size * 0.43;
	double stroke = size * 0.075;
	if (stroke < 1.5) {
		stroke = 1.5;
	}

	for (size_t i = 0; i < WLF_BUSY_INDICATOR_SPOKE_COUNT; ++i) {
		double angle = ((double)i / WLF_BUSY_INDICATOR_SPOKE_COUNT) *
			2.0 * M_PI;
		double x1 = center + cos(angle) * inner;
		double y1 = center + sin(angle) * inner;
		double x2 = center + cos(angle) * outer;
		double y2 = center + sin(angle) * outer;
		wlf_scene_line_set_points(widget->spokes[i], x1, y1, x2, y2);
		wlf_scene_line_set_stroke_width(widget->spokes[i], stroke);
	}
}

static void busy_indicator_apply_style(struct wlf_busy_indicator *widget) {
	size_t active = (size_t)(widget->base.progress *
		WLF_BUSY_INDICATOR_SPOKE_COUNT) % WLF_BUSY_INDICATOR_SPOKE_COUNT;

	for (size_t i = 0; i < WLF_BUSY_INDICATOR_SPOKE_COUNT; ++i) {
		struct wlf_color color = widget->base.enabled ?
			COLOR_SPOKE_IDLE : COLOR_SPOKE_DISABLED;
		if (widget->base.running && widget->base.enabled) {
			double alpha = 0.25 + 0.75 *
				(1.0 - (double)((i + WLF_BUSY_INDICATOR_SPOKE_COUNT - active) %
					WLF_BUSY_INDICATOR_SPOKE_COUNT) /
					WLF_BUSY_INDICATOR_SPOKE_COUNT);
			color = COLOR_SPOKE_ACTIVE;
			color.a = alpha;
		}
		wlf_scene_line_set_color(widget->spokes[i], &color);
	}
}

static void busy_indicator_destroy_impl(
		struct wlf_abstract_busy_indicator *base) {
	struct wlf_busy_indicator *widget = wlf_container_of(base, widget, base);

	if (widget->base.tree != NULL) {
		wlf_scene_node_destroy(&widget->base.tree->base);
		widget->base.tree = NULL;
	}

	free(widget);
}

static void busy_indicator_set_frame_impl(
		struct wlf_abstract_busy_indicator *base,
		const struct wlf_frect *frame) {
	struct wlf_busy_indicator *widget = wlf_container_of(base, widget, base);
	(void)frame;
	busy_indicator_layout(widget);
}

static void busy_indicator_set_enabled_impl(
		struct wlf_abstract_busy_indicator *base, bool enabled) {
	struct wlf_busy_indicator *widget = wlf_container_of(base, widget, base);
	(void)enabled;
	busy_indicator_apply_style(widget);
}

static void busy_indicator_set_state_impl(
		struct wlf_abstract_busy_indicator *base,
		enum wlf_widget_state state) {
	struct wlf_busy_indicator *widget = wlf_container_of(base, widget, base);
	(void)state;
	busy_indicator_apply_style(widget);
}

static void busy_indicator_set_running_impl(
		struct wlf_abstract_busy_indicator *base, bool running) {
	struct wlf_busy_indicator *widget = wlf_container_of(base, widget, base);
	(void)running;
	busy_indicator_apply_style(widget);
}

static void busy_indicator_set_progress_impl(
		struct wlf_abstract_busy_indicator *base, double progress) {
	struct wlf_busy_indicator *widget = wlf_container_of(base, widget, base);
	(void)progress;
	busy_indicator_apply_style(widget);
}

static const struct wlf_abstract_busy_indicator_impl busy_indicator_impl = {
	.name = "wlf_busy_indicator",
	.destroy = busy_indicator_destroy_impl,
	.set_frame = busy_indicator_set_frame_impl,
	.set_enabled = busy_indicator_set_enabled_impl,
	.set_state = busy_indicator_set_state_impl,
	.set_running = busy_indicator_set_running_impl,
	.set_progress = busy_indicator_set_progress_impl,
};

struct wlf_busy_indicator *wlf_busy_indicator_create(
		struct wlf_scene_tree *parent, const struct wlf_frect *frame,
		bool running) {
	assert(frame != NULL);

	struct wlf_busy_indicator *widget = calloc(1, sizeof(*widget));
	if (widget == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_busy_indicator");
		return NULL;
	}

	wlf_abstract_busy_indicator_init(&widget->base, &busy_indicator_impl,
		parent, frame);
	if (widget->base.tree == NULL) {
		free(widget);
		return NULL;
	}

	for (size_t i = 0; i < WLF_BUSY_INDICATOR_SPOKE_COUNT; ++i) {
		widget->spokes[i] = wlf_scene_line_create(widget->base.tree,
			0.0, 0.0, 0.0, 0.0, &COLOR_SPOKE_IDLE, 2.0);
		if (widget->spokes[i] == NULL) {
			wlf_abstract_busy_indicator_destroy(&widget->base);
			return NULL;
		}
	}

	widget->base.running = running;
	busy_indicator_layout(widget);
	busy_indicator_apply_style(widget);
	return widget;
}

bool wlf_abstract_busy_indicator_is_default(
		const struct wlf_abstract_busy_indicator *widget) {
	return widget->impl == &busy_indicator_impl;
}

struct wlf_busy_indicator *wlf_busy_indicator_from_abstract(
		struct wlf_abstract_busy_indicator *widget) {
	assert(widget->impl == &busy_indicator_impl);

	struct wlf_busy_indicator *indicator =
		wlf_container_of(widget, indicator, base);
	return indicator;
}
