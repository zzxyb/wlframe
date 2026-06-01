#include "wlf/scene/wlf_scene_polygon.h"

#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

static bool scene_polygon_set_points_internal(struct wlf_scene_polygon *polygon,
		const struct wlf_fpoint *points, size_t point_count) {
	if (point_count == 0 || points == NULL) {
		free(polygon->points);
		polygon->points = NULL;
		polygon->point_count = 0;
		polygon->base.state.width = 0.0;
		polygon->base.state.height = 0.0;
		return true;
	}

	struct wlf_fpoint *copy = malloc(sizeof(*copy) * point_count);
	if (copy == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate polygon points");
		return false;
	}

	double min_x = DBL_MAX, min_y = DBL_MAX;
	double max_x = -DBL_MAX, max_y = -DBL_MAX;
	for (size_t i = 0; i < point_count; ++i) {
		if (points[i].x < min_x) {
			min_x = points[i].x;
		}
		if (points[i].y < min_y) {
			min_y = points[i].y;
		}
		if (points[i].x > max_x) {
			max_x = points[i].x;
		}
		if (points[i].y > max_y) {
			max_y = points[i].y;
		}
	}

	for (size_t i = 0; i < point_count; ++i) {
		copy[i].x = points[i].x - min_x;
		copy[i].y = points[i].y - min_y;
	}

	free(polygon->points);
	polygon->points = copy;
	polygon->point_count = point_count;
	polygon->base.state.width = max_x - min_x;
	polygon->base.state.height = max_y - min_y;
	return true;
}

static bool point_in_polygon(const struct wlf_scene_polygon *polygon,
		double x, double y) {
	if (polygon->point_count < 3) {
		return false;
	}

	bool inside = false;
	for (size_t i = 0, j = polygon->point_count - 1;
			i < polygon->point_count; j = i++) {
		const struct wlf_fpoint *pi = &polygon->points[i];
		const struct wlf_fpoint *pj = &polygon->points[j];
		bool intersects = ((pi->y > y) != (pj->y > y)) &&
			(x < (pj->x - pi->x) * (y - pi->y) / (pj->y - pi->y) + pi->x);
		if (intersects) {
			inside = !inside;
		}
	}

	return inside;
}

static void scene_polygon_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_polygon *polygon = (struct wlf_scene_polygon *)node;
	free(polygon->points);
	free(polygon);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_polygon *polygon = wlf_scene_polygon_from_node(node);
	*width = polygon->base.state.width;
	*height = polygon->base.state.height;
}

static void scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
		double y, struct wlf_region *opaque) {
	struct wlf_scene_polygon *polygon = wlf_scene_polygon_from_node(node);
	wlf_region_clear(opaque);

	if (!node->state.enabled || polygon->color.a != 1.0 || polygon->point_count < 3) {
		return;
	}

	struct wlf_frect rect = {
		.x = x,
		.y = y,
		.width = polygon->base.state.width,
		.height = polygon->base.state.height,
	};
	wlf_region_union_rect(opaque, opaque, &rect);
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_polygon *polygon = wlf_scene_polygon_from_node(node);
	return polygon->color.a == 0.0 || polygon->point_count < 3;
}

static void scene_node_visibility(struct wlf_scene_node *node,
		struct wlf_region *visible) {
	if (!node->state.enabled || wlf_scene_node_invisible(node)) {
		return;
	}

	wlf_region_union(visible, visible, &node->state.visible);
}

static bool scene_node_at_iterator(struct wlf_scene_node *node,
		double lx, double ly, void *data) {
	struct wlf_node_at_data *at_data = data;
	at_data->rx = at_data->lx - lx;
	at_data->ry = at_data->ly - ly;
	at_data->node = node;
	return true;
}

static struct wlf_scene_node *scene_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	struct wlf_scene_polygon *polygon = wlf_scene_polygon_from_node(node);
	if (!point_in_polygon(polygon, lx, ly)) {
		return NULL;
	}

	struct wlf_node_at_data data = {
		.lx = lx,
		.ly = ly,
	};
	scene_node_at_iterator(node, 0.0, 0.0, &data);
	if (nx) {
		*nx = data.rx;
	}
	if (ny) {
		*ny = data.ry;
	}
	return data.node;
}

static bool _scene_nodes_in_box(struct wlf_scene_node *node, struct wlf_frect *box,
		scene_node_box_iterator_func_t iterator, void *user_data,
		double lx, double ly) {
	if (!node->state.enabled || wlf_scene_node_invisible(node)) {
		return false;
	}

	struct wlf_frect node_box = {
		.x = lx,
		.y = ly,
		.width = node->state.width,
		.height = node->state.height,
	};
	if (wlf_frect_intersection(&node_box, &node_box, box) &&
			iterator(node, lx, ly, user_data)) {
		return true;
	}
	return false;
}

static bool scene_nodes_in_box(struct wlf_scene_node *node, struct wlf_frect *box,
		scene_node_box_iterator_func_t iterator, void *user_data) {
	double x, y;
	wlf_scene_node_coords(node, &x, &y);
	return _scene_nodes_in_box(node, box, iterator, user_data, x, y);
}

static const struct wlf_scene_node_impl scene_node_impl = {
	.destroy = scene_polygon_destroy,
	.set_enabled = NULL,
	.set_position = NULL,
	.set_opacity = NULL,
	.get_size = scene_node_get_size,
	.get_children = NULL,
	.get_opaque_region = scene_node_get_opaque_region,
	.invisible = scene_node_invisible,
	.visibility = scene_node_visibility,
	.at = scene_node_at,
	.coords = NULL,
	.update = NULL,
	.bounds = NULL,
	.in_box = scene_nodes_in_box,
};

struct wlf_scene_polygon *wlf_scene_polygon_create(struct wlf_scene_tree *parent,
		const struct wlf_fpoint *points, size_t point_count,
		const struct wlf_color *color) {
	assert(parent != NULL);
	assert(color != NULL);

	struct wlf_scene_polygon *polygon = calloc(1, sizeof(*polygon));
	if (polygon == NULL) {
		return NULL;
	}

	wlf_scene_node_init(&polygon->base, &scene_node_impl, &parent->base);
	polygon->color = *color;
	if (!scene_polygon_set_points_internal(polygon, points, point_count)) {
		free(polygon);
		return NULL;
	}

	return polygon;
}

void wlf_scene_polygon_set_points(struct wlf_scene_polygon *polygon,
		const struct wlf_fpoint *points, size_t point_count) {
	if (!scene_polygon_set_points_internal(polygon, points, point_count)) {
		return;
	}

	wlf_scene_node_update(&polygon->base, NULL);
}

void wlf_scene_polygon_set_color(struct wlf_scene_polygon *polygon,
		const struct wlf_color *color) {
	if (wlf_color_equal(&polygon->color, color)) {
		return;
	}

	polygon->color = *color;
	wlf_scene_node_update(&polygon->base, NULL);
}

bool wlf_scene_node_is_polygon(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_polygon *wlf_scene_polygon_from_node(struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);
	struct wlf_scene_polygon *polygon = NULL;
	return wlf_container_of(node, polygon, base);
}
