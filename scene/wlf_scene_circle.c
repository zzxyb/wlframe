#include "wlf/scene/wlf_scene_circle.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static double clamp_non_negative(double value) {
	return value < 0.0 ? 0.0 : value;
}

static bool point_on_circle(const struct wlf_scene_circle *circle,
		double x, double y) {
	double radius = circle->radius;
	double stroke_width = circle->stroke_width;
	double cx = radius;
	double cy = radius;
	double dx = x - cx;
	double dy = y - cy;
	double distance = sqrt(dx * dx + dy * dy);
	double half = stroke_width / 2.0;
	return distance >= radius - half && distance <= radius + half;
}

static void scene_circle_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_circle *circle = (struct wlf_scene_circle *)node;
	free(circle);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_circle *circle = wlf_scene_circle_from_node(node);
	*width = circle->radius * 2.0;
	*height = circle->radius * 2.0;
}

static void scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
		double y, struct wlf_region *opaque) {
	(void)node;
	(void)x;
	(void)y;
	wlf_region_clear(opaque);
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_circle *circle = wlf_scene_circle_from_node(node);
	return circle->color.a == 0.0 || circle->radius <= 0.0 ||
		circle->stroke_width <= 0.0;
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
	struct wlf_scene_circle *circle = wlf_scene_circle_from_node(node);
	if (!point_on_circle(circle, lx, ly)) {
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

	double half = wlf_scene_circle_from_node(node)->stroke_width / 2.0;
	struct wlf_frect node_box = {
		.x = lx - half,
		.y = ly - half,
		.width = node->state.width + half * 2.0,
		.height = node->state.height + half * 2.0,
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
	.destroy = scene_circle_destroy,
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

struct wlf_scene_circle *wlf_scene_circle_create(struct wlf_scene_tree *parent,
		double radius, const struct wlf_color *color, double stroke_width) {
	assert(parent != NULL);
	assert(color != NULL);

	struct wlf_scene_circle *circle = calloc(1, sizeof(*circle));
	if (circle == NULL) {
		return NULL;
	}

	wlf_scene_node_init(&circle->base, &scene_node_impl, &parent->base);
	circle->radius = clamp_non_negative(radius);
	circle->stroke_width = clamp_non_negative(stroke_width);
	circle->color = *color;
	circle->base.state.width = circle->radius * 2.0;
	circle->base.state.height = circle->radius * 2.0;
	return circle;
}

void wlf_scene_circle_set_radius(struct wlf_scene_circle *circle, double radius) {
	radius = clamp_non_negative(radius);
	if (circle->radius == radius) {
		return;
	}

	circle->radius = radius;
	circle->base.state.width = radius * 2.0;
	circle->base.state.height = radius * 2.0;
	wlf_scene_node_update(&circle->base, NULL);
}

void wlf_scene_circle_set_color(struct wlf_scene_circle *circle,
		const struct wlf_color *color) {
	if (wlf_color_equal(&circle->color, color)) {
		return;
	}

	circle->color = *color;
	wlf_scene_node_update(&circle->base, NULL);
}

void wlf_scene_circle_set_stroke_width(struct wlf_scene_circle *circle,
		double stroke_width) {
	stroke_width = clamp_non_negative(stroke_width);
	if (circle->stroke_width == stroke_width) {
		return;
	}

	circle->stroke_width = stroke_width;
	wlf_scene_node_update(&circle->base, NULL);
}

bool wlf_scene_node_is_circle(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_circle *wlf_scene_circle_from_node(struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);
	struct wlf_scene_circle *circle = NULL;
	return wlf_container_of(node, circle, base);
}
