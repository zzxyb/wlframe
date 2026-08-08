#ifndef SCENE_WLF_ELLIPSE_NODE_H
#define SCENE_WLF_ELLIPSE_NODE_H

#include "wlf/pass/wlf_ellipse_pass.h"
#include "wlf/scene/wlf_scene_node.h"

struct wlf_ellipse_node {
	struct wlf_scene_node base;
	struct wlf_ellipse_shape *shape;
	enum wlf_render_blend_mode blend_mode;
};

struct wlf_ellipse_node *wlf_ellipse_node_create(struct wlf_scene_node *parent,
	double x, double y, struct wlf_ellipse_shape *shape);
bool wlf_scene_node_is_ellipse(const struct wlf_scene_node *node);
struct wlf_ellipse_node *wlf_ellipse_node_from_node(struct wlf_scene_node *node);
void wlf_ellipse_node_render(struct wlf_ellipse_node *node,
	struct wlf_ellipse_pass *pass, struct wlf_render_target_info *target,
	const pixman_region32_t *clip);

#endif
