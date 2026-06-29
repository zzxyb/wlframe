/**
 * @file        wlf_line_node.h
 * @brief       Line scene-node interface.
 * @details     Declares the scene node that owns and renders a line shape.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef SCENE_WLF_LINE_NODE_H
#define SCENE_WLF_LINE_NODE_H

#include "wlf/pass/wlf_line_pass.h"
#include "wlf/scene/wlf_scene_node.h"

/**
 * @brief Scene node containing a line shape.
 *
 * The node owns @p shape and submits it through a line rendering pass.
 */
struct wlf_line_node {
	struct wlf_scene_node base; /**< Common scene-node state. */
	struct wlf_line_shape *shape; /**< Shape owned by the node. */
	enum wlf_render_blend_mode blend_mode; /**< Compositing mode. */
};

/**
 * @brief Creates a line node at a position relative to @p parent.
 * @param parent Parent scene node.
 * @param x Initial x position relative to @p parent.
 * @param y Initial y position relative to @p parent.
 * @param shape Line shape owned by the new node.
 * @return Newly allocated line node, or NULL on failure.
 */
struct wlf_line_node *wlf_line_node_create(struct wlf_scene_node *parent,
	int x, int y, struct wlf_line_shape *shape);

/**
 * @brief Checks whether a scene node is a line node.
 * @param node Scene node to inspect.
 * @return true when @p node is a line node, false otherwise.
 */
bool wlf_scene_node_is_line(const struct wlf_scene_node *node);

/**
 * @brief Casts a scene node to a line node.
 * @param node Scene node known to be a line node.
 * @return The enclosing line node.
 * @note The function asserts when @p node has another type.
 */
struct wlf_line_node *wlf_line_node_from_node(struct wlf_scene_node *node);

/**
 * @brief Renders a line node through the supplied pass.
 * @param node Line node to render.
 * @param pass Line pass used for geometry submission.
 * @param target Destination render target.
 * @param clip Optional clip region.
 */
void wlf_line_node_render(struct wlf_line_node *node,
	struct wlf_line_pass *pass, struct wlf_render_target_info *target,
	const pixman_region32_t *clip);

#endif
