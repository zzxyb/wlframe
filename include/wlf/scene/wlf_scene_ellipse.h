/**
 * @file        wlf_scene_ellipse.h
 * @brief       Ellipse scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_ELLIPSE_H
#define SCENE_WLF_SCENE_ELLIPSE_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

struct wlf_scene_ellipse {
	struct wlf_scene_node base;
	struct wlf_color color;
	double radius_x;
	double radius_y;
};

struct wlf_scene_ellipse *wlf_scene_ellipse_create(struct wlf_scene_tree *parent,
	double radius_x, double radius_y, const struct wlf_color *color);
void wlf_scene_ellipse_set_radii(struct wlf_scene_ellipse *ellipse,
	double radius_x, double radius_y);
void wlf_scene_ellipse_set_color(struct wlf_scene_ellipse *ellipse,
	const struct wlf_color *color);

bool wlf_scene_node_is_ellipse(const struct wlf_scene_node *node);
struct wlf_scene_ellipse *wlf_scene_ellipse_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_ELLIPSE_H
