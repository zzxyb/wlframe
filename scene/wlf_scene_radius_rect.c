#include "wlf/scene/wlf_scene_radius_rect.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static double clamp_radius(double radius, double width, double height) {
	double max_radius = fmin(width, height) / 2.0;
	if (max_radius < 0.0) {
		max_radius = 0.0;
	}
	if (radius < 0.0) {
		return 0.0;
	}
	return fmin(radius, max_radius);
}

static bool point_in_rounded_rect(const struct wlf_scene_radius_rect *scene_rect,
		double x, double y) {
	double width = scene_rect->base.state.width;
	double height = scene_rect->base.state.height;
	if (x < 0.0 || y < 0.0 || x > width || y > height) {
		return false;
	}

	double tl = clamp_radius(scene_rect->radius_top_left, width, height);
	double tr = clamp_radius(scene_rect->radius_top_right, width, height);
	double br = clamp_radius(scene_rect->radius_bottom_right, width, height);
	double bl = clamp_radius(scene_rect->radius_bottom_left, width, height);

	double dx, dy, radius;

	if (x < tl && y < tl && tl > 0.0) {
		dx = x - tl;
		dy = y - tl;
		radius = tl;
		return dx * dx + dy * dy <= radius * radius;
	}
	if (x > width - tr && y < tr && tr > 0.0) {
		dx = x - (width - tr);
		dy = y - tr;
		radius = tr;
		return dx * dx + dy * dy <= radius * radius;
	}
	if (x > width - br && y > height - br && br > 0.0) {
		dx = x - (width - br);
		dy = y - (height - br);
		radius = br;
		return dx * dx + dy * dy <= radius * radius;
	}
	if (x < bl && y > height - bl && bl > 0.0) {
		dx = x - bl;
		dy = y - (height - bl);
		radius = bl;
		return dx * dx + dy * dy <= radius * radius;
	}

	return true;
}

static void scene_radius_rect_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_radius_rect *rect = (struct wlf_scene_radius_rect *)node;
	free(rect);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_radius_rect *scene_rect =
		wlf_scene_radius_rect_from_node(node);
	*width = scene_rect->base.state.width;
	*height = scene_rect->base.state.height;
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_radius_rect *scene_rect =
		wlf_scene_radius_rect_from_node(node);

	return scene_rect->color.a == 0.f;
}

static void scene_node_visibility(struct wlf_scene_node *node,
		struct wlf_region *visible) {
	if (!node->state.enabled) {
		return;
	}

	wlf_region_union(visible, visible, &node->state.visible);
}

static bool scene_node_at_iterator(struct wlf_scene_node *node,
		double lx, double ly, void *data) {
	struct wlf_node_at_data *at_data = data;

	double rx = at_data->lx - lx;
	double ry = at_data->ly - ly;

	at_data->rx = rx;
	at_data->ry = ry;
	at_data->node = node;
	return true;
}

static struct wlf_scene_node *scene_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	struct wlf_scene_radius_rect *scene_rect =
		wlf_scene_radius_rect_from_node(node);
	if (!point_in_rounded_rect(scene_rect, lx, ly)) {
		return NULL;
	}

	struct wlf_node_at_data data = {
		.lx = lx,
		.ly = ly
	};
	if (scene_node_at_iterator(node, 0.0, 0.0, &data)) {
		if (nx) {
			*nx = data.rx;
		}
		if (ny) {
			*ny = data.ry;
		}
		return data.node;
	}

	return NULL;
}

static bool _scene_nodes_in_box(struct wlf_scene_node *node, struct wlf_frect *box,
		scene_node_box_iterator_func_t iterator, void *user_data,
		double lx, double ly) {
	if (!node->state.enabled) {
		return false;
	}

	struct wlf_frect node_box = { .x = lx, .y = ly };
	scene_node_get_size(node, &node_box.width, &node_box.height);

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
	.destroy = scene_radius_rect_destroy,
	.set_enabled = NULL,
	.set_position = NULL,
	.set_opacity = NULL,
	.get_size = scene_node_get_size,
	.get_children = NULL,
	.get_opaque_region = NULL,
	.invisible = scene_node_invisible,
	.visibility = scene_node_visibility,
	.at = scene_node_at,
	.coords = NULL,
	.update = NULL,
	.bounds = NULL,
	.in_box = scene_nodes_in_box,
};

struct wlf_scene_radius_rect *wlf_scene_radius_rect_create(
		struct wlf_scene_tree *parent, double width, double height,
		const struct wlf_color *color) {
	assert(parent != NULL);

	struct wlf_scene_radius_rect *rect = calloc(1, sizeof(*rect));
	if (rect == NULL) {
		return NULL;
	}

	wlf_scene_node_init(&rect->base, &scene_node_impl, &parent->base);
	rect->base.state.width = width;
	rect->base.state.height = height;
	rect->color = *color;

	return rect;
}

void wlf_scene_radius_rect_set_size(struct wlf_scene_radius_rect *rect,
		double width, double height) {
	if (rect->base.state.width == width && rect->base.state.height == height) {
		return;
	}

	assert(width >= 0 && height >= 0);

	rect->base.state.width = width;
	rect->base.state.height = height;
	wlf_scene_node_update(&rect->base, NULL);
}

void wlf_scene_radius_rect_set_color(struct wlf_scene_radius_rect *rect,
		const struct wlf_color *color) {
	if (wlf_color_equal(&rect->color, color)) {
		return;
	}

	rect->color = *color;
	wlf_scene_node_update(&rect->base, NULL);
}

void wlf_scene_radius_rect_set_corner_radii(struct wlf_scene_radius_rect *rect,
		double top_left, double top_right, double bottom_right,
		double bottom_left) {
	assert(top_left >= 0.0);
	assert(top_right >= 0.0);
	assert(bottom_right >= 0.0);
	assert(bottom_left >= 0.0);

	if (rect->radius_top_left == top_left &&
			rect->radius_top_right == top_right &&
			rect->radius_bottom_right == bottom_right &&
			rect->radius_bottom_left == bottom_left) {
		return;
	}

	rect->radius_top_left = top_left;
	rect->radius_top_right = top_right;
	rect->radius_bottom_right = bottom_right;
	rect->radius_bottom_left = bottom_left;
	wlf_scene_node_update(&rect->base, NULL);
}

bool wlf_scene_node_is_radius_rect(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_radius_rect *wlf_scene_radius_rect_from_node(
		struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);

	struct wlf_scene_radius_rect *scene_rect =
		wlf_container_of(node, scene_rect, base);

	return scene_rect;
}
