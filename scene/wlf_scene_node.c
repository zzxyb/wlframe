#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/pass/wlf_rect_pass.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wlf_window.h"
#include "wlf_scene_node_internal.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

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
	pixman_region32_t visible;
	pixman_region32_init(&visible);
	if (wlf_scene_node_coords(node, &x, &y)) {
		wlf_scene_node_visibility(node, &visible);
	}

	wlf_linked_list_remove(&node->link);
	node->parent = new_parent;
	struct wlf_linked_list *children = wlf_scene_node_get_children(new_parent);
	wlf_linked_list_insert(children->prev, &node->link);
	wlf_scene_node_update(node, &visible);
	pixman_region32_fini(&visible);
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

	pixman_region32_init(&node->state.visible);
	pixman_region32_init(&node->state.transparent_region);
	pixman_region32_init(&node->state.input_passthrough_region);

	if (parent != NULL) {
		wlf_linked_list_insert(parent->impl->get_children(parent)->prev, &node->link);
		node->scene = parent->scene;
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
	pixman_region32_fini(&node->state.visible);
	pixman_region32_fini(&node->state.transparent_region);
	pixman_region32_fini(&node->state.input_passthrough_region);
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
		pixman_region32_t visible;
		pixman_region32_init(&visible);
		if (wlf_scene_node_coords(node, &x, &y)) {
			wlf_scene_node_visibility(node, &visible);
		}

		node->state.enabled = enabled;

		wlf_scene_node_update(node, &visible);
		pixman_region32_fini(&visible);
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
		return;
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

void wlf_scene_node_opaque_region(struct wlf_scene_node *node, double x,
		double y, pixman_region32_t *opaque) {
	if (node->impl->opaque_region == NULL) {
		/* Unknown node types are conservatively treated as non-opaque. */
		return;
	}

	node->impl->opaque_region(node, x, y, opaque);
}

void wlf_scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
		double y, pixman_region32_t *opaque) {
	wlf_scene_node_opaque_region(node, x, y, opaque);
}

bool wlf_scene_node_invisible(struct wlf_scene_node *node) {
	if (node->impl->invisible == NULL) {
		return false;
	}

	return node->impl->invisible(node);
}

void wlf_scene_node_visibility(struct wlf_scene_node *node, pixman_region32_t *visible) {
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

void wlf_scene_node_update(struct wlf_scene_node *node, pixman_region32_t *damage) {
	if (node->impl->update == NULL) {
		pixman_region32_t old_visible;
		pixman_region32_t new_visible;
		pixman_region32_t changed;
		pixman_region32_init(&old_visible);
		pixman_region32_init(&new_visible);
		pixman_region32_init(&changed);

		if (damage != NULL) {
			pixman_region32_copy(&old_visible, damage);
		} else {
			pixman_region32_copy(&old_visible, &node->state.visible);
		}
		if (node->scene != NULL) {
			wlf_scene_recalculate_visibility(node->scene);
		} else {
			double x, y;
			if (wlf_scene_node_coords(node, &x, &y)) {
				wlf_scene_node_bounds(node, x, y, &new_visible);
				pixman_region32_copy(&node->state.visible, &new_visible);
			}
		}
		pixman_region32_copy(&new_visible, &node->state.visible);
		pixman_region32_union(&changed, &old_visible, &new_visible);

		if (node->scene != NULL) {
			wlf_scene_damage(node->scene, &changed);
		}

		pixman_region32_fini(&changed);
		pixman_region32_fini(&new_visible);
		pixman_region32_fini(&old_visible);
		return;
	}

	node->impl->update(node, damage);
}

void wlf_scene_node_bounds(struct wlf_scene_node *node,
		double x, double y, pixman_region32_t *visible) {
	if (node->impl->bounds == NULL) {
		if (!node->state.enabled) {
			return;
		}

		double width, height;
		wlf_scene_node_get_size(node, &width, &height);
		pixman_region32_union_rect(visible, visible, (int)x, (int)y,
			(uint32_t)width, (uint32_t)height);
		return;
	}

	node->impl->bounds(node, x, y, visible);
}

bool wlf_scene_node_nodes_in_box(struct wlf_scene_node *node,
		struct wlf_frect *box,
		scene_node_box_iterator_func_t iterator, void *user_data) {
	if (node->impl->in_box == NULL) {
		return false;
	}

	return node->impl->in_box(node, box, iterator, user_data);
}

bool wlf_scene_node_in_box(struct wlf_scene_node *node, struct wlf_frect *box,
		scene_node_box_iterator_func_t iterator, void *user_data) {
	return wlf_scene_node_nodes_in_box(node, box, iterator, user_data);
}

bool wlf_scene_node_construct_render_list_iterator(
		struct wlf_scene_node *node, double lx, double ly, void *data) {
	if (node->impl->construct_render_list_iterator == NULL) {
		return false;
	}

	return node->impl->construct_render_list_iterator(node, lx, ly, data);
}

bool wlf_scene_node_add_render_list_entry(struct wlf_scene_node *node,
		double lx, double ly, void *_data) {
	struct wlf_render_list_constructor_data *data = _data;
	if (wlf_scene_node_invisible(node)) {
		return false;
	}

	pixman_region32_t intersection;
	pixman_region32_init(&intersection);
	pixman_region32_intersect_rect(&intersection, &node->state.visible,
		(int)data->box.x, (int)data->box.y,
		(uint32_t)data->box.width, (uint32_t)data->box.height);
	bool visible = !pixman_region32_empty(&intersection);
	pixman_region32_fini(&intersection);
	if (!visible) {
		return false;
	}

	struct wlf_render_list_entry *entry = wlf_array_add(data->render_list,
		sizeof(*entry));
	if (entry == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to grow scene render list");
		data->failed = true;
		return true;
	}

	*entry = (struct wlf_render_list_entry){
		.node = node,
		.highlight_transparent_region = data->highlight_transparent_region,
		.x = lx,
		.y = ly,
	};
	return false;
}

bool wlf_scene_node_init_render_region(struct wlf_render_list_entry *entry,
		const struct wlf_render_data *data,
		pixman_region32_t *render_region) {
	pixman_region32_init(render_region);
	pixman_region32_intersect(render_region, &entry->node->state.visible,
		(pixman_region32_t *)&data->damage);
	return !pixman_region32_empty(render_region);
}

void wlf_scene_node_render(struct wlf_render_list_entry *entry,
		const struct wlf_render_data *data) {
	if (entry->node->impl->render == NULL) {
		return;
	}

	entry->node->impl->render(entry, data);
	if (!entry->highlight_transparent_region) {
		return;
	}

	pixman_region32_t transparent;
	pixman_region32_t opaque;
	pixman_region32_init(&transparent);
	pixman_region32_init(&opaque);
	pixman_region32_intersect(&transparent, &entry->node->state.visible,
		(pixman_region32_t *)&data->damage);
	wlf_scene_node_opaque_region(entry->node, entry->x, entry->y, &opaque);
	pixman_region32_subtract(&transparent, &transparent, &opaque);
	if (!pixman_region32_empty(&transparent)) {
		wlf_render_pass_add_rect(data->scene->rect_pass, data->target,
			&(struct wlf_render_rect_options){
				.box = data->logical,
				.color = { .g = 0.3, .a = 0.3 },
				.clip = &transparent,
				.blend_mode = WLF_RENDER_BLEND_MODE_PREMULTIPLIED,
			});
	}
	pixman_region32_fini(&opaque);
	pixman_region32_fini(&transparent);
}
