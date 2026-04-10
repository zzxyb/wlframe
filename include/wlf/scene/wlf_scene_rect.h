#ifndef SCENE_WLF_SCENE_RECT_H
#define SCENE_WLF_SCENE_RECT_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

struct wlf_scene_rect {
	struct wlf_scene_node node;
	struct wlf_color color;
};

struct wlf_scene_rect *wlf_scene_rect_create(struct wlf_scene_tree *parent,
	double width, double height, const struct wlf_color *color);
bool wlf_scene_node_is_rect(const struct wlf_scene_node *node);
struct wlf_scene_rect *wlf_scene_rect_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_RECT_H
