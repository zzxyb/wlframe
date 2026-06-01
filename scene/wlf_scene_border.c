#include "wlf/scene/wlf_scene_border.h"

#include <assert.h>
#include <stdlib.h>

static double clamp_border_width(double border_width, double width, double height) {
	double max_border_width = width < height ? width : height;
	if (max_border_width < 0.0) {
		max_border_width = 0.0;
	}
	if (border_width < 0.0) {
		return 0.0;
	}
	return border_width > max_border_width ? max_border_width : border_width;
}

static bool point_in_border(const struct wlf_scene_border *border,
		double x, double y) {
	double width = border->base.state.width;
	double height = border->base.state.height;
	double border_width = clamp_border_width(border->border_width, width, height);
	if (x < 0.0 || y < 0.0 || x > width || y > height || border_width <= 0.0) {
		return false;
	}

	return x < border_width || x > width - border_width ||
		y < border_width || y > height - border_width;
}

static void scene_border_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_border *border = (struct wlf_scene_border *)node;
	free(border);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_border *scene_border = wlf_scene_border_from_node(node);
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
	struct wlf_scene_border *scene_border = wlf_scene_border_from_node(node);
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
	struct wlf_scene_border *scene_border = wlf_scene_border_from_node(node);
	if (!point_in_border(scene_border, lx, ly)) {
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
	.destroy = scene_border_destroy,
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

struct wlf_scene_border *wlf_scene_border_create(struct wlf_scene_tree *parent,
		double width, double height, const struct wlf_color *color,
		double border_width) {
	assert(parent != NULL);
	assert(color != NULL);
	assert(width >= 0.0 && height >= 0.0);

	struct wlf_scene_border *border = calloc(1, sizeof(*border));
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

void wlf_scene_border_set_size(struct wlf_scene_border *border,
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

void wlf_scene_border_set_color(struct wlf_scene_border *border,
		const struct wlf_color *color) {
	if (wlf_color_equal(&border->color, color)) {
		return;
	}

	border->color = *color;
	wlf_scene_node_update(&border->base, NULL);
}

void wlf_scene_border_set_border_width(struct wlf_scene_border *border,
		double border_width) {
	border_width = clamp_border_width(border_width,
		border->base.state.width, border->base.state.height);
	if (border->border_width == border_width) {
		return;
	}

	border->border_width = border_width;
	wlf_scene_node_update(&border->base, NULL);
}

bool wlf_scene_node_is_border(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_border *wlf_scene_border_from_node(struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);
	struct wlf_scene_border *scene_border = NULL;
	return wlf_container_of(node, scene_border, base);
}
