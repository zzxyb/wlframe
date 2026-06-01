#include "wlf/scene/wlf_scene_radius_border.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static double clamp_border_width(double border_width, double width, double height) {
	double max_border_width = fmin(width, height);
	if (max_border_width < 0.0) {
		max_border_width = 0.0;
	}
	if (border_width < 0.0) {
		return 0.0;
	}
	return fmin(border_width, max_border_width);
}

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

static bool point_in_rounded_rect(double width, double height,
		double top_left, double top_right, double bottom_right,
		double bottom_left, double x, double y) {
	if (x < 0.0 || y < 0.0 || x > width || y > height) {
		return false;
	}

	double tl = clamp_radius(top_left, width, height);
	double tr = clamp_radius(top_right, width, height);
	double br = clamp_radius(bottom_right, width, height);
	double bl = clamp_radius(bottom_left, width, height);
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

static bool point_in_radius_border(
		const struct wlf_scene_radius_border *border, double x, double y) {
	double width = border->base.state.width;
	double height = border->base.state.height;
	double border_width = clamp_border_width(border->border_width, width, height);
	if (border_width <= 0.0) {
		return false;
	}

	if (!point_in_rounded_rect(width, height,
			border->radius_top_left, border->radius_top_right,
			border->radius_bottom_right, border->radius_bottom_left, x, y)) {
		return false;
	}

	double inner_width = width - border_width * 2.0;
	double inner_height = height - border_width * 2.0;
	if (inner_width <= 0.0 || inner_height <= 0.0) {
		return true;
	}

	double inner_x = x - border_width;
	double inner_y = y - border_width;
	double inner_top_left =
		fmax(0.0, clamp_radius(border->radius_top_left, width, height) - border_width);
	double inner_top_right =
		fmax(0.0, clamp_radius(border->radius_top_right, width, height) - border_width);
	double inner_bottom_right =
		fmax(0.0, clamp_radius(border->radius_bottom_right, width, height) - border_width);
	double inner_bottom_left =
		fmax(0.0, clamp_radius(border->radius_bottom_left, width, height) - border_width);

	return !point_in_rounded_rect(inner_width, inner_height,
		inner_top_left, inner_top_right, inner_bottom_right, inner_bottom_left,
		inner_x, inner_y);
}

static void scene_radius_border_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_radius_border *border =
		(struct wlf_scene_radius_border *)node;
	free(border);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_radius_border *scene_border =
		wlf_scene_radius_border_from_node(node);
	*width = scene_border->base.state.width;
	*height = scene_border->base.state.height;
}

static void scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
		double y, struct wlf_region *opaque) {
	(void)node;
	(void)x;
	(void)y;
	wlf_region_clear(opaque);
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_radius_border *scene_border =
		wlf_scene_radius_border_from_node(node);
	return scene_border->color.a == 0.0 ||
		clamp_border_width(scene_border->border_width,
			scene_border->base.state.width,
			scene_border->base.state.height) <= 0.0;
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
	struct wlf_scene_radius_border *scene_border =
		wlf_scene_radius_border_from_node(node);
	if (!point_in_radius_border(scene_border, lx, ly)) {
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
	.destroy = scene_radius_border_destroy,
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

struct wlf_scene_radius_border *wlf_scene_radius_border_create(
		struct wlf_scene_tree *parent, double width, double height,
		const struct wlf_color *color, double border_width) {
	assert(parent != NULL);
	assert(color != NULL);
	assert(width >= 0.0 && height >= 0.0);

	struct wlf_scene_radius_border *border = calloc(1, sizeof(*border));
	if (border == NULL) {
		return NULL;
	}

	wlf_scene_node_init(&border->base, &scene_node_impl, &parent->base);
	border->base.state.width = width;
	border->base.state.height = height;
	border->color = *color;
	border->border_width = clamp_border_width(border_width, width, height);
	return border;
}

void wlf_scene_radius_border_set_size(struct wlf_scene_radius_border *border,
		double width, double height) {
	if (border->base.state.width == width &&
			border->base.state.height == height) {
		return;
	}

	assert(width >= 0.0 && height >= 0.0);

	border->base.state.width = width;
	border->base.state.height = height;
	border->border_width = clamp_border_width(border->border_width, width, height);
	wlf_scene_node_update(&border->base, NULL);
}

void wlf_scene_radius_border_set_color(struct wlf_scene_radius_border *border,
		const struct wlf_color *color) {
	if (wlf_color_equal(&border->color, color)) {
		return;
	}

	border->color = *color;
	wlf_scene_node_update(&border->base, NULL);
}

void wlf_scene_radius_border_set_border_width(
		struct wlf_scene_radius_border *border, double border_width) {
	border_width = clamp_border_width(border_width,
		border->base.state.width, border->base.state.height);
	if (border->border_width == border_width) {
		return;
	}

	border->border_width = border_width;
	wlf_scene_node_update(&border->base, NULL);
}

void wlf_scene_radius_border_set_corner_radii(
		struct wlf_scene_radius_border *border, double top_left, double top_right,
		double bottom_right, double bottom_left) {
	assert(top_left >= 0.0);
	assert(top_right >= 0.0);
	assert(bottom_right >= 0.0);
	assert(bottom_left >= 0.0);

	if (border->radius_top_left == top_left &&
			border->radius_top_right == top_right &&
			border->radius_bottom_right == bottom_right &&
			border->radius_bottom_left == bottom_left) {
		return;
	}

	border->radius_top_left = top_left;
	border->radius_top_right = top_right;
	border->radius_bottom_right = bottom_right;
	border->radius_bottom_left = bottom_left;
	wlf_scene_node_update(&border->base, NULL);
}

bool wlf_scene_node_is_radius_border(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_radius_border *wlf_scene_radius_border_from_node(
		struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);
	struct wlf_scene_radius_border *scene_border = NULL;
	return wlf_container_of(node, scene_border, base);
}
