#include "wlf/scene/wlf_scene_ellipse.h"

#include <assert.h>
#include <stdlib.h>

static double clamp_non_negative(double value) {
	return value < 0.0 ? 0.0 : value;
}

static bool point_in_ellipse(const struct wlf_scene_ellipse *ellipse,
		double x, double y) {
	if (ellipse->radius_x <= 0.0 || ellipse->radius_y <= 0.0) {
		return false;
	}

	double nx = (x - ellipse->radius_x) / ellipse->radius_x;
	double ny = (y - ellipse->radius_y) / ellipse->radius_y;
	return nx * nx + ny * ny <= 1.0;
}

static void scene_ellipse_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_ellipse *ellipse = (struct wlf_scene_ellipse *)node;
	free(ellipse);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_ellipse *ellipse = wlf_scene_ellipse_from_node(node);
	*width = ellipse->radius_x * 2.0;
	*height = ellipse->radius_y * 2.0;
}

static void scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
		double y, struct wlf_region *opaque) {
	struct wlf_scene_ellipse *ellipse = wlf_scene_ellipse_from_node(node);
	wlf_region_clear(opaque);

	if (!node->state.enabled || ellipse->color.a != 1.0 ||
			ellipse->radius_x <= 0.0 || ellipse->radius_y <= 0.0) {
		return;
	}

	struct wlf_frect rect = {
		.x = x,
		.y = y,
		.width = ellipse->radius_x * 2.0,
		.height = ellipse->radius_y * 2.0,
	};
	wlf_region_union_rect(opaque, opaque, &rect);
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_ellipse *ellipse = wlf_scene_ellipse_from_node(node);
	return ellipse->color.a == 0.0 || ellipse->radius_x <= 0.0 ||
		ellipse->radius_y <= 0.0;
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
	struct wlf_scene_ellipse *ellipse = wlf_scene_ellipse_from_node(node);
	if (!point_in_ellipse(ellipse, lx, ly)) {
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
	.destroy = scene_ellipse_destroy,
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

struct wlf_scene_ellipse *wlf_scene_ellipse_create(struct wlf_scene_tree *parent,
		double radius_x, double radius_y, const struct wlf_color *color) {
	assert(parent != NULL);
	assert(color != NULL);

	struct wlf_scene_ellipse *ellipse = calloc(1, sizeof(*ellipse));
	if (ellipse == NULL) {
		return NULL;
	}

	wlf_scene_node_init(&ellipse->base, &scene_node_impl, &parent->base);
	ellipse->radius_x = clamp_non_negative(radius_x);
	ellipse->radius_y = clamp_non_negative(radius_y);
	ellipse->color = *color;
	ellipse->base.state.width = ellipse->radius_x * 2.0;
	ellipse->base.state.height = ellipse->radius_y * 2.0;
	return ellipse;
}

void wlf_scene_ellipse_set_radii(struct wlf_scene_ellipse *ellipse,
		double radius_x, double radius_y) {
	radius_x = clamp_non_negative(radius_x);
	radius_y = clamp_non_negative(radius_y);
	if (ellipse->radius_x == radius_x && ellipse->radius_y == radius_y) {
		return;
	}

	ellipse->radius_x = radius_x;
	ellipse->radius_y = radius_y;
	ellipse->base.state.width = radius_x * 2.0;
	ellipse->base.state.height = radius_y * 2.0;
	wlf_scene_node_update(&ellipse->base, NULL);
}

void wlf_scene_ellipse_set_color(struct wlf_scene_ellipse *ellipse,
		const struct wlf_color *color) {
	if (wlf_color_equal(&ellipse->color, color)) {
		return;
	}

	ellipse->color = *color;
	wlf_scene_node_update(&ellipse->base, NULL);
}

bool wlf_scene_node_is_ellipse(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_ellipse *wlf_scene_ellipse_from_node(struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);
	struct wlf_scene_ellipse *ellipse = NULL;
	return wlf_container_of(node, ellipse, base);
}
