/**
 * @file        wlf_scene_lame_rect.h
 * @brief       Lamé-curve rounded rectangle scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_LAME_RECT_H
#define SCENE_WLF_SCENE_LAME_RECT_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

struct wlf_scene_lame_rect {
	struct wlf_scene_node base;
	struct wlf_color color;
	double lame_m;
	double lame_n;
};

struct wlf_scene_lame_rect *wlf_scene_lame_rect_create(
	struct wlf_scene_tree *parent, double width, double height,
	const struct wlf_color *color);
void wlf_scene_lame_rect_set_size(struct wlf_scene_lame_rect *rect,
	double width, double height);
void wlf_scene_lame_rect_set_color(struct wlf_scene_lame_rect *rect,
	const struct wlf_color *color);
void wlf_scene_lame_rect_set_lame_exponents(struct wlf_scene_lame_rect *rect,
	double lame_m, double lame_n);

bool wlf_scene_node_is_lame_rect(const struct wlf_scene_node *node);
struct wlf_scene_lame_rect *wlf_scene_lame_rect_from_node(
	struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_LAME_RECT_H
