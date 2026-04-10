#ifndef SCENE_WLF_SCENE_TREE_H
#define SCENE_WLF_SCENE_TREE_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>

struct wlf_scene_tree {
	struct wlf_scene_node node;

	struct wlf_linked_list children; // wlf_scene_node.link
};

void wlf_scene_tree_init(struct wlf_scene_tree *tree,
	struct wlf_scene_tree *parent);
void wlf_scene_tree_finish(struct wlf_scene_tree *tree);
struct wlf_scene_tree *wlf_scene_tree_create(struct wlf_scene_tree *parent);
bool wlf_scene_node_is_tree(const struct wlf_scene_node *node);
struct wlf_scene_tree *wlf_scene_tree_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_TREE_H
