#include "wlf/scene/wlf_scene_bezier_curve.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

#define WLF_BEZIER_SEGMENTS 32

static double clamp_stroke_width(double stroke_width) {
	return stroke_width < 0.0 ? 0.0 : stroke_width;
}

static struct wlf_fpoint bezier_point(const struct wlf_scene_bezier_curve *curve,
		double t) {
	double mt = 1.0 - t;
	double mt2 = mt * mt;
	double mt3 = mt2 * mt;
	double t2 = t * t;
	double t3 = t2 * t;

	return (struct wlf_fpoint){
		.x = mt3 * curve->p0.x + 3.0 * mt2 * t * curve->p1.x +
			3.0 * mt * t2 * curve->p2.x + t3 * curve->p3.x,
		.y = mt3 * curve->p0.y + 3.0 * mt2 * t * curve->p1.y +
			3.0 * mt * t2 * curve->p2.y + t3 * curve->p3.y,
	};
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

static void normalize_curve_geometry(struct wlf_scene_bezier_curve *curve,
		const struct wlf_fpoint *p0, const struct wlf_fpoint *p1,
		const struct wlf_fpoint *p2, const struct wlf_fpoint *p3) {
	double min_x = fmin(fmin(p0->x, p1->x), fmin(p2->x, p3->x));
	double min_y = fmin(fmin(p0->y, p1->y), fmin(p2->y, p3->y));
	double max_x = fmax(fmax(p0->x, p1->x), fmax(p2->x, p3->x));
	double max_y = fmax(fmax(p0->y, p1->y), fmax(p2->y, p3->y));

	curve->p0 = (struct wlf_fpoint){p0->x - min_x, p0->y - min_y};
	curve->p1 = (struct wlf_fpoint){p1->x - min_x, p1->y - min_y};
	curve->p2 = (struct wlf_fpoint){p2->x - min_x, p2->y - min_y};
	curve->p3 = (struct wlf_fpoint){p3->x - min_x, p3->y - min_y};
	curve->base.state.width = max_x - min_x;
	curve->base.state.height = max_y - min_y;
}

static bool point_on_curve(const struct wlf_scene_bezier_curve *curve,
		double x, double y) {
	double half = clamp_stroke_width(curve->stroke_width) / 2.0;
	struct wlf_frect hit_box = {
		.x = -half,
		.y = -half,
		.width = curve->base.state.width + curve->stroke_width,
		.height = curve->base.state.height + curve->stroke_width,
	};
	if (!wlf_frect_contains_point(&hit_box, x, y)) {
		return false;
	}

	struct wlf_fpoint prev = curve->p0;
	for (int i = 1; i <= WLF_BEZIER_SEGMENTS; ++i) {
		double t = (double)i / (double)WLF_BEZIER_SEGMENTS;
		struct wlf_fpoint cur = bezier_point(curve, t);
		if (point_segment_distance(x, y, prev.x, prev.y, cur.x, cur.y) <= half) {
			return true;
		}
		prev = cur;
	}
	return false;
}

static void scene_bezier_curve_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_bezier_curve *curve = (struct wlf_scene_bezier_curve *)node;
	free(curve);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_bezier_curve *curve =
		wlf_scene_bezier_curve_from_node(node);
	*width = curve->base.state.width;
	*height = curve->base.state.height;
}

static void scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
		double y, struct wlf_region *opaque) {
	(void)node;
	(void)x;
	(void)y;
	wlf_region_clear(opaque);
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_bezier_curve *curve =
		wlf_scene_bezier_curve_from_node(node);
	return curve->color.a == 0.0 || clamp_stroke_width(curve->stroke_width) <= 0.0;
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
	struct wlf_scene_bezier_curve *curve =
		wlf_scene_bezier_curve_from_node(node);
	if (!point_on_curve(curve, lx, ly)) {
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

	double half =
		clamp_stroke_width(wlf_scene_bezier_curve_from_node(node)->stroke_width) / 2.0;
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
	.destroy = scene_bezier_curve_destroy,
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

struct wlf_scene_bezier_curve *wlf_scene_bezier_curve_create(
		struct wlf_scene_tree *parent, const struct wlf_fpoint *p0,
		const struct wlf_fpoint *p1, const struct wlf_fpoint *p2,
		const struct wlf_fpoint *p3, const struct wlf_color *color,
		double stroke_width) {
	assert(parent != NULL);
	assert(p0 != NULL);
	assert(p1 != NULL);
	assert(p2 != NULL);
	assert(p3 != NULL);
	assert(color != NULL);

	struct wlf_scene_bezier_curve *curve = calloc(1, sizeof(*curve));
	if (curve == NULL) {
		return NULL;
	}

	wlf_scene_node_init(&curve->base, &scene_node_impl, &parent->base);
	curve->color = *color;
	curve->stroke_width = clamp_stroke_width(stroke_width);
	normalize_curve_geometry(curve, p0, p1, p2, p3);
	return curve;
}

void wlf_scene_bezier_curve_set_points(struct wlf_scene_bezier_curve *curve,
		const struct wlf_fpoint *p0, const struct wlf_fpoint *p1,
		const struct wlf_fpoint *p2, const struct wlf_fpoint *p3) {
	assert(p0 != NULL);
	assert(p1 != NULL);
	assert(p2 != NULL);
	assert(p3 != NULL);

	struct wlf_scene_bezier_curve next = *curve;
	normalize_curve_geometry(&next, p0, p1, p2, p3);
	if (wlf_fpoint_equal(&curve->p0, &next.p0) &&
			wlf_fpoint_equal(&curve->p1, &next.p1) &&
			wlf_fpoint_equal(&curve->p2, &next.p2) &&
			wlf_fpoint_equal(&curve->p3, &next.p3) &&
			curve->base.state.width == next.base.state.width &&
			curve->base.state.height == next.base.state.height) {
		return;
	}

	normalize_curve_geometry(curve, p0, p1, p2, p3);
	wlf_scene_node_update(&curve->base, NULL);
}

void wlf_scene_bezier_curve_set_color(struct wlf_scene_bezier_curve *curve,
		const struct wlf_color *color) {
	if (wlf_color_equal(&curve->color, color)) {
		return;
	}

	curve->color = *color;
	wlf_scene_node_update(&curve->base, NULL);
}

void wlf_scene_bezier_curve_set_stroke_width(
		struct wlf_scene_bezier_curve *curve, double stroke_width) {
	stroke_width = clamp_stroke_width(stroke_width);
	if (curve->stroke_width == stroke_width) {
		return;
	}

	curve->stroke_width = stroke_width;
	wlf_scene_node_update(&curve->base, NULL);
}

bool wlf_scene_node_is_bezier_curve(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_bezier_curve *wlf_scene_bezier_curve_from_node(
		struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);
	struct wlf_scene_bezier_curve *curve = NULL;
	return wlf_container_of(node, curve, base);
}
