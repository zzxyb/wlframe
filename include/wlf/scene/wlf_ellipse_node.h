/**
 * @file        wlf_ellipse_node.h
 * @brief       Ellipse scene-node interface.
 * @details     Declares the scene node that owns and renders an ellipse shape.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef SCENE_WLF_ELLIPSE_NODE_H
#define SCENE_WLF_ELLIPSE_NODE_H

#include "wlf/pass/wlf_ellipse_pass.h"
#include "wlf/scene/wlf_scene_node.h"

/**
 * @brief Scene node containing an ellipse shape.
 *
 * The node owns @p shape and submits it through an ellipse rendering pass.
 */
struct wlf_ellipse_node {
	struct wlf_scene_node base; /**< Common scene-node state. */
	struct wlf_ellipse_shape *shape; /**< Shape owned by the node. */
	enum wlf_render_blend_mode blend_mode; /**< Compositing mode. */
};

/**
 * @brief Creates an ellipse node at a position relative to @p parent.
 * @param parent Parent scene node.
 * @param x Initial x position relative to @p parent.
 * @param y Initial y position relative to @p parent.
 * @param shape Ellipse shape owned by the new node.
 * @return Newly allocated ellipse node, or NULL on failure.
 */
struct wlf_ellipse_node *wlf_ellipse_node_create(struct wlf_scene_node *parent,
	int x, int y, struct wlf_ellipse_shape *shape);

/**
 * @brief Checks whether a scene node is an ellipse node.
 * @param node Scene node to inspect.
 * @return true when @p node is an ellipse node, false otherwise.
 */
bool wlf_scene_node_is_ellipse(const struct wlf_scene_node *node);

/**
 * @brief Casts a scene node to an ellipse node.
 * @param node Scene node known to be an ellipse node.
 * @return The enclosing ellipse node.
 * @note The function asserts when @p node has another type.
 */
struct wlf_ellipse_node *wlf_ellipse_node_from_node(struct wlf_scene_node *node);

/**
 * @brief Renders an ellipse node through the supplied pass.
 * @param node Ellipse node to render.
 * @param pass Ellipse pass used for geometry submission.
 * @param target Destination render target.
 * @param clip Optional clip region.
 */
void wlf_ellipse_node_render(struct wlf_ellipse_node *node,
	struct wlf_ellipse_pass *pass, struct wlf_render_target_info *target,
	const pixman_region32_t *clip);

#endif
