/**
 * @file        wlf_scene_bezier_curve.h
 * @brief       Bezier curve scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_BEZIER_CURVE_H
#define SCENE_WLF_SCENE_BEZIER_CURVE_H

#include "wlf/math/wlf_fpoint.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

struct wlf_scene_bezier_curve {
	struct wlf_scene_node base;
	struct wlf_color color;
	double stroke_width;
	struct wlf_fpoint p0;
	struct wlf_fpoint p1;
	struct wlf_fpoint p2;
	struct wlf_fpoint p3;
};

struct wlf_scene_bezier_curve *wlf_scene_bezier_curve_create(
	struct wlf_scene_tree *parent, const struct wlf_fpoint *p0,
	const struct wlf_fpoint *p1, const struct wlf_fpoint *p2,
	const struct wlf_fpoint *p3, const struct wlf_color *color,
	double stroke_width);
void wlf_scene_bezier_curve_set_points(struct wlf_scene_bezier_curve *curve,
	const struct wlf_fpoint *p0, const struct wlf_fpoint *p1,
	const struct wlf_fpoint *p2, const struct wlf_fpoint *p3);
void wlf_scene_bezier_curve_set_color(struct wlf_scene_bezier_curve *curve,
	const struct wlf_color *color);
void wlf_scene_bezier_curve_set_stroke_width(
	struct wlf_scene_bezier_curve *curve, double stroke_width);

bool wlf_scene_node_is_bezier_curve(const struct wlf_scene_node *node);
struct wlf_scene_bezier_curve *wlf_scene_bezier_curve_from_node(
	struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_BEZIER_CURVE_H
