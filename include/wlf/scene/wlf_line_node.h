#ifndef SCENE_WLF_LINE_NODE_H
#define SCENE_WLF_LINE_NODE_H

#include "wlf/pass/wlf_line_pass.h"
#include "wlf/scene/wlf_scene_node.h"

struct wlf_line_node {
	struct wlf_scene_node base;
	struct wlf_line_shape *shape;
	enum wlf_render_blend_mode blend_mode;
};

struct wlf_line_node *wlf_line_node_create(struct wlf_scene_node *parent,
	double x, double y, struct wlf_line_shape *shape);
bool wlf_scene_node_is_line(const struct wlf_scene_node *node);
struct wlf_line_node *wlf_line_node_from_node(struct wlf_scene_node *node);
void wlf_line_node_render(struct wlf_line_node *node,
	struct wlf_line_pass *pass, struct wlf_render_target_info *target,
	const pixman_region32_t *clip);

#endif
