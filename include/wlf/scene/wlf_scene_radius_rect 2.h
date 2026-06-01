/**
 * @file        wlf_scene_radius_rect.h
 * @brief       Rounded rectangle scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_RADIUS_RECT_H
#define SCENE_WLF_SCENE_RADIUS_RECT_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

struct wlf_scene_radius_rect {
	struct wlf_scene_node base;
	struct wlf_color color;
	double radius_top_left;
	double radius_top_right;
	double radius_bottom_right;
	double radius_bottom_left;
};

struct wlf_scene_radius_rect *wlf_scene_radius_rect_create(
	struct wlf_scene_tree *parent, double width, double height,
	const struct wlf_color *color);

void wlf_scene_radius_rect_set_size(struct wlf_scene_radius_rect *rect,
	double width, double height);
void wlf_scene_radius_rect_set_color(struct wlf_scene_radius_rect *rect,
	const struct wlf_color *color);
void wlf_scene_radius_rect_set_corner_radii(struct wlf_scene_radius_rect *rect,
	double top_left, double top_right, double bottom_right, double bottom_left);

bool wlf_scene_node_is_radius_rect(const struct wlf_scene_node *node);
struct wlf_scene_radius_rect *wlf_scene_radius_rect_from_node(
	struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_RADIUS_RECT_H
