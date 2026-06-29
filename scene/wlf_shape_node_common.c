#include "wlf_shape_node_common.h"

#include <math.h>

bool wlf_shape_node_common_init(struct wlf_scene_node *node,
		const struct wlf_scene_node_impl *impl, struct wlf_scene_node *parent,
		int x, int y, double minx, double miny, double maxx, double maxy) {
	if (node == NULL || impl == NULL || parent == NULL ||
			!isfinite(minx) || !isfinite(miny) || maxx < minx || maxy < miny) {
		return false;
	}
	wlf_scene_node_init(node, impl, parent);
	node->state.x = x;
	node->state.y = y;
	node->state.width = (uint32_t)ceil(maxx - minx);
	node->state.height = (uint32_t)ceil(maxy - miny);
	return true;
}

bool wlf_shape_node_common_refresh_at(struct wlf_scene_node *node,
		double minx, double miny, double maxx, double maxy, double x, double y,
		double *offset_x, double *offset_y) {
	if (!isfinite(minx) || !isfinite(miny) || maxx < minx || maxy < miny) {
		return false;
	}
	node->state.width = (uint32_t)ceil(maxx - minx);
	node->state.height = (uint32_t)ceil(maxy - miny);
	*offset_x = x - minx;
	*offset_y = y - miny;
	return true;
}

void wlf_shape_node_common_get_size(struct wlf_scene_node *node,
		uint32_t *width, uint32_t *height) {
	*width = node->state.width;
	*height = node->state.height;
}

bool wlf_shape_node_common_invisible(struct wlf_scene_node *node) {
	return !node->state.enabled || node->state.opacity <= 0 ||
		node->state.width <= 0 || node->state.height <= 0;
}

void wlf_shape_node_common_visibility(struct wlf_scene_node *node,
		pixman_region32_t *visible) {
	if (!node->state.enabled) {
		return;
	}
	pixman_region32_union(visible, visible, &node->state.visible);
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
		int x, int y, pixman_region32_t *visible) {
	if (!wlf_shape_node_common_invisible(node))
		pixman_region32_union_rect(visible, visible, x, y,
			node->state.width, node->state.height);
}

bool wlf_shape_node_common_in_box(struct wlf_scene_node *node,
		struct wlf_frect *box, scene_node_box_iterator_func_t iterator, void *data) {
	if (wlf_shape_node_common_invisible(node)) return false;
	int x, y;
	if (!wlf_scene_node_coords(node, &x, &y)) return false;
	if (x >= box->x + box->width || x + node->state.width <= box->x ||
			y >= box->y + box->height || y + node->state.height <= box->y) return false;
	return iterator(node, x, y, data);
}

bool wlf_shape_node_common_construct_render_list_iterator(
		struct wlf_scene_node *node, int lx, int ly, void *data) {
	return wlf_scene_node_add_render_list_entry(node, lx, ly, data);
}
