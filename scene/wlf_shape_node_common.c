#include "wlf_shape_node_common.h"

#include <math.h>

bool wlf_shape_node_common_init(struct wlf_scene_node *node,
		const struct wlf_scene_node_impl *impl, struct wlf_scene_node *parent,
		double x, double y, double minx, double miny, double maxx, double maxy) {
	if (node == NULL || impl == NULL || parent == NULL ||
			!isfinite(minx) || !isfinite(miny) || maxx < minx || maxy < miny) {
		return false;
	}
	wlf_scene_node_init(node, impl, parent);
	node->state.x = x;
	node->state.y = y;
	node->state.width = ceil(maxx - minx);
	node->state.height = ceil(maxy - miny);
	return true;
}

bool wlf_shape_node_common_refresh(struct wlf_scene_node *node,
		double minx, double miny, double maxx, double maxy,
		double *offset_x, double *offset_y) {
	if (!isfinite(minx) || !isfinite(miny) || maxx < minx || maxy < miny) return false;
	node->state.width = ceil(maxx - minx);
	node->state.height = ceil(maxy - miny);
	double x, y;
	if (!wlf_scene_node_coords(node, &x, &y)) return false;
	*offset_x = x - minx;
	*offset_y = y - miny;
	return true;
}

void wlf_shape_node_common_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	*width = node->state.width;
	*height = node->state.height;
}

bool wlf_shape_node_common_invisible(struct wlf_scene_node *node) {
	return !node->state.enabled || node->state.opacity <= 0 ||
		node->state.width <= 0 || node->state.height <= 0;
}

void wlf_shape_node_common_visibility(struct wlf_scene_node *node,
		pixman_region32_t *visible) {
	if (wlf_shape_node_common_invisible(node)) return;
	double x, y;
	if (wlf_scene_node_coords(node, &x, &y))
		pixman_region32_union_rect(visible, visible, (int)x, (int)y,
			(uint32_t)node->state.width, (uint32_t)node->state.height);
}

struct wlf_scene_node *wlf_shape_node_common_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	if (wlf_shape_node_common_invisible(node) || lx < 0 || ly < 0 ||
			lx >= node->state.width || ly >= node->state.height) return NULL;
	if (nx != NULL) *nx = lx;
	if (ny != NULL) *ny = ly;
	return node;
}

void wlf_shape_node_common_bounds(struct wlf_scene_node *node,
		double x, double y, pixman_region32_t *visible) {
	if (!wlf_shape_node_common_invisible(node))
		pixman_region32_union_rect(visible, visible, (int)x, (int)y,
			(uint32_t)node->state.width, (uint32_t)node->state.height);
}

bool wlf_shape_node_common_in_box(struct wlf_scene_node *node,
		struct wlf_frect *box, scene_node_box_iterator_func_t iterator, void *data) {
	if (wlf_shape_node_common_invisible(node)) return false;
	double x, y;
	if (!wlf_scene_node_coords(node, &x, &y)) return false;
	if (x >= box->x + box->width || x + node->state.width <= box->x ||
			y >= box->y + box->height || y + node->state.height <= box->y) return false;
	return iterator(node, x, y, data);
}
