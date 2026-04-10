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
	wlr_addon_set_finish(&node->addons);

	wlr_scene_node_set_enabled(node, false);

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
	}
}

void wlf_scene_node_set_position(struct wlf_scene_node *node, double x, double y) {
	if (node->state.x == x && node->state.y == y) {
		return;
	}

	node->state.x = x;
	node->state.y = y;
	scene_node_update(node, NULL);
}

void wlf_scene_node_place_above(struct wlf_scene_node *node,
		struct wlf_scene_node *sibling) {
	if (node == NULL || sibling == NULL || node == sibling ||
			node->parent == NULL || node->parent != sibling->parent) {
		return;
	}

	wlf_linked_list_remove(&node->link);
	wlf_linked_list_insert(&sibling->link, &node->link);
	wlf_scene_node_damage_whole_capture(node);
}

void wlf_scene_node_place_below(struct wlf_scene_node *node,
		struct wlf_scene_node *sibling) {
	if (node == NULL || sibling == NULL || node == sibling ||
			node->parent == NULL || node->parent != sibling->parent) {
		return;
	}

	if (node->impl->place_below != NULL) {
		node->impl->place_below(node, sibling);
		wlf_scene_node_damage_whole_capture(node);
		return;
	}

	wlf_linked_list_remove(&node->link);
	wlf_linked_list_insert(sibling->link.prev, &node->link);
	wlf_scene_node_damage_whole_capture(node);
}

void wlf_scene_node_raise_to_top(struct wlf_scene_node *node) {
	if (node == NULL || node->parent == NULL) {
		return;
	}

	wlf_linked_list_remove(&node->link);
	wlf_linked_list_insert(node->parent->children.prev, &node->link);
	wlf_scene_node_damage_whole_capture(node);
}

void wlf_scene_node_lower_to_bottom(struct wlf_scene_node *node) {
	if (node == NULL || node->parent == NULL) {
		return;
	}

	wlf_linked_list_remove(&node->link);
	wlf_linked_list_insert(&node->parent->children, &node->link);
	wlf_scene_node_damage_whole_capture(node);
}

void wlf_scene_node_reparent(struct wlf_scene_node *node,
		struct wlf_scene_tree *new_parent) {
	if (node == NULL || new_parent == NULL || node->parent == new_parent) {
		return;
	}

	wlf_scene_node_damage_whole_capture(node);

	if (node->parent != NULL) {
		wlf_linked_list_remove(&node->link);
	}

	node->parent = new_parent;
	wlf_linked_list_insert(new_parent->children.prev, &node->link);

	wlf_scene_node_damage_whole_capture(node);
}

bool wlf_scene_node_coords(struct wlf_scene_node *node, int *lx, int *ly) {
	double x = 0.0;
	double y = 0.0;

	if (node == NULL) {
		return false;
	}

	for (struct wlf_scene_node *iter = node; iter != NULL; iter =
			iter->parent != NULL ? &iter->parent->node : NULL) {
		if (!iter->state.enabled) {
			return false;
		}

		x += iter->state.x;
		y += iter->state.y;
	}

	if (lx != NULL) {
		*lx = (int)x;
	}
	if (ly != NULL) {
		*ly = (int)y;
	}

	return true;
}

struct wlf_scene_node *wlf_scene_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	if (node == NULL || !node->state.enabled) {
		return NULL;
	}

	double local_x = lx - node->state.x;
	double local_y = ly - node->state.y;

	if (wlf_scene_node_is_tree(node)) {
		struct wlf_scene_tree *tree = (struct wlf_scene_tree *)node;
		struct wlf_scene_node *child;

		wlf_linked_list_for_each_reverse(child, &tree->children, link) {
			struct wlf_scene_node *found =
				wlf_scene_node_at(child, local_x, local_y, nx, ny);
			if (found != NULL) {
				return found;
			}
		}

		return NULL;
	}

	if (!scene_node_point_visible(node, local_x, local_y)) {
		return NULL;
	}

	if (nx != NULL) {
		*nx = local_x;
	}
	if (ny != NULL) {
		*ny = local_y;
	}

	return node;
}

void wlf_scene_node_set_opacity(struct wlf_scene_node *node,
		float opacity) {
	if (node == NULL) {
		return;
	}

	if (opacity < 0.0f) {
		opacity = 0.0f;
	} else if (opacity > 1.0f) {
		opacity = 1.0f;
	}

	node->state.opacity = opacity;
	wlf_scene_node_damage_whole_capture(node);
}

void wlf_scene_buffer_set_opacity(struct wlf_scene_node *node,
		float opacity) {
	wlf_scene_node_set_opacity(node, opacity);
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
