/**
 * @file        wlf_scene_radius_border.h
 * @brief       Rounded border scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_RADIUS_BORDER_H
#define SCENE_WLF_SCENE_RADIUS_BORDER_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

struct wlf_scene_radius_border {
	struct wlf_scene_node base;
	struct wlf_color color;
	double border_width;
	double radius_top_left;
	double radius_top_right;
	double radius_bottom_right;
	double radius_bottom_left;
};

struct wlf_scene_radius_border *wlf_scene_radius_border_create(
	struct wlf_scene_tree *parent, double width, double height,
	const struct wlf_color *color, double border_width);
void wlf_scene_radius_border_set_size(struct wlf_scene_radius_border *border,
	double width, double height);
void wlf_scene_radius_border_set_color(struct wlf_scene_radius_border *border,
	const struct wlf_color *color);
void wlf_scene_radius_border_set_border_width(
	struct wlf_scene_radius_border *border, double border_width);
void wlf_scene_radius_border_set_corner_radii(
	struct wlf_scene_radius_border *border, double top_left, double top_right,
	double bottom_right, double bottom_left);

bool wlf_scene_node_is_radius_border(const struct wlf_scene_node *node);
struct wlf_scene_radius_border *wlf_scene_radius_border_from_node(
	struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_RADIUS_BORDER_H
