#include "wlf/widgets/wlf_abstract_checkbox.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static struct wlf_scene_tree *widget_scene_tree_create(
		struct wlf_scene_tree *parent) {
	return parent != NULL ? wlf_scene_tree_create(&parent->base) :
		wlf_root_scene_tree_create();
}

void wlf_abstract_checkbox_init(struct wlf_abstract_checkbox *widget,
		const struct wlf_abstract_checkbox_impl *impl,
		struct wlf_scene_tree *parent, const struct wlf_frect *frame) {
	assert(widget != NULL);
	assert(impl != NULL);
	assert(impl->name != NULL);
	assert(impl->destroy != NULL);
	assert(frame != NULL);

	*widget = (struct wlf_abstract_checkbox){
		.impl = impl,
		.frame = *frame,
		.enabled = true,
		.state = WLF_WIDGET_STATE_NORMAL,
	};

	widget->tree = widget_scene_tree_create(parent);
	if (widget->tree == NULL) {
		wlf_log(WLF_ERROR, "failed to create checkbox scene tree");
		return;
	}

	wlf_scene_node_set_position(&widget->tree->base, frame->x, frame->y);
	wlf_signal_init(&widget->events.destroy);
	wlf_signal_init(&widget->events.toggled);
}

void wlf_abstract_checkbox_destroy(struct wlf_abstract_checkbox *widget) {
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

void wlf_abstract_checkbox_set_frame(struct wlf_abstract_checkbox *widget,
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

void wlf_abstract_checkbox_set_enabled(struct wlf_abstract_checkbox *widget,
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

void wlf_abstract_checkbox_set_state(struct wlf_abstract_checkbox *widget,
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

void wlf_abstract_checkbox_set_checked(struct wlf_abstract_checkbox *widget,
		bool checked) {
	assert(widget != NULL);
	if (widget->checked == checked) {
		return;
	}

	widget->checked = checked;
	if (widget->impl->set_checked != NULL) {
		widget->impl->set_checked(widget, checked);
	}

	struct wlf_abstract_checkbox_value_event event = {
		.widget = widget,
		.checked = widget->checked,
		.indeterminate = widget->indeterminate,
	};
	wlf_signal_emit_mutable(&widget->events.toggled, &event);
}

void wlf_abstract_checkbox_set_indeterminate(struct wlf_abstract_checkbox *widget,
		bool indeterminate) {
	assert(widget != NULL);
	if (widget->indeterminate == indeterminate) {
		return;
	}

	widget->indeterminate = indeterminate;
	if (widget->impl->set_indeterminate != NULL) {
		widget->impl->set_indeterminate(widget, indeterminate);
	}

	struct wlf_abstract_checkbox_value_event event = {
		.widget = widget,
		.checked = widget->checked,
		.indeterminate = widget->indeterminate,
	};
	wlf_signal_emit_mutable(&widget->events.toggled, &event);
}

void wlf_abstract_checkbox_toggle(struct wlf_abstract_checkbox *widget) {
	assert(widget != NULL);
	if (!widget->enabled) {
		return;
	}

	if (widget->impl->toggle != NULL) {
		widget->impl->toggle(widget);
	}

	struct wlf_abstract_checkbox_value_event event = {
		.widget = widget,
		.checked = widget->checked,
		.indeterminate = widget->indeterminate,
	};
	wlf_signal_emit_mutable(&widget->events.toggled, &event);
}
