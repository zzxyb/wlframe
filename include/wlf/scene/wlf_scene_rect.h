/**
 * @file        wlf_scene_rect.h
 * @brief       Rectangle scene node type for wlframe.
 * @details     This file provides the rectangle-based scene node used by wlframe scene graphs.
 *              A rectangle scene node is a concrete `wlf_scene_node` implementation with a fixed
 *              size and fill color, suitable for solid-color visual elements in the scene tree.
 * @author      YaoBing Xiao
 * @date        2026-05-21
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-21, initial version\n
 */

#ifndef SCENE_WLF_SCENE_RECT_H
#define SCENE_WLF_SCENE_RECT_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

/**
 * @brief Rectangle scene node.
 *
 * A concrete scene node that represents a filled rectangle in the scene graph.
 * It embeds `struct wlf_scene_node` as its base type and stores the rectangle's
 * fill color.
 */
struct wlf_scene_rect {
	struct wlf_scene_node base; /**< Base scene node */
	struct wlf_color color;     /**< Fill color of the rectangle */
};

/**
 * @brief Create a rectangle scene node.
 *
 * Allocates and initializes a rectangle node with the specified size and color.
 * If `parent` is not NULL, the new node is attached to the given scene tree.
 *
 * @param parent Parent scene tree to attach to, or NULL to create the node unattached.
 * @param width Rectangle width.
 * @param height Rectangle height.
 * @param color Fill color to use. If NULL, a transparent color is used.
 * @return Pointer to the newly created rectangle node, or NULL on allocation failure.
 */
struct wlf_scene_rect *wlf_scene_rect_create(struct wlf_scene_tree *parent,
	double width, double height, const struct wlf_color *color);

/**
 * @brief Set the rectangle size.
 *
 * Updates the width and height of the rectangle node. This affects the
 * rectangle's geometry and may trigger a scene graph update so that the
 * new size is reflected during rendering.
 *
 * @param rect Rectangle scene node.
 * @param width New rectangle width.
 * @param height New rectangle height.
 */
void wlf_scene_rect_set_size(struct wlf_scene_rect *rect, double width, double height);

/**
 * @brief Set the rectangle fill color.
 *
 * Updates the fill color used when rendering the rectangle.
 *
 * @param rect Rectangle scene node.
 * @param color New fill color.
 */
void wlf_scene_rect_set_color(struct wlf_scene_rect *rect, const struct wlf_color *color);

/**
 * @brief Check whether a scene node is a rectangle node.
 * @param node Scene node to test.
 * @return true if the node is a rectangle scene node, false otherwise.
 */
bool wlf_scene_node_is_rect(const struct wlf_scene_node *node);

/**
 * @brief Cast a scene node to a rectangle scene node.
 *
 * The caller must ensure that `node` is actually a rectangle node, typically by
 * checking `wlf_scene_node_is_rect()` first.
 *
 * @param node Scene node to cast.
 * @return Pointer to the corresponding rectangle scene node.
 */
struct wlf_scene_rect *wlf_scene_rect_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_RECT_H
