/**
 * @file        wlf_path_node.h
 * @brief       Path scene-node interface.
 * @details     Declares the scene node that owns and renders a path shape.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef SCENE_WLF_PATH_NODE_H
#define SCENE_WLF_PATH_NODE_H

#include "wlf/pass/wlf_path_pass.h"
#include "wlf/scene/wlf_scene_node.h"

/**
 * @brief Scene node containing a path shape.
 *
 * The node owns @p shape and submits it through a path rendering pass.
 */
struct wlf_path_node {
	struct wlf_scene_node base; /**< Common scene-node state. */
	struct wlf_path_shape *shape; /**< Shape owned by the node. */
	enum wlf_render_blend_mode blend_mode; /**< Compositing mode. */
};

/**
 * @brief Creates a path node at a position relative to @p parent.
 * @param parent Parent scene node.
 * @param x Initial x position relative to @p parent.
 * @param y Initial y position relative to @p parent.
 * @param shape Path shape owned by the new node.
 * @return Newly allocated path node, or NULL on failure.
 */
struct wlf_path_node *wlf_path_node_create(struct wlf_scene_node *parent,
	int x, int y, struct wlf_path_shape *shape);

/**
 * @brief Checks whether a scene node is a path node.
 * @param node Scene node to inspect.
 * @return true when @p node is a path node, false otherwise.
 */
bool wlf_scene_node_is_path(const struct wlf_scene_node *node);

/**
 * @brief Casts a scene node to a path node.
 * @param node Scene node known to be a path node.
 * @return The enclosing path node.
 * @note The function asserts when @p node has another type.
 */
struct wlf_path_node *wlf_path_node_from_node(struct wlf_scene_node *node);

/**
 * @brief Renders a path node through the supplied pass.
 * @param node Path node to render.
 * @param pass Path pass used for geometry submission.
 * @param target Destination render target.
 * @param clip Optional clip region.
 */
void wlf_path_node_render(struct wlf_path_node *node,
	struct wlf_path_pass *pass, struct wlf_render_target_info *target,
	const pixman_region32_t *clip);

#endif
