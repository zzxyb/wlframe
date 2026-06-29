/**
 * @file        wlf_rect_node.h
 * @brief       Rectangle scene node.
 */

#ifndef SCENE_WLF_RECT_NODE_H
#define SCENE_WLF_RECT_NODE_H

#include "wlf/pass/wlf_rect_pass.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/types/wlf_color.h"

/**
 * @brief Scene node that renders a filled rectangle.
 */
struct wlf_rect_node {
	struct wlf_scene_node base;
	struct wlf_color color;
	enum wlf_render_blend_mode blend_mode;
};

/**
 * @brief Creates a rectangle node under a parent scene node.
 */
struct wlf_rect_node *wlf_rect_node_create(struct wlf_scene_node *parent,
	int x, int y, uint32_t width, uint32_t height,
	const struct wlf_color *color);

/** Changes the fill color and damages the complete rectangle. */
void wlf_rect_node_set_color(struct wlf_rect_node *rect,
	const struct wlf_color *color);

/**
 * @brief Checks whether a scene node is a rectangle node.
 */
bool wlf_scene_node_is_rect(const struct wlf_scene_node *node);

/**
 * @brief Casts a scene node to a rectangle node.
 */
struct wlf_rect_node *wlf_rect_node_from_node(struct wlf_scene_node *node);

/**
 * @brief Renders a rectangle node through a rectangle pass.
 */
void wlf_rect_node_render(struct wlf_rect_node *rect,
	struct wlf_rect_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);

#endif // SCENE_WLF_RECT_NODE_H
