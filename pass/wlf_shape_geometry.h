#ifndef WLF_SHAPE_GEOMETRY_H
#define WLF_SHAPE_GEOMETRY_H

#include "wlf/pass/wlf_vector_pass.h"
#include "wlf/shapes/wlf_shape.h"

struct wlf_rect_shape;

struct wlf_shape_vertices {
	struct wlf_vector_vertex *data;
	size_t len;
	size_t capacity;
	bool failed;
};

void wlf_shape_vertices_finish(struct wlf_shape_vertices *vertices);
void wlf_shape_add_triangle(struct wlf_shape_vertices *vertices,
	double ax, double ay, double bx, double by, double cx, double cy);
void wlf_shape_add_quad(struct wlf_shape_vertices *vertices,
	double ax, double ay, double bx, double by,
	double cx, double cy, double dx, double dy);
void wlf_shape_add_segment(struct wlf_shape_vertices *vertices,
	double x1, double y1, double x2, double y2, double width);
void wlf_shape_add_polygon_fill(struct wlf_shape_vertices *vertices,
	const float *points, int count, double ox, double oy);
void wlf_shape_add_polygon_stroke(struct wlf_shape_vertices *vertices,
	const float *points, int count, bool closed,
	double width, double ox, double oy);
int wlf_shape_rounded_rect_points(const struct wlf_rect_shape *rect,
	float *points, int corner_segments);
void wlf_shape_submit(struct wlf_vector_pass *pass,
	struct wlf_render_target_info *target,
	const struct wlf_shape_vertices *vertices, struct wlf_color color,
	float alpha, const pixman_region32_t *clip,
	enum wlf_render_blend_mode blend_mode);

#endif
