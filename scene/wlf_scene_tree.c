#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wlf_window.h"

#include <stdlib.h>
#include <assert.h>

static void scene_node_destroy(struct wlf_scene_node *node) {
	struct wlf_window *window = node->window;
	struct wlf_scene_tree *tree = wlf_scene_tree_from_node(node);

	if (tree == window->tree) {
		assert(!node->parent);
	} else {
		assert(node->parent);
	}

	struct wlf_scene_node *child, *child_tmp;
	wlf_linked_list_for_each_safe(child, child_tmp,
			&tree->children, link) {
		wlf_scene_node_destroy(child);
	}

	free(tree);
}

static struct wlf_linked_list *scene_node_get_children(struct wlf_scene_node *node) {
	struct wlf_scene_tree *tree = wlf_scene_tree_from_node(node);
	return &tree->children;
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	return true;
}

static void scene_node_visibility(struct wlf_scene_node *node,
		struct wlf_region *visible) {
	if (!node->state.enabled) {
		return;
	}

	struct wlf_scene_tree *tree = wlf_scene_tree_from_node(node);
	struct wlf_scene_node *child, *child_tmp;
	wlf_linked_list_for_each_safe(child, child_tmp,
			&tree->children, link) {
		wlf_scene_node_visibility(child, visible);
	}
}

static bool scene_node_at_iterator(struct wlf_scene_node *node,
		double lx, double ly, void *data) {
	struct wlf_node_at_data *at_data = data;

	double rx = at_data->lx - lx;
	double ry = at_data->ly - ly;

	at_data->rx = rx;
	at_data->ry = ry;
	at_data->node = node;
	return true;
}

static struct wlf_scene_node *scene_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	struct wlf_frect box = {
		.x = lx,
		.y = ly,
		.width = 1,
		.height = 1
	};

	struct wlf_node_at_data data = {
		.lx = lx,
		.ly = ly
	};

	if (wlf_scene_node_in_box(node, &box, scene_node_at_iterator, &data)) {
		if (nx) {
			*nx = data.rx;
		}
		if (ny) {
			*ny = data.ry;
		}
		return data.node;
	}

	return NULL;
}

static void scene_node_bounds(struct wlf_scene_node *node,
		int x, int y, struct wlf_region *visible) {
	if (!node->state.enabled) {
		return;
	}

	struct wlf_scene_tree *tree = wlf_scene_tree_from_node(node);
	struct wlf_scene_node *child;
	wlf_linked_list_for_each(child, &tree->children, link) {
		wlf_scene_node_bounds(child, x + child->state.x, y + child->state.y, visible);
	}
}

static bool scene_nodes_in_box(struct wlf_scene_node *node, struct wlf_frect *box,
		scene_node_box_iterator_func_t iterator, void *user_data) {
	double x, y;
	wlf_scene_node_coords(node, &x, &y);

	// return _scene_nodes_in_box(node, box, iterator, user_data, x, y);
	if (!node->state.enabled) {
		return false;
	}

	struct wlf_scene_tree *tree = wlf_scene_tree_from_node(node);
	struct wlf_scene_node *child;
	wlf_linked_list_for_each_reverse(child, &tree->children, link) {
		if (wlf_scene_node_in_box(child, box, iterator, user_data)) {
			return true;
		}
	}

	return false;

}

static const struct wlf_scene_node_impl scene_node_impl = {
	.destroy = scene_node_destroy,
	.set_enabled = NULL,
	.set_position = NULL,
	.set_opacity = NULL,
	.get_size = NULL,
	.get_children = scene_node_get_children,
	.get_opaque_region = NULL,
	.invisible = scene_node_invisible,
	.visibility = scene_node_visibility,
	.at = scene_node_at,
	.coords = NULL,
	.update = NULL,
	.bounds = scene_node_bounds,
	.in_box = scene_nodes_in_box,
};

static struct wlf_scene_tree *scene_tree_create(struct wlf_scene_node *parent) {
	struct wlf_scene_tree *tree = calloc(1, sizeof(*tree));
	if (tree == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_scene_tree");
		return NULL;
	}

	wlf_scene_node_init(&tree->base, &scene_node_impl, parent);
	wlf_linked_list_init(&tree->children);
	return tree;
}

struct wlf_scene_tree *wlf_scene_tree_create(struct wlf_scene_node *parent) {
	assert(parent);

	return scene_tree_create(parent);
}

struct wlf_scene_tree *wlf_root_scene_tree_create(void) {
	return scene_tree_create(NULL);
}

bool wlf_scene_node_is_tree(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_tree *wlf_scene_tree_from_node(struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);

	struct wlf_scene_tree *scene_tree =
		wlf_container_of(node, scene_tree, base);

	return scene_tree;
}
