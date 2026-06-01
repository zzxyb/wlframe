#include "wlf/widgets/wlf_abstract_progress_bar.h"

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

void wlf_abstract_progress_bar_init(struct wlf_abstract_progress_bar *widget,
		const struct wlf_abstract_progress_bar_impl *impl,
		struct wlf_scene_tree *parent, const struct wlf_frect *frame,
		enum wlf_widget_orientation orientation) {
	assert(widget != NULL);
	assert(impl != NULL);
	assert(impl->name != NULL);
	assert(impl->destroy != NULL);
	assert(frame != NULL);

	*widget = (struct wlf_abstract_progress_bar){
		.impl = impl,
		.frame = *frame,
		.orientation = orientation,
		.enabled = true,
		.state = WLF_WIDGET_STATE_NORMAL,
	};

	widget->tree = widget_scene_tree_create(parent);
	if (widget->tree == NULL) {
		wlf_log(WLF_ERROR, "failed to create progress bar scene tree");
		return;
	}

	wlf_scene_node_set_position(&widget->tree->base, frame->x, frame->y);
	wlf_signal_init(&widget->events.destroy);
	wlf_signal_init(&widget->events.value_changed);
}

void wlf_abstract_progress_bar_destroy(struct wlf_abstract_progress_bar *widget) {
	if (widget == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&widget->events.destroy, widget);
	assert(wlf_linked_list_empty(&widget->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&widget->events.value_changed.listener_list));

	if (widget->impl != NULL && widget->impl->destroy != NULL) {
		widget->impl->destroy(widget);
	} else {
		free(widget);
	}
}

void wlf_abstract_progress_bar_set_frame(struct wlf_abstract_progress_bar *widget,
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

void wlf_abstract_progress_bar_set_enabled(
		struct wlf_abstract_progress_bar *widget, bool enabled) {
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

void wlf_abstract_progress_bar_set_state(struct wlf_abstract_progress_bar *widget,
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

void wlf_abstract_progress_bar_set_range(struct wlf_abstract_progress_bar *widget,
		double minimum, double maximum) {
	assert(widget != NULL);
	if (maximum < minimum) {
		double tmp = minimum;
		minimum = maximum;
		maximum = tmp;
	}

	widget->minimum = minimum;
	widget->maximum = maximum;
	if (widget->impl->set_range != NULL) {
		widget->impl->set_range(widget, minimum, maximum);
	}
}

void wlf_abstract_progress_bar_set_value(struct wlf_abstract_progress_bar *widget,
		double value) {
	assert(widget != NULL);
	if (value < widget->minimum) {
		value = widget->minimum;
	} else if (value > widget->maximum) {
		value = widget->maximum;
	}
	if (widget->value == value) {
		return;
	}

	widget->value = value;
	if (widget->maximum > widget->minimum) {
		widget->normalized_value = clamp01(
			(value - widget->minimum) / (widget->maximum - widget->minimum));
	} else {
		widget->normalized_value = 0.0;
	}

	if (widget->impl->set_value != NULL) {
		widget->impl->set_value(widget, value);
	}

	struct wlf_abstract_progress_bar_value_event event = {
		.widget = widget,
		.value = widget->value,
		.normalized_value = widget->normalized_value,
	};
	wlf_signal_emit_mutable(&widget->events.value_changed, &event);
}
