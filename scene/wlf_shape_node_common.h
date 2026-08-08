#ifndef WLF_SHAPE_NODE_COMMON_H
#define WLF_SHAPE_NODE_COMMON_H

#include "wlf/scene/wlf_scene_node.h"

bool wlf_shape_node_common_init(struct wlf_scene_node *node,
	const struct wlf_scene_node_impl *impl, struct wlf_scene_node *parent,
	double x, double y, double minx, double miny, double maxx, double maxy);
bool wlf_shape_node_common_refresh(struct wlf_scene_node *node,
	double minx, double miny, double maxx, double maxy,
	double *offset_x, double *offset_y);
void wlf_shape_node_common_get_size(struct wlf_scene_node *node,
	double *width, double *height);
bool wlf_shape_node_common_invisible(struct wlf_scene_node *node);
void wlf_shape_node_common_visibility(struct wlf_scene_node *node,
	pixman_region32_t *visible);
struct wlf_scene_node *wlf_shape_node_common_at(struct wlf_scene_node *node,
	double lx, double ly, double *nx, double *ny);
void wlf_shape_node_common_bounds(struct wlf_scene_node *node,
	double x, double y, pixman_region32_t *visible);
bool wlf_shape_node_common_in_box(struct wlf_scene_node *node,
	struct wlf_frect *box, scene_node_box_iterator_func_t iterator, void *data);

#endif
