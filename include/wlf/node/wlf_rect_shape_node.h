/**
 * @file        wlf_rect_shape_node.h
 * @brief       Rectangle-shape scene-node interface.
 * @details     Declares the scene node that owns and renders a rectangle
 *              shape, including rounded corners when present.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef SCENE_WLF_RECT_SHAPE_NODE_H
#define SCENE_WLF_RECT_SHAPE_NODE_H

#include "wlf/pass/wlf_rect_shape_pass.h"
#include "wlf/node/wlf_scene_node.h"

/**
 * @brief Scene node containing a rectangle shape.
 *
 * The node owns @p shape and submits it through a rectangle-shape rendering
 * pass.
 */
struct wlf_rect_shape_node {
	struct wlf_scene_node base; /**< Common scene-node state. */
	struct wlf_rect_shape *shape; /**< Shape owned by the node. */
	enum wlf_render_blend_mode blend_mode; /**< Compositing mode. */
};

/**
 * @brief Creates a rectangle-shape node at a position relative to @p parent.
 * @param parent Parent scene node.
 * @param x Initial x position relative to @p parent.
 * @param y Initial y position relative to @p parent.
 * @param shape Rectangle shape owned by the new node.
 * @return Newly allocated rectangle-shape node, or NULL on failure.
 */
struct wlf_rect_shape_node *wlf_rect_shape_node_create(
	struct wlf_scene_node *parent, int x, int y,
	struct wlf_rect_shape *shape);

/**
 * @brief Checks whether a scene node is a rectangle-shape node.
 * @param node Scene node to inspect.
 * @return true when @p node is a rectangle-shape node, false otherwise.
 */
bool wlf_scene_node_is_rect_shape(const struct wlf_scene_node *node);

/**
 * @brief Casts a scene node to a rectangle-shape node.
 * @param node Scene node known to be a rectangle-shape node.
 * @return The enclosing rectangle-shape node.
 * @note The function asserts when @p node has another type.
 */
struct wlf_rect_shape_node *wlf_rect_shape_node_from_node(
	struct wlf_scene_node *node);

/**
 * @brief Renders a rectangle-shape node through the supplied pass.
 * @param node Rectangle-shape node to render.
 * @param pass Rectangle-shape pass used for geometry submission.
 * @param target Destination render target.
 * @param clip Optional clip region.
 */
void wlf_rect_shape_node_render(struct wlf_rect_shape_node *node,
	struct wlf_rect_shape_pass *pass,
	struct wlf_render_target_info *target, const pixman_region32_t *clip);

#endif
