#include "wlf/widgets/wlf_abstract_tabbar.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static struct wlf_scene_tree *widget_scene_tree_create(
		struct wlf_scene_tree *parent) {
	return parent != NULL ? wlf_scene_tree_create(&parent->base) :
		wlf_root_scene_tree_create();
}

void wlf_abstract_tabbar_init(struct wlf_abstract_tabbar *widget,
		const struct wlf_abstract_tabbar_impl *impl,
		struct wlf_scene_tree *parent, const struct wlf_frect *frame) {
	assert(widget != NULL);
	assert(impl != NULL);
	assert(impl->name != NULL);
	assert(impl->destroy != NULL);
	assert(frame != NULL);

	*widget = (struct wlf_abstract_tabbar){
		.impl = impl,
		.frame = *frame,
		.enabled = true,
		.selected_index = -1,
		.state = WLF_WIDGET_STATE_NORMAL,
	};

	widget->tree = widget_scene_tree_create(parent);
	if (widget->tree == NULL) {
		wlf_log(WLF_ERROR, "failed to create tabbar scene tree");
		return;
	}

	wlf_scene_node_set_position(&widget->tree->base, frame->x, frame->y);
	wlf_signal_init(&widget->events.destroy);
	wlf_signal_init(&widget->events.selection_changed);
}

void wlf_abstract_tabbar_destroy(struct wlf_abstract_tabbar *widget) {
	if (widget == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&widget->events.destroy, widget);
	assert(wlf_linked_list_empty(&widget->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&widget->events.selection_changed.listener_list));

	if (widget->impl != NULL && widget->impl->destroy != NULL) {
		widget->impl->destroy(widget);
	} else {
		free(widget);
	}
}

void wlf_abstract_tabbar_set_frame(struct wlf_abstract_tabbar *widget,
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

void wlf_abstract_tabbar_set_enabled(struct wlf_abstract_tabbar *widget,
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

void wlf_abstract_tabbar_set_state(struct wlf_abstract_tabbar *widget,
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

void wlf_abstract_tabbar_set_items(struct wlf_abstract_tabbar *widget,
		const struct wlf_tabbar_item *items, size_t item_count) {
	assert(widget != NULL);
	if (widget->impl->set_items != NULL) {
		widget->impl->set_items(widget, items, item_count);
	}
}

void wlf_abstract_tabbar_set_selected_index(struct wlf_abstract_tabbar *widget,
		int selected_index) {
	assert(widget != NULL);
	if (widget->selected_index == selected_index) {
		return;
	}

	widget->selected_index = selected_index;
	if (widget->impl->set_selected_index != NULL) {
		widget->impl->set_selected_index(widget, selected_index);
	}

	struct wlf_abstract_tabbar_selection_event event = {
		.widget = widget,
		.selected_index = widget->selected_index,
	};
	wlf_signal_emit_mutable(&widget->events.selection_changed, &event);
}
