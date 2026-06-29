/**
 * @file        wlf_circle_node.h
 * @brief       Circle scene-node interface.
 * @details     Declares the scene node that owns and renders a circle shape.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef SCENE_WLF_CIRCLE_NODE_H
#define SCENE_WLF_CIRCLE_NODE_H

#include "wlf/pass/wlf_circle_pass.h"
#include "wlf/scene/wlf_scene_node.h"

/**
 * @brief Scene node containing a circle shape.
 *
 * The node owns @p shape and submits it through a circle rendering pass.
 */
struct wlf_circle_node {
	struct wlf_scene_node base; /**< Common scene-node state. */
	struct wlf_circle_shape *shape; /**< Shape owned by the node. */
	enum wlf_render_blend_mode blend_mode; /**< Compositing mode. */
};

/**
 * @brief Creates a circle node at a position relative to @p parent.
 * @param parent Parent scene node.
 * @param x Initial x position relative to @p parent.
 * @param y Initial y position relative to @p parent.
 * @param shape Circle shape owned by the new node.
 * @return Newly allocated circle node, or NULL on failure.
 */
struct wlf_circle_node *wlf_circle_node_create(struct wlf_scene_node *parent,
	int x, int y, struct wlf_circle_shape *shape);

/**
 * @brief Checks whether a scene node is a circle node.
 * @param node Scene node to inspect.
 * @return true when @p node is a circle node, false otherwise.
 */
bool wlf_scene_node_is_circle(const struct wlf_scene_node *node);

/**
 * @brief Casts a scene node to a circle node.
 * @param node Scene node known to be a circle node.
 * @return The enclosing circle node.
 * @note The function asserts when @p node has another type.
 */
struct wlf_circle_node *wlf_circle_node_from_node(struct wlf_scene_node *node);

/**
 * @brief Renders a circle node through the supplied pass.
 * @param node Circle node to render.
 * @param pass Circle pass used for geometry submission.
 * @param target Destination render target.
 * @param clip Optional clip region.
 */
void wlf_circle_node_render(struct wlf_circle_node *node,
	struct wlf_circle_pass *pass, struct wlf_render_target_info *target,
	const pixman_region32_t *clip);

#endif
