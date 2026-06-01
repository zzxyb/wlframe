#include "wlf/widgets/wlf_abstract_busy_indicator.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static struct wlf_scene_tree *widget_scene_tree_create(
		struct wlf_scene_tree *parent) {
	return parent != NULL ? wlf_scene_tree_create(&parent->base) :
		wlf_root_scene_tree_create();
}

static double clamp01(double value) {
	if (value < 0.0) {
		return 0.0;
	}
	if (value > 1.0) {
		return 1.0;
	}
	return value;
}

void wlf_abstract_busy_indicator_init(
		struct wlf_abstract_busy_indicator *widget,
		const struct wlf_abstract_busy_indicator_impl *impl,
		struct wlf_scene_tree *parent, const struct wlf_frect *frame) {
	assert(widget != NULL);
	assert(impl != NULL);
	assert(impl->name != NULL);
	assert(impl->destroy != NULL);
	assert(frame != NULL);

	*widget = (struct wlf_abstract_busy_indicator){
		.impl = impl,
		.frame = *frame,
		.enabled = true,
		.running = false,
		.progress = 0.0,
		.state = WLF_WIDGET_STATE_NORMAL,
	};

	widget->tree = widget_scene_tree_create(parent);
	if (widget->tree == NULL) {
		wlf_log(WLF_ERROR, "failed to create busy indicator scene tree");
		return;
	}

	wlf_scene_node_set_position(&widget->tree->base, frame->x, frame->y);
	wlf_signal_init(&widget->events.destroy);
	wlf_signal_init(&widget->events.running_changed);
	wlf_signal_init(&widget->events.progress_changed);
}

void wlf_abstract_busy_indicator_destroy(
		struct wlf_abstract_busy_indicator *widget) {
	if (widget == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&widget->events.destroy, widget);
	assert(wlf_linked_list_empty(&widget->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&widget->events.running_changed.listener_list));
	assert(wlf_linked_list_empty(&widget->events.progress_changed.listener_list));

	if (widget->impl != NULL && widget->impl->destroy != NULL) {
		widget->impl->destroy(widget);
	} else {
		free(widget);
	}
}

void wlf_abstract_busy_indicator_set_frame(
		struct wlf_abstract_busy_indicator *widget,
		const struct wlf_frect *frame) {
	assert(widget != NULL);
	assert(frame != NULL);
	if (wlf_frect_equal(&widget->frame, frame)) {
		return;
	}

	widget->frame = *frame;
	wlf_scene_node_set_position(&widget->tree->base, frame->x, frame->y);
	if (widget->impl->set_frame != NULL) {
		widget->impl->set_frame(widget, frame);
	}
}

void wlf_abstract_busy_indicator_set_enabled(
		struct wlf_abstract_busy_indicator *widget, bool enabled) {
	assert(widget != NULL);
	if (widget->enabled == enabled) {
		return;
	}

	widget->enabled = enabled;
	wlf_scene_node_set_enabled(&widget->tree->base, enabled);
	if (widget->impl->set_enabled != NULL) {
		widget->impl->set_enabled(widget, enabled);
	}
}

void wlf_abstract_busy_indicator_set_state(
		struct wlf_abstract_busy_indicator *widget,
		enum wlf_widget_state state) {
	assert(widget != NULL);
	if (widget->state == state) {
		return;
	}

	widget->state = state;
	if (widget->impl->set_state != NULL) {
		widget->impl->set_state(widget, state);
	}
}

void wlf_abstract_busy_indicator_set_running(
		struct wlf_abstract_busy_indicator *widget, bool running) {
	assert(widget != NULL);
	if (widget->running == running) {
		return;
	}

	widget->running = running;
	if (widget->impl->set_running != NULL) {
		widget->impl->set_running(widget, running);
	}
	wlf_signal_emit_mutable(&widget->events.running_changed, widget);
}

void wlf_abstract_busy_indicator_set_progress(
		struct wlf_abstract_busy_indicator *widget, double progress) {
	assert(widget != NULL);
	progress = clamp01(progress);
	if (widget->progress == progress) {
		return;
	}

	widget->progress = progress;
	if (widget->impl->set_progress != NULL) {
		widget->impl->set_progress(widget, progress);
	}
	wlf_signal_emit_mutable(&widget->events.progress_changed, widget);
}
