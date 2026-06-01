#include "wlf/scene/wlf_scene_line.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static double clamp_stroke_width(double stroke_width) {
	return stroke_width < 0.0 ? 0.0 : stroke_width;
}

static void normalize_line_geometry(struct wlf_scene_line *line,
		double x1, double y1, double x2, double y2) {
	double min_x = fmin(x1, x2);
	double min_y = fmin(y1, y2);
	double max_x = fmax(x1, x2);
	double max_y = fmax(y1, y2);

	line->x1 = x1 - min_x;
	line->y1 = y1 - min_y;
	line->x2 = x2 - min_x;
	line->y2 = y2 - min_y;
	line->base.state.width = max_x - min_x;
	line->base.state.height = max_y - min_y;
}

static double point_segment_distance(double px, double py,
		double x1, double y1, double x2, double y2) {
	double dx = x2 - x1;
	double dy = y2 - y1;
	double len_sq = dx * dx + dy * dy;
	if (len_sq == 0.0) {
		double ox = px - x1;
		double oy = py - y1;
		return sqrt(ox * ox + oy * oy);
	}

	double t = ((px - x1) * dx + (py - y1) * dy) / len_sq;
	if (t < 0.0) {
		t = 0.0;
	} else if (t > 1.0) {
		t = 1.0;
	}

	double proj_x = x1 + t * dx;
	double proj_y = y1 + t * dy;
	double ox = px - proj_x;
	double oy = py - proj_y;
	return sqrt(ox * ox + oy * oy);
}

static bool point_on_line(const struct wlf_scene_line *line, double x, double y) {
	double half = clamp_stroke_width(line->stroke_width) / 2.0;
	struct wlf_frect hit_box = {
		.x = -half,
		.y = -half,
		.width = line->base.state.width + line->stroke_width,
		.height = line->base.state.height + line->stroke_width,
	};
	if (!wlf_frect_contains_point(&hit_box, x, y)) {
		return false;
	}

	return point_segment_distance(x, y, line->x1, line->y1, line->x2, line->y2) <= half;
}

static void scene_line_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_line *line = (struct wlf_scene_line *)node;
	free(line);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_line *line = wlf_scene_line_from_node(node);
	*width = line->base.state.width;
	*height = line->base.state.height;
}

static void scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
		double y, struct wlf_region *opaque) {
	(void)node;
	(void)x;
	(void)y;
	wlf_region_clear(opaque);
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_line *line = wlf_scene_line_from_node(node);
	return line->color.a == 0.0 || clamp_stroke_width(line->stroke_width) <= 0.0;
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
	struct wlf_scene_line *line = wlf_scene_line_from_node(node);
	if (!point_on_line(line, lx, ly)) {
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

	double half = clamp_stroke_width(wlf_scene_line_from_node(node)->stroke_width) / 2.0;
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
	.destroy = scene_line_destroy,
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

struct wlf_scene_line *wlf_scene_line_create(struct wlf_scene_tree *parent,
		double x1, double y1, double x2, double y2,
		const struct wlf_color *color, double stroke_width) {
	assert(parent != NULL);
	assert(color != NULL);

	struct wlf_scene_line *line = calloc(1, sizeof(*line));
	if (line == NULL) {
		return NULL;
	}

	wlf_scene_node_init(&line->base, &scene_node_impl, &parent->base);
	line->color = *color;
	line->stroke_width = clamp_stroke_width(stroke_width);
	normalize_line_geometry(line, x1, y1, x2, y2);
	return line;
}

void wlf_scene_line_set_points(struct wlf_scene_line *line,
		double x1, double y1, double x2, double y2) {
	double old_x1 = line->x1;
	double old_y1 = line->y1;
	double old_x2 = line->x2;
	double old_y2 = line->y2;
	double old_width = line->base.state.width;
	double old_height = line->base.state.height;

	normalize_line_geometry(line, x1, y1, x2, y2);
	if (line->x1 == old_x1 && line->y1 == old_y1 &&
			line->x2 == old_x2 && line->y2 == old_y2 &&
			line->base.state.width == old_width &&
			line->base.state.height == old_height) {
		return;
	}

	wlf_scene_node_update(&line->base, NULL);
}

void wlf_scene_line_set_color(struct wlf_scene_line *line,
		const struct wlf_color *color) {
	if (wlf_color_equal(&line->color, color)) {
		return;
	}

	line->color = *color;
	wlf_scene_node_update(&line->base, NULL);
}

void wlf_scene_line_set_stroke_width(struct wlf_scene_line *line,
		double stroke_width) {
	stroke_width = clamp_stroke_width(stroke_width);
	if (line->stroke_width == stroke_width) {
		return;
	}

	line->stroke_width = stroke_width;
	wlf_scene_node_update(&line->base, NULL);
}

bool wlf_scene_node_is_line(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_line *wlf_scene_line_from_node(struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);
	struct wlf_scene_line *line = NULL;
	return wlf_container_of(node, line, base);
}
