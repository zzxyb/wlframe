/**
 * @file        wlf_shape_tree.h
 * @brief       Tree container shape for grouping multiple child shapes.
 * @details     A shape tree owns zero or more child shapes and can be used as a composite
 *              node in higher-level shape hierarchies.
 * @author      YaoBing Xiao
 * @date        2026-04-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-07, initial version\n
 */

#ifndef SHAPES_WLF_SHAPE_TREE_H
#define SHAPES_WLF_SHAPE_TREE_H

#include "wlf/shapes/wlf_shape.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>

struct wlf_shape_tree {
	struct wlf_shape base;

	struct wlf_linked_list children; /**< Child list using struct wlf_shape::link. */
};

/**
 * @brief Create an empty shape tree.
 *
 * @return A new shape tree as base shape pointer, or NULL on allocation failure.
 */
struct wlf_shape *wlf_shape_tree_create(void);

/**
 * @brief Append a child shape to the tree.
 *
 * @param tree Destination tree.
 * @param shape Child shape to add.
 */
void wlf_shape_tree_add(struct wlf_shape_tree *tree, struct wlf_shape *shape);

/**
 * @brief Remove a child shape from the tree.
 *
 * @param tree Tree containing the child.
 * @param shape Child shape to remove.
 */
void wlf_shape_tree_remove(struct wlf_shape_tree *tree, struct wlf_shape *shape);

/**
 * @brief Get the number of child shapes in a tree.
 *
 * @param tree Shape tree.
 * @return Number of child shapes.
 */
int wlf_shape_tree_child_count(const struct wlf_shape_tree *tree);

/**
 * @brief Check whether a base shape is a shape tree.
 *
 * @param shape Shape to test.
 * @return true if shape is a tree, false otherwise.
 */
bool wlf_shape_is_tree(struct wlf_shape *shape);

/**
 * @brief Cast a base shape pointer to a shape tree pointer.
 *
 * @param shape Base shape pointer.
 * @return Shape tree pointer (asserts if shape is not a tree).
 */
struct wlf_shape_tree *wlf_shape_tree_from_shape(struct wlf_shape *shape);

#endif // SHAPES_WLF_SHAPE_TREE_H
