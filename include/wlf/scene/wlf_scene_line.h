/**
 * @file        wlf_scene_line.h
 * @brief       Line scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_LINE_H
#define SCENE_WLF_SCENE_LINE_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

struct wlf_scene_line {
	struct wlf_scene_node base;
	struct wlf_color color;
	double stroke_width;
	double x1, y1;
	double x2, y2;
};

struct wlf_scene_line *wlf_scene_line_create(struct wlf_scene_tree *parent,
	double x1, double y1, double x2, double y2,
	const struct wlf_color *color, double stroke_width);
void wlf_scene_line_set_points(struct wlf_scene_line *line,
	double x1, double y1, double x2, double y2);
void wlf_scene_line_set_color(struct wlf_scene_line *line,
	const struct wlf_color *color);
void wlf_scene_line_set_stroke_width(struct wlf_scene_line *line,
	double stroke_width);

bool wlf_scene_node_is_line(const struct wlf_scene_node *node);
struct wlf_scene_line *wlf_scene_line_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_LINE_H
