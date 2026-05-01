#include "wlf/scene/wlf_scene_node.h"
#include "wlf/window/wlf_window.h"
#include "wlf/utils/wlf_linked_list.h"

#include <assert.h>
#include <stdlib.h>

void wlf_scene_node_place_above(struct wlf_scene_node *node,
		struct wlf_scene_node *sibling) {
	assert(node != sibling);
	assert(node->parent == sibling->parent);

	if (node->link.prev == &sibling->link) {
		return;
	}

	wlf_linked_list_remove(&node->link);
	wlf_linked_list_insert(&sibling->link, &node->link);
	wlf_scene_node_update(node, NULL);
}

void wlf_scene_node_place_below(struct wlf_scene_node *node,
		struct wlf_scene_node *sibling) {
	assert(node != sibling);
	assert(node->parent == sibling->parent);

	if (node->link.next == &sibling->link) {
		return;
	}

	wlf_linked_list_remove(&node->link);
	wlf_linked_list_insert(sibling->link.prev, &node->link);
	wlf_scene_node_update(node, NULL);
}

void wlf_scene_node_raise_to_top(struct wlf_scene_node *node) {
	struct wlf_linked_list *children = wlf_scene_node_get_children(node->parent);
	struct wlf_scene_node *current_top = wlf_container_of(
		children->prev, current_top, link);
	if (node == current_top) {
		return;
	}

	wlf_scene_node_place_above(node, current_top);
}

void wlf_scene_node_lower_to_bottom(struct wlf_scene_node *node) {
	struct wlf_linked_list *children = wlf_scene_node_get_children(node->parent);
	struct wlf_scene_node *current_bottom = wlf_container_of(
		children->next, current_bottom, link);
	if (node == current_bottom) {
		return;
	}

	wlf_scene_node_place_below(node, current_bottom);
}

void wlf_scene_node_reparent(struct wlf_scene_node *node,
		struct wlf_scene_node *new_parent) {
	assert(new_parent != NULL);
	assert(wlf_scene_node_get_children(node->parent) != NULL);

	if (node->parent == new_parent) {
		return;
	}

	for (struct wlf_scene_node *ancestor = new_parent; ancestor != NULL;
			ancestor = ancestor->parent) {
		assert(ancestor != node);
	}

	double x, y;
	struct wlf_region visible;
	wlf_region_init(&visible);
	if (wlf_scene_node_coords(node, &x, &y)) {
		wlf_scene_node_visibility(node, &visible);
	}

	wlf_linked_list_remove(&node->link);
	node->parent = new_parent;
	struct wlf_linked_list *children = wlf_scene_node_get_children(new_parent);
	wlf_linked_list_insert(children->prev, &node->link);
	wlf_scene_node_update(node, &visible);
}

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

	wlf_region_init(&node->state.visible);
	wlf_region_init(&node->state.transparent_region);
	wlf_region_init(&node->state.input_passthrough_region);

	if (parent != NULL) {
		wlf_linked_list_insert(parent->impl->get_children(parent)->prev, &node->link);
		node->window = parent->window;
	}

	wlf_addon_set_init(&node->addons);
}

void wlf_scene_node_destroy(struct wlf_scene_node *node) {
	if (node == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&node->events.destroy, node);
	assert(wlf_linked_list_empty(&node->events.destroy.listener_list));
	wlf_addon_set_finish(&node->addons);

	wlf_scene_node_set_enabled(node, false);
	wlf_linked_list_remove(&node->link);
	wlf_region_fini(&node->state.visible);
	wlf_region_fini(&node->state.transparent_region);
	wlf_region_fini(&node->state.input_passthrough_region);
	if (node->impl->destroy != NULL) {
		node->impl->destroy(node);
	} else {
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
		return;
	}

	node->impl->set_enabled(node, enabled);
}

void wlf_scene_node_set_position(struct wlf_scene_node *node, double x, double y) {
	if (node->state.x == x && node->state.y == y) {
		return;
	}

	if (node->impl->set_position == NULL) {
		node->state.x = x;
		node->state.y = y;
		wlf_scene_node_update(node, NULL);
		return;
	}

	node->impl->set_position(node, x, y);
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

	if (node->impl->set_opacity == NULL) {
		node->state.opacity = opacity;
		wlf_scene_node_update(node, NULL);
	}

	node->impl->set_opacity(node, opacity);
}

void wlf_scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	if (node->impl->get_size == NULL) {
		*width = 0;
		*height = 0;
		return;
	}

	node->impl->get_size(node, width, height);
}

struct wlf_linked_list *wlf_scene_node_get_children(struct wlf_scene_node *node) {
	if (node->impl->get_children == NULL) {
		return NULL;
	}

	return node->impl->get_children(node);
}

void wlf_scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
		double y, struct wlf_region *opaque) {
	if (node->impl->get_opaque_region == NULL) {
		double width, height;
		wlf_scene_node_get_size(node, &width, &height);
		wlf_region_fini(opaque);
		const struct wlf_frect rect = {
			.x = x,
			.y = y,
			.width = width,
			.height = height,
		};
		wlf_region_init_rect(opaque, &rect);
		return;
	}

	node->impl->get_opaque_region(node, x, y, opaque);
}

bool wlf_scene_node_invisible(struct wlf_scene_node *node) {
		if (node->impl->invisible == NULL) {
		return false;
	}

	return node->impl->invisible(node);
}

void wlf_scene_node_visibility(struct wlf_scene_node *node, struct wlf_region *visible) {
	if (node->impl->visibility == NULL) {
		return;
	}

	node->impl->visibility(node, visible);
}

struct wlf_scene_node *wlf_scene_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	if (node->impl->at == NULL) {
		return NULL;
	}

	return node->impl->at(node, lx, ly, nx, ny);
}

bool wlf_scene_node_coords(struct wlf_scene_node *node,
		double *lx_ptr, double *ly_ptr) {
	if (node->impl->coords == NULL) {
		double lx = 0, ly = 0;
		bool enabled = true;
		while (true) {
			lx += node->state.x;
			ly += node->state.y;
			enabled = enabled && node->state.enabled;
			if (node->parent == NULL) {
				break;
			}
	
			node = node->parent;
		}

		*lx_ptr = lx;
		*ly_ptr = ly;
		return enabled;
	}

	return node->impl->coords(node, lx_ptr, ly_ptr);
}

void wlf_scene_node_update(struct wlf_scene_node *node, struct wlf_region *damage) {
	// if (node->impl->update == NULL) {
	// 	struct wlr_scene *scene = node->scene;
	
	// 	int x, y;
	// 	if (!wlr_scene_node_coords(node, &x, &y)) {
	// 		// We assume explicit damage on a disabled tree means the node was just
	// 		// disabled.
	// 		if (damage) {
	// 			wlr_scene_node_cleanup_when_disabled(node, scene->restack_xwayland_surfaces, &scene->outputs);
	
	// 			scene_update_region(scene, damage);
	// 			scene_damage_outputs(scene, damage);
	// 			pixman_region32_fini(damage);
	// 		}
	
	// 		return;
	// 	}

	// 	pixman_region32_t visible;
	// 	if (!damage) {
	// 		pixman_region32_init(&visible);
	// 		wlr_scene_node_visibility(node, &visible);
	// 		damage = &visible;
	// 	}
	
	// 	pixman_region32_t update_region;
	// 	pixman_region32_init(&update_region);
	// 	pixman_region32_copy(&update_region, damage);
	// 	wlr_scene_node_bounds(node, x, y, &update_region);

	// 	scene_update_region(scene, &update_region);
	// 	pixman_region32_fini(&update_region);
	
	// 	wlr_scene_node_visibility(node, damage);
	// 	scene_damage_outputs(scene, damage);
	// 	pixman_region32_fini(damage);
	// 	return;
	// }

	// node->impl->update(node, damage);
}

void wlf_scene_node_bounds(struct wlf_scene_node *node,
		int x, int y, struct wlf_region *visible) {
	if (node->impl->bounds == NULL) {
		if (!node->state.enabled) {
			return;
		}

		double width, height;
		wlf_scene_node_get_size(node, &width, &height);
		const struct wlf_frect rect = {
			.x = x,
			.y = y,
			.width = width,
			.height = height,
		};
		wlf_region_union_rect(visible, visible, &rect);
		return;
	}

	node->impl->bounds(node, x, y, visible);
}
