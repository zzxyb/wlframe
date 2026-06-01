#include "wlf/widgets/wlf_abstract_switch.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static struct wlf_scene_tree *widget_scene_tree_create(
		struct wlf_scene_tree *parent) {
	return parent != NULL ? wlf_scene_tree_create(&parent->base) :
		wlf_root_scene_tree_create();
}

void wlf_abstract_switch_init(struct wlf_abstract_switch *widget,
		const struct wlf_abstract_switch_impl *impl, struct wlf_scene_tree *parent,
		const struct wlf_frect *frame) {
	assert(widget != NULL);
	assert(impl != NULL);
	assert(impl->name != NULL);
	assert(impl->destroy != NULL);
	assert(frame != NULL);

	*widget = (struct wlf_abstract_switch){
		.impl = impl,
		.frame = *frame,
		.enabled = true,
		.state = WLF_WIDGET_STATE_NORMAL,
	};

	widget->tree = widget_scene_tree_create(parent);
	if (widget->tree == NULL) {
		wlf_log(WLF_ERROR, "failed to create switch scene tree");
		return;
	}

	wlf_scene_node_set_position(&widget->tree->base, frame->x, frame->y);
	wlf_signal_init(&widget->events.destroy);
	wlf_signal_init(&widget->events.toggled);
}

void wlf_abstract_switch_destroy(struct wlf_abstract_switch *widget) {
	if (widget == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&widget->events.destroy, widget);
	assert(wlf_linked_list_empty(&widget->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&widget->events.toggled.listener_list));

	if (widget->impl != NULL && widget->impl->destroy != NULL) {
		widget->impl->destroy(widget);
	} else {
		free(widget);
	}
}

void wlf_abstract_switch_set_frame(struct wlf_abstract_switch *widget,
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

void wlf_abstract_switch_set_enabled(struct wlf_abstract_switch *widget,
		bool enabled) {
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

void wlf_abstract_switch_set_state(struct wlf_abstract_switch *widget,
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

void wlf_abstract_switch_set_checked(struct wlf_abstract_switch *widget,
		bool checked) {
	assert(widget != NULL);
	if (widget->checked == checked) {
		return;
	}

	widget->checked = checked;
	if (widget->impl->set_checked != NULL) {
		widget->impl->set_checked(widget, checked);
	}

	struct wlf_abstract_switch_value_event event = {
		.widget = widget,
		.checked = widget->checked,
		.position = widget->position,
	};
	wlf_signal_emit_mutable(&widget->events.toggled, &event);
}

void wlf_abstract_switch_set_position(struct wlf_abstract_switch *widget,
		double position) {
	assert(widget != NULL);
	if (position < 0.0) {
		position = 0.0;
	} else if (position > 1.0) {
		position = 1.0;
	}
	if (widget->position == position) {
		return;
	}

	widget->position = position;
	widget->checked = position >= 0.5;
	if (widget->impl->set_position != NULL) {
		widget->impl->set_position(widget, position);
	}
}

void wlf_abstract_switch_toggle(struct wlf_abstract_switch *widget) {
	assert(widget != NULL);
	if (!widget->enabled) {
		return;
	}

	if (widget->impl->toggle != NULL) {
		widget->impl->toggle(widget);
	}

	struct wlf_abstract_switch_value_event event = {
		.widget = widget,
		.checked = widget->checked,
		.position = widget->position,
	};
	wlf_signal_emit_mutable(&widget->events.toggled, &event);
}
