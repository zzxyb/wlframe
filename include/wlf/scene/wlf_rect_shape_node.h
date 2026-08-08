#ifndef SCENE_WLF_RECT_SHAPE_NODE_H
#define SCENE_WLF_RECT_SHAPE_NODE_H

#include "wlf/pass/wlf_rect_shape_pass.h"
#include "wlf/scene/wlf_scene_node.h"

struct wlf_rect_shape_node {
	struct wlf_scene_node base;
	struct wlf_rect_shape *shape;
	enum wlf_render_blend_mode blend_mode;
};

struct wlf_rect_shape_node *wlf_rect_shape_node_create(
	struct wlf_scene_node *parent, double x, double y,
	struct wlf_rect_shape *shape);
bool wlf_scene_node_is_rect_shape(const struct wlf_scene_node *node);
struct wlf_rect_shape_node *wlf_rect_shape_node_from_node(
	struct wlf_scene_node *node);
void wlf_rect_shape_node_render(struct wlf_rect_shape_node *node,
	struct wlf_rect_shape_pass *pass,
	struct wlf_render_target_info *target, const pixman_region32_t *clip);

#endif
