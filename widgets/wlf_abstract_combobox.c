#include "wlf/widgets/wlf_abstract_combobox.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static struct wlf_scene_tree *widget_scene_tree_create(
		struct wlf_scene_tree *parent) {
	return parent != NULL ? wlf_scene_tree_create(&parent->base) :
		wlf_root_scene_tree_create();
}

void wlf_abstract_combobox_init(struct wlf_abstract_combobox *widget,
		const struct wlf_abstract_combobox_impl *impl, struct wlf_scene_tree *parent,
		const struct wlf_frect *frame) {
	assert(widget != NULL);
	assert(impl != NULL);
	assert(impl->name != NULL);
	assert(impl->destroy != NULL);
	assert(frame != NULL);

	*widget = (struct wlf_abstract_combobox){
		.impl = impl,
		.frame = *frame,
		.enabled = true,
		.open = false,
		.selected_index = -1,
		.state = WLF_WIDGET_STATE_NORMAL,
	};

	widget->tree = widget_scene_tree_create(parent);
	if (widget->tree == NULL) {
		wlf_log(WLF_ERROR, "failed to create combobox scene tree");
		return;
	}

	wlf_scene_node_set_position(&widget->tree->base, frame->x, frame->y);
	wlf_signal_init(&widget->events.destroy);
	wlf_signal_init(&widget->events.selection_changed);
	wlf_signal_init(&widget->events.open_changed);
}

void wlf_abstract_combobox_destroy(struct wlf_abstract_combobox *widget) {
	if (widget == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&widget->events.destroy, widget);
	assert(wlf_linked_list_empty(&widget->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&widget->events.selection_changed.listener_list));
	assert(wlf_linked_list_empty(&widget->events.open_changed.listener_list));

	if (widget->impl != NULL && widget->impl->destroy != NULL) {
		widget->impl->destroy(widget);
	} else {
		free(widget);
	}
}

void wlf_abstract_combobox_set_frame(struct wlf_abstract_combobox *widget,
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

void wlf_abstract_combobox_set_enabled(struct wlf_abstract_combobox *widget,
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

void wlf_abstract_combobox_set_state(struct wlf_abstract_combobox *widget,
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

void wlf_abstract_combobox_set_options(struct wlf_abstract_combobox *widget,
		const struct wlf_combobox_option *options, size_t option_count) {
	assert(widget != NULL);
	if (widget->impl->set_options != NULL) {
		widget->impl->set_options(widget, options, option_count);
	}
}

void wlf_abstract_combobox_set_selected_index(
		struct wlf_abstract_combobox *widget, int selected_index) {
	assert(widget != NULL);
	if (widget->selected_index == selected_index) {
		return;
	}

	widget->selected_index = selected_index;
	if (widget->impl->set_selected_index != NULL) {
		widget->impl->set_selected_index(widget, selected_index);
	}

	struct wlf_abstract_combobox_selection_event event = {
		.widget = widget,
		.selected_index = widget->selected_index,
	};
	wlf_signal_emit_mutable(&widget->events.selection_changed, &event);
}

void wlf_abstract_combobox_set_open(struct wlf_abstract_combobox *widget,
		bool open) {
	assert(widget != NULL);
	if (widget->open == open) {
		return;
	}

	widget->open = open;
	if (widget->impl->set_open != NULL) {
		widget->impl->set_open(widget, open);
	}
	wlf_signal_emit_mutable(&widget->events.open_changed, widget);
}
