/**
 * @file        wlf_scene_polygon.h
 * @brief       Polygon scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_POLYGON_H
#define SCENE_WLF_SCENE_POLYGON_H

#include "wlf/math/wlf_fpoint.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"

#include <stddef.h>

struct wlf_scene_polygon {
	struct wlf_scene_node base;
	struct wlf_color color;
	struct wlf_fpoint *points;
	size_t point_count;
};

struct wlf_scene_polygon *wlf_scene_polygon_create(struct wlf_scene_tree *parent,
	const struct wlf_fpoint *points, size_t point_count,
	const struct wlf_color *color);
void wlf_scene_polygon_set_points(struct wlf_scene_polygon *polygon,
	const struct wlf_fpoint *points, size_t point_count);
void wlf_scene_polygon_set_color(struct wlf_scene_polygon *polygon,
	const struct wlf_color *color);

bool wlf_scene_node_is_polygon(const struct wlf_scene_node *node);
struct wlf_scene_polygon *wlf_scene_polygon_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_POLYGON_H
