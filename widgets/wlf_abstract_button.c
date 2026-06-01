#include "wlf/widgets/wlf_abstract_button.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static struct wlf_scene_tree *widget_scene_tree_create(
		struct wlf_scene_tree *parent) {
	if (parent != NULL) {
		return wlf_scene_tree_create(&parent->base);
	}

	return wlf_root_scene_tree_create();
}

void wlf_abstract_button_init(struct wlf_abstract_button *button,
		const struct wlf_abstract_button_impl *impl, struct wlf_scene_tree *parent,
		const struct wlf_frect *frame) {
	assert(button != NULL);
	assert(impl != NULL);
	assert(impl->name != NULL);
	assert(impl->destroy != NULL);
	assert(frame != NULL);

	*button = (struct wlf_abstract_button){
		.impl = impl,
		.frame = *frame,
		.enabled = true,
		.state = WLF_WIDGET_STATE_NORMAL,
	};

	button->tree = widget_scene_tree_create(parent);
	if (button->tree == NULL) {
		wlf_log(WLF_ERROR, "failed to create button scene tree");
		return;
	}

	wlf_scene_node_set_position(&button->tree->base, frame->x, frame->y);
	wlf_scene_node_set_enabled(&button->tree->base, true);

	wlf_signal_init(&button->events.destroy);
	wlf_signal_init(&button->events.clicked);
}

void wlf_abstract_button_destroy(struct wlf_abstract_button *button) {
	if (button == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&button->events.destroy, button);

	assert(wlf_linked_list_empty(&button->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&button->events.clicked.listener_list));

	if (button->impl != NULL && button->impl->destroy != NULL) {
		button->impl->destroy(button);
	} else {
		free(button);
	}
}

void wlf_abstract_button_set_frame(struct wlf_abstract_button *button,
		const struct wlf_frect *frame) {
	assert(button != NULL);
	assert(frame != NULL);

	if (wlf_frect_equal(&button->frame, frame)) {
		return;
	}

	button->frame = *frame;

	if (button->tree != NULL) {
		wlf_scene_node_set_position(&button->tree->base, frame->x, frame->y);
	}

	if (button->impl != NULL && button->impl->set_frame != NULL) {
		button->impl->set_frame(button, frame);
	}
}

void wlf_abstract_button_set_enabled(struct wlf_abstract_button *button, bool enabled) {
	assert(button != NULL);

	if (button->enabled == enabled) {
		return;
	}

	button->enabled = enabled;

	if (button->tree != NULL) {
		wlf_scene_node_set_enabled(&button->tree->base, enabled);
	}

	if (button->impl != NULL && button->impl->set_enabled != NULL) {
		button->impl->set_enabled(button, enabled);
	}
}

void wlf_abstract_button_set_state(struct wlf_abstract_button *button,
		enum wlf_widget_state state) {
	assert(button != NULL);

	if (button->state == state) {
		return;
	}

	button->state = state;

	if (button->impl != NULL && button->impl->set_state != NULL) {
		button->impl->set_state(button, state);
	}
}

void wlf_abstract_button_click(struct wlf_abstract_button *button) {
	assert(button != NULL);

	if (!button->enabled) {
		return;
	}

	if (button->impl != NULL && button->impl->click != NULL) {
		button->impl->click(button);
	}

	wlf_signal_emit_mutable(&button->events.clicked, button);
}
