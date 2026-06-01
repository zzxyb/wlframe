#include "wlf/scene/wlf_scene_lame_rect.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static double clamp_positive_exponent(double value) {
	return value > 0.0 ? value : 2.0;
}

static bool point_in_superellipse(double width, double height,
		double lame_m, double lame_n, double x, double y) {
	if (x < 0.0 || y < 0.0 || x > width || y > height) {
		return false;
	}
	if (width <= 0.0 || height <= 0.0) {
		return false;
	}

	double a = width / 2.0;
	double b = height / 2.0;
	double cx = a;
	double cy = b;
	lame_m = clamp_positive_exponent(lame_m);
	lame_n = clamp_positive_exponent(lame_n);

	double nx = fabs((x - cx) / a);
	double ny = fabs((y - cy) / b);
	return pow(nx, lame_m) + pow(ny, lame_n) <= 1.0;
}

static void scene_lame_rect_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_lame_rect *rect = (struct wlf_scene_lame_rect *)node;
	free(rect);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_lame_rect *scene_rect = wlf_scene_lame_rect_from_node(node);
	*width = scene_rect->base.state.width;
	*height = scene_rect->base.state.height;
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_lame_rect *scene_rect = wlf_scene_lame_rect_from_node(node);
	return scene_rect->color.a == 0.0;
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
	at_data->rx = at_data->lx - lx;
	at_data->ry = at_data->ly - ly;
	at_data->node = node;
	return true;
}

static struct wlf_scene_node *scene_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	struct wlf_scene_lame_rect *scene_rect = wlf_scene_lame_rect_from_node(node);
	if (!point_in_superellipse(scene_rect->base.state.width,
			scene_rect->base.state.height,
			scene_rect->lame_m, scene_rect->lame_n, lx, ly)) {
		return NULL;
	}

	struct wlf_node_at_data data = {
		.lx = lx,
		.ly = ly
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
	.destroy = scene_lame_rect_destroy,
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

struct wlf_scene_lame_rect *wlf_scene_lame_rect_create(
		struct wlf_scene_tree *parent, double width, double height,
		const struct wlf_color *color) {
	assert(parent != NULL);
	assert(color != NULL);

	struct wlf_scene_lame_rect *rect = calloc(1, sizeof(*rect));
	if (rect == NULL) {
		return NULL;
	}

	wlf_scene_node_init(&rect->base, &scene_node_impl, &parent->base);
	rect->base.state.width = width;
	rect->base.state.height = height;
	rect->color = *color;
	rect->lame_m = 2.0;
	rect->lame_n = 2.0;
	return rect;
}

void wlf_scene_lame_rect_set_size(struct wlf_scene_lame_rect *rect,
		double width, double height) {
	if (rect->base.state.width == width &&
			rect->base.state.height == height) {
		return;
	}

	assert(width >= 0.0 && height >= 0.0);
	rect->base.state.width = width;
	rect->base.state.height = height;
	wlf_scene_node_update(&rect->base, NULL);
}

void wlf_scene_lame_rect_set_color(struct wlf_scene_lame_rect *rect,
		const struct wlf_color *color) {
	if (wlf_color_equal(&rect->color, color)) {
		return;
	}

	rect->color = *color;
	wlf_scene_node_update(&rect->base, NULL);
}

void wlf_scene_lame_rect_set_lame_exponents(struct wlf_scene_lame_rect *rect,
		double lame_m, double lame_n) {
	lame_m = clamp_positive_exponent(lame_m);
	lame_n = clamp_positive_exponent(lame_n);
	if (rect->lame_m == lame_m && rect->lame_n == lame_n) {
		return;
	}

	rect->lame_m = lame_m;
	rect->lame_n = lame_n;
	wlf_scene_node_update(&rect->base, NULL);
}

bool wlf_scene_node_is_lame_rect(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_lame_rect *wlf_scene_lame_rect_from_node(
		struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);
	struct wlf_scene_lame_rect *scene_rect = NULL;
	return wlf_container_of(node, scene_rect, base);
}
