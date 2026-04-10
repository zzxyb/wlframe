#include "wlf/scene/wlf_scene_tree.h"

#include <stdlib.h>

static void scene_tree_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_tree *tree = (struct wlf_scene_tree *)node;
	free(tree);
}

static const struct wlf_scene_node_impl scene_tree_impl = {
	.destroy = scene_tree_destroy,
};

void wlf_scene_tree_init(struct wlf_scene_tree *tree,
		struct wlf_scene_tree *parent) {
	wlf_scene_node_init(&tree->node, &scene_tree_impl);
	wlf_linked_list_init(&tree->children);

	if (parent != NULL) {
		wlf_scene_node_reparent(&tree->node, parent);
	}
}

void wlf_scene_tree_finish(struct wlf_scene_tree *tree) {
	if (tree == NULL) {
		return;
	}

	while (!wlf_linked_list_empty(&tree->children)) {
		struct wlf_scene_node *child =
			wlf_container_of(tree->children.next, child, link);
		wlf_scene_node_destroy(child);
	}
}

struct wlf_scene_tree *wlf_scene_tree_create(struct wlf_scene_tree *parent) {
	struct wlf_scene_tree *tree = malloc(sizeof(*tree));
	if (tree == NULL) {
		return NULL;
	}

	wlf_scene_tree_init(tree, parent);
	return tree;
}

bool wlf_scene_node_is_tree(const struct wlf_scene_node *node) {
	return node != NULL && node->impl == &scene_tree_impl;
}

struct wlf_scene_tree *wlf_scene_tree_from_node(struct wlf_scene_node *node) {
	if (!wlf_scene_node_is_tree(node)) {
		return NULL;
	}

	return (struct wlf_scene_tree *)node;
}
