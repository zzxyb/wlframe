/**
 * @file        wlf_scene_lame_border.h
 * @brief       Lamé-curve rounded border scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_LAME_BORDER_H
#define SCENE_WLF_SCENE_LAME_BORDER_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

struct wlf_scene_lame_border {
	struct wlf_scene_node base;
	struct wlf_color color;
	double border_width;
	double lame_m;
	double lame_n;
};

struct wlf_scene_lame_border *wlf_scene_lame_border_create(
	struct wlf_scene_tree *parent, double width, double height,
	const struct wlf_color *color, double border_width);
void wlf_scene_lame_border_set_size(struct wlf_scene_lame_border *border,
	double width, double height);
void wlf_scene_lame_border_set_color(struct wlf_scene_lame_border *border,
	const struct wlf_color *color);
void wlf_scene_lame_border_set_border_width(
	struct wlf_scene_lame_border *border, double border_width);
void wlf_scene_lame_border_set_lame_exponents(
	struct wlf_scene_lame_border *border, double lame_m, double lame_n);

bool wlf_scene_node_is_lame_border(const struct wlf_scene_node *node);
struct wlf_scene_lame_border *wlf_scene_lame_border_from_node(
	struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_LAME_BORDER_H
