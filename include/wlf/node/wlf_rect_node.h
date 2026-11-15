/**
 * @file        wlf_rect_node.h
 * @brief       Rectangle scene node.
 * @details     This file defines a scene node that renders a filled rectangle
 *              and exposes its color, blend mode, and rendering helpers.
 * @author      YaoBing Xiao
 * @date        2026-08-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-09, initial version\n
 */

#ifndef SCENE_WLF_RECT_NODE_H
#define SCENE_WLF_RECT_NODE_H

#include "wlf/pass/wlf_rect_pass.h"
#include "wlf/node/wlf_scene_node.h"
#include "wlf/types/wlf_color.h"

/**
 * @brief Scene node that renders a filled rectangle.
 */
struct wlf_rect_node {
	struct wlf_scene_node base; /**< Generic wlf_scene_node interface. */
	struct wlf_color color; /**< Text or fill color. */
	enum wlf_render_blend_mode blend_mode; /**< Blending mode used when rendering the node. */
};

/**
 * @brief Creates a rectangle node under a parent scene node.
 *
 * @param parent Parent scene node.
 * @param x X coordinate relative to the parent.
 * @param y Y coordinate relative to the parent.
 * @param width Initial width in logical pixels.
 * @param height Initial height in logical pixels.
 * @param color Fill color.
 * @return New rectangle node, or NULL on allocation failure.
 */
struct wlf_rect_node *wlf_rect_node_create(struct wlf_scene_node *parent,
	int x, int y, uint32_t width, uint32_t height,
	const struct wlf_color *color);

/**
 * @brief Changes the fill color and damages the complete rectangle.
 *
 * @param rect Rectangle node to modify or render.
 * @param color Fill color.
 */
void wlf_rect_node_set_color(struct wlf_rect_node *rect,
	const struct wlf_color *color);

/**
 * @brief Checks whether a scene node is a rectangle node.
 *
 * @param node Scene node to inspect or modify.
 * @return true for a rectangle node, false otherwise.
 */
bool wlf_scene_node_is_rect(const struct wlf_scene_node *node);

/**
 * @brief Casts a scene node to a rectangle node.
 *
 * @param node Scene node to inspect or modify.
 * @return Enclosing rectangle node; asserts on an incompatible node.
 */
struct wlf_rect_node *wlf_rect_node_from_node(struct wlf_scene_node *node);

/**
 * @brief Renders a rectangle node through a rectangle pass.
 *
 * @param rect Rectangle node to modify or render.
 * @param pass Rectangle rendering pass.
 * @param render_target_info Render target receiving the rectangle.
 * @param clip Region limiting rendering.
 */
void wlf_rect_node_render(struct wlf_rect_node *rect,
	struct wlf_rect_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);

#endif // SCENE_WLF_RECT_NODE_H
