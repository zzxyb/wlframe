/**
 * @file        wlf_scene_circle.h
 * @brief       Circle outline scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_CIRCLE_H
#define SCENE_WLF_SCENE_CIRCLE_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

struct wlf_scene_circle {
	struct wlf_scene_node base;
	struct wlf_color color;
	double radius;
	double stroke_width;
};

struct wlf_scene_circle *wlf_scene_circle_create(struct wlf_scene_tree *parent,
	double radius, const struct wlf_color *color, double stroke_width);
void wlf_scene_circle_set_radius(struct wlf_scene_circle *circle, double radius);
void wlf_scene_circle_set_color(struct wlf_scene_circle *circle,
	const struct wlf_color *color);
void wlf_scene_circle_set_stroke_width(struct wlf_scene_circle *circle,
	double stroke_width);

bool wlf_scene_node_is_circle(const struct wlf_scene_node *node);
struct wlf_scene_circle *wlf_scene_circle_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_CIRCLE_H
