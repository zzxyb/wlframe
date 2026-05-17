/**
 * @file        wlf_scene_tree.h
 * @brief       Scene tree container for wlframe.
 * @details     Defines the scene tree node type used for managing a collection of child
 *              scene nodes. A scene tree is a special scene node that can own and
 *              manage child nodes using a linked list.
 * @author      YaoBing Xiao
 * @date        2026-05-01
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-01, initial version\n
 */

#ifndef SCENE_WLF_SCENE_TREE_H
#define SCENE_WLF_SCENE_TREE_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>

/**
 * @brief Scene tree structure.
 *
 * A scene tree is a scene node implementation that maintains an explicit list of children.
 */
struct wlf_scene_tree {
	struct wlf_scene_node base;          /**< Base scene node */

	struct wlf_linked_list children;     /**< List of child nodes (wlf_scene_node.link) */
};

/**
 * @brief Creates a scene tree with a parent node.
 *
 * @param parent Parent scene node for the new tree.
 * @return Newly allocated scene tree.
 */
struct wlf_scene_tree *wlf_scene_tree_create(struct wlf_scene_node *parent);

/**
 * @brief Creates a root scene tree.
 *
 * @return Newly allocated root scene tree.
 */
struct wlf_scene_tree *wlf_root_scene_tree_create(void);

/**
 * @brief Checks whether a scene node is a scene tree.
 *
 * @param node Scene node to test.
 * @return True if the node is a scene tree.
 */
bool wlf_scene_node_is_tree(const struct wlf_scene_node *node);

/**
 * @brief Converts a scene node pointer to a scene tree pointer.
 *
 * @param node Scene node pointer.
 * @return Scene tree pointer if the node is a tree, otherwise NULL.
 */
struct wlf_scene_tree *wlf_scene_tree_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_TREE_H
