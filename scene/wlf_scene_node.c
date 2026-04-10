#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"

#include <assert.h>
#include <stdlib.h>

void wlf_scene_node_init(struct wlf_scene_node *node,
		const struct wlf_scene_node_impl *impl, struct wlf_scene_node *parent) {
	assert(node != NULL);
	assert(impl != NULL);
	assert(impl->destroy != NULL);

	*node = (struct wlf_scene_node){
		.impl = impl,
		.parent = parent,
		.state = {
			.enabled = true,
			.opacity = 1.0f,
			.focus_policy = CLICK_FOCUS,
		},
	};

	wlf_linked_list_init(&node->link);

	wlf_signal_init(&node->events.destroy);

	wlf_addon_set_init(&node->addons);

	wlf_region_init(&node->state.visible);
	wlf_region_init(&node->state.transparent_region);
	wlf_region_init(&node->state.input_passthrough_region);
}

void wlf_scene_node_destroy(struct wlf_scene_node *node) {
	if (node == NULL) {
		return;
	}

	wl_signal_emit_mutable(&node->events.destroy, NULL);
	wlf_addon_set_finish(&node->addons);

	wlf_scene_node_set_enabled(node, false);

	if (node->impl->destroy != NULL) {
		node->impl->destroy(node);
		assert(wl_list_empty(&node->events.destroy.listener_list));
	} else {
		wl_list_remove(&node->link);
		wlf_region_fini(&node->state.visible);
		wlf_region_fini(&node->state.transparent_region);
		wlf_region_fini(&node->state.input_passthrough_region);
		free(node);
	}
}

void wlf_scene_node_set_enabled(struct wlf_scene_node *node, bool enabled) {
	if (node->state.enabled == enabled) {
		return;
	}

	if (node->impl->set_enabled == NULL) {
		double x, y;
		struct wlf_region visible;
		wlf_region_init(&visible);
		if (wlf_scene_node_coords(node, &x, &y)) {
			wlf_scene_node_visibility(node, &visible);
		}

		node->state.enabled = enabled;

		wlf_scene_node_update(node, &visible);
	} else {
		node->impl->set_enabled(node, enabled);
	}
}

void wlf_scene_node_set_position(struct wlf_scene_node *node, double x, double y) {
	if (node->state.x == x && node->state.y == y) {
		return;
	}

	if (node->impl->set_position == NULL) {
		node->state.x = x;
		node->state.y = y;
		wlf_scene_node_update(node, NULL);
	} else {
		node->impl->set_position(node, x, y);
	}
}

void wlf_scene_node_place_above(struct wlf_scene_node *node,
		struct wlf_scene_node *sibling) {
	assert(node != sibling);
	assert(node->parent == sibling->parent);

	if (node->link.prev == &sibling->link) {
		return;
	}

	if (node->impl->place_above == NULL) {
		wlf_linked_list_remove(&node->link);
		wlf_linked_list_insert(&sibling->link, &node->link);
		wlf_scene_node_update(node, NULL);
	} else {
		node->impl->place_above(node, sibling);
	}
}

void wlf_scene_node_place_below(struct wlf_scene_node *node,
		struct wlf_scene_node *sibling) {
	assert(node != sibling);
	assert(node->parent == sibling->parent);

	if (node->link.next == &sibling->link) {
		return;
	}

	if (node->impl->place_below == NULL) {
		wlf_linked_list_remove(&node->link);
		wlf_linked_list_insert(sibling->link.prev, &node->link);
		wlf_scene_node_update(node, NULL);
	} else {
		node->impl->place_below(node, sibling);
	}
}

void wlf_scene_node_raise_to_top(struct wlf_scene_node *node) {
	assert(wlf_scene_node_get_children(node->parent) == NULL);

	if (node->impl->raise_to_top == NULL) {
		struct wlf_linked_list *list = wlf_scene_node_get_children(node->parent);
		struct wlf_scene_node *current_top = wlf_container_of(
			list->prev, current_top, link);
		if (node == current_top) {
			return;
		}

		wlf_scene_node_place_above(node, current_top);
	} else {
		node->impl->raise_to_top(node);
	}
}

void wlf_scene_node_lower_to_bottom(struct wlf_scene_node *node) {
	assert(wlf_scene_node_get_children(node->parent) == NULL);

	if (node->impl->lower_to_bottom == NULL) {
		struct wlf_linked_list *list = wlf_scene_node_get_children(node->parent);
		struct wlf_scene_node *current_bottom = wlf_container_of(
			list->next, current_bottom, link);
		
		if (node == current_bottom) {
			return;
		}

		wlf_scene_node_place_below(node, current_bottom);
	} else {
		node->impl->lower_to_bottom(node);
	}
}

void wlf_scene_node_reparent(struct wlf_scene_node *node,
		struct wlf_scene_node *new_parent) {
	assert(new_parent != NULL);
	assert(wlf_scene_node_get_children(node->parent) == NULL);

	if (node->parent == new_parent) {
		return;
	}

	for (struct wlf_scene_node *ancestor = new_parent; ancestor != NULL;
			ancestor = ancestor->parent) {
		assert(&ancestor != node);
	}

	if (node->impl->reparent == NULL) {
		double x, y;
		struct wlf_region visible;
		wlf_region_init(&visible);
		if (wlf_scene_node_coords(node, &x, &y)) {
			wlf_scene_node_visibility(node, &visible);
		}

		wl_list_remove(&node->link);
		node->parent = new_parent;
		struct wlf_linked_list *list = wlf_scene_node_get_children(new_parent);
		wl_list_insert(list->prev, &node->link);
		scene_node_update(node, &visible);
	} else {
		node->impl->reparent(node, new_parent);
	}
}

bool wlf_scene_node_coords(struct wlf_scene_node *node,
		double *lx_ptr, double *ly_ptr) {
	assert(node != NULL);

	int lx = 0, ly = 0;
	bool enabled = true;
	while (true) {
		lx += node->state.x;
		ly += node->state.y;
		enabled = enabled && node->state.enabled;
		if (node->parent == NULL) {
			break;
		}

		node = wlf_scene_node_get_base(&node->parent);
	}

	*lx_ptr = lx;
	*ly_ptr = ly;
	return enabled;
}

struct wlf_scene_node *wlf_scene_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	assert(node->impl->at != NULL);

	if (node == NULL || !node->state.enabled) {
		return NULL;
	}

	return node->impl->at(node, lx, ly, nx, ny);
}

struct wlf_linked_list *wlf_scene_node_get_children(struct wlf_scene_node *node) {
	assert(node->impl->get_children != NULL);

	return node->impl->get_children(node); 
}

struct wlf_scene_node *wlf_scene_node_get_base(struct wlf_scene_node *node) {
	assert(node->impl->get_base != NULL);

	return node->impl->get_children(node);
}

void wlf_scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	assert(node->impl->get_size != NULL);

	return node->impl->get_size(node, width, height);
}

void wlf_scene_node_set_opacity(struct wlf_scene_node *node,
		float opacity) {
	if (node->state.opacity == opacity) {
		return;
	}

	if (opacity < 0.0f) {
		opacity = 0.0f;
	} else if (opacity > 1.0f) {
		opacity = 1.0f;
	}

	assert(opacity >= 0 && opacity <= 1);
	node->state.opacity = opacity;
	wlf_scene_node_update(node, NULL);
}

void wlf_scene_node_set_transparent_region(struct wlf_scene_node *node,
		struct wlf_region *region) {
	if (node == NULL) {
		return;
	}

	scene_node_region_assign(&node->state.transparent_region, region);
	wlf_scene_node_damage_whole_capture(node);
}

void wlf_scene_node_set_input_passthrough_region(struct wlf_scene_node *node,
		struct wlf_region *region) {
	if (node == NULL) {
		return;
	}

	scene_node_region_assign(&node->state.input_passthrough_region, region);
	wlf_scene_node_damage_whole_capture(node);
}

void wlf_scene_node_update(struct wlf_scene_node *node, struct wlf_region *damage) {
	assert(node->impl->update != NULL);

	node->impl->update(node, damage);
}

void wlf_scene_node_visibility(struct wlf_scene_node *node, struct wlf_region *visible) {
	assert(node->impl->visibility != NULL);

	return node->impl->visibility(node, visible);
}

