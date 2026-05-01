#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>

static void scene_tree_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_tree *tree = wlf_scene_tree_from_node(node);
	free(tree);
}

static const struct wlf_scene_node_impl scene_node_impl = {
	.destroy = scene_tree_destroy,
	.set_enabled = NULL,
	.set_position = NULL,
	.set_opacity = NULL,
	.get_size = NULL,
	.get_children = NULL,
	.get_opaque_region = NULL,
	.invisible = NULL,
	.visibility = NULL,
	.at = NULL,
	.coords = NULL,
	.update = NULL,
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
