#include "wlf_shape_geometry.h"

#include "wlf/shapes/wlf_rect_shape.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define WLF_PI 3.14159265358979323846
#define AA_WIDTH 1.0

static bool reserve(struct wlf_shape_vertices *vertices, size_t count) {
	if (vertices->failed) return false;
	if (vertices->len + count <= vertices->capacity) return true;
	size_t capacity = vertices->capacity > 0 ? vertices->capacity * 2 : 96;
	while (capacity < vertices->len + count) capacity *= 2;
	void *data = realloc(vertices->data, capacity * sizeof(*vertices->data));
	if (data == NULL) {
		vertices->failed = true;
		return false;
	}
	vertices->data = data;
	vertices->capacity = capacity;
	return true;
}

void wlf_shape_vertices_finish(struct wlf_shape_vertices *vertices) {
	free(vertices->data);
	*vertices = (struct wlf_shape_vertices){0};
}

void wlf_shape_add_triangle(struct wlf_shape_vertices *vertices,
		double ax, double ay, double bx, double by, double cx, double cy) {
	wlf_shape_add_triangle_coverage(vertices,
		ax, ay, 1, bx, by, 1, cx, cy, 1);
}

void wlf_shape_add_triangle_coverage(struct wlf_shape_vertices *vertices,
		double ax, double ay, float ac, double bx, double by, float bc,
		double cx, double cy, float cc) {
	if (!reserve(vertices, 3)) return;
	vertices->data[vertices->len++] = (struct wlf_vector_vertex){ ax, ay, ac };
	vertices->data[vertices->len++] = (struct wlf_vector_vertex){ bx, by, bc };
	vertices->data[vertices->len++] = (struct wlf_vector_vertex){ cx, cy, cc };
}

void wlf_shape_add_quad(struct wlf_shape_vertices *vertices,
		double ax, double ay, double bx, double by,
		double cx, double cy, double dx, double dy) {
	wlf_shape_add_triangle(vertices, ax, ay, bx, by, cx, cy);
	wlf_shape_add_triangle(vertices, ax, ay, cx, cy, dx, dy);
}

void wlf_shape_add_quad_coverage(struct wlf_shape_vertices *vertices,
		double ax, double ay, float ac, double bx, double by, float bc,
		double cx, double cy, float cc, double dx, double dy, float dc) {
	wlf_shape_add_triangle_coverage(vertices,
		ax, ay, ac, bx, by, bc, cx, cy, cc);
	wlf_shape_add_triangle_coverage(vertices,
		ax, ay, ac, cx, cy, cc, dx, dy, dc);
}

void wlf_shape_add_segment(struct wlf_shape_vertices *vertices,
		double x1, double y1, double x2, double y2, double width) {
	double dx = x2 - x1, dy = y2 - y1;
	double length = hypot(dx, dy);
	if (length <= 0 || width <= 0) return;
	double nx = -dy / length * width / 2;
	double ny = dx / length * width / 2;
	wlf_shape_add_quad(vertices, x1 + nx, y1 + ny, x2 + nx, y2 + ny,
		x2 - nx, y2 - ny, x1 - nx, y1 - ny);
	double ax = nx / (width / 2) * AA_WIDTH;
	double ay = ny / (width / 2) * AA_WIDTH;
	double tx = dx / length * AA_WIDTH;
	double ty = dy / length * AA_WIDTH;
	wlf_shape_add_quad_coverage(vertices,
		x1 + nx, y1 + ny, 1, x2 + nx, y2 + ny, 1,
		x2 + nx + ax, y2 + ny + ay, 0,
		x1 + nx + ax, y1 + ny + ay, 0);
	wlf_shape_add_quad_coverage(vertices,
		x1 - nx, y1 - ny, 1, x1 - nx - ax, y1 - ny - ay, 0,
		x2 - nx - ax, y2 - ny - ay, 0, x2 - nx, y2 - ny, 1);
	wlf_shape_add_quad_coverage(vertices,
		x1 - nx, y1 - ny, 1, x1 + nx, y1 + ny, 1,
		x1 + nx - tx, y1 + ny - ty, 0,
		x1 - nx - tx, y1 - ny - ty, 0);
	wlf_shape_add_quad_coverage(vertices,
		x2 + nx, y2 + ny, 1, x2 - nx, y2 - ny, 1,
		x2 - nx + tx, y2 - ny + ty, 0,
		x2 + nx + tx, y2 + ny + ty, 0);
}

static double polygon_area(const float *points, int count) {
	double area = 0;
	for (int i = 0; i < count; i++) {
		int j = (i + 1) % count;
		area += points[i * 2] * points[j * 2 + 1] -
			points[j * 2] * points[i * 2 + 1];
	}
	return area / 2;
}

static bool point_in_triangle(double px, double py,
		double ax, double ay, double bx, double by, double cx, double cy) {
	double ab = (bx - ax) * (py - ay) - (by - ay) * (px - ax);
	double bc = (cx - bx) * (py - by) - (cy - by) * (px - bx);
	double ca = (ax - cx) * (py - cy) - (ay - cy) * (px - cx);
	return (ab >= 0 && bc >= 0 && ca >= 0) ||
		(ab <= 0 && bc <= 0 && ca <= 0);
}

void wlf_shape_add_polygon_fill(struct wlf_shape_vertices *vertices,
		const float *points, int count, double ox, double oy) {
	if (points == NULL || count < 3) return;
	int *indices = malloc((size_t)count * sizeof(*indices));
	if (indices == NULL) {
		vertices->failed = true;
		return;
	}
	bool ccw = polygon_area(points, count) > 0;
	for (int i = 0; i < count; i++) indices[i] = ccw ? i : count - 1 - i;
	int remaining = count;
	int guard = count * count;
	while (remaining > 2 && guard-- > 0) {
		bool clipped = false;
		for (int i = 0; i < remaining; i++) {
			int ia = indices[(i + remaining - 1) % remaining];
			int ib = indices[i];
			int ic = indices[(i + 1) % remaining];
			double ax = points[ia * 2], ay = points[ia * 2 + 1];
			double bx = points[ib * 2], by = points[ib * 2 + 1];
			double cx = points[ic * 2], cy = points[ic * 2 + 1];
			if ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax) <= 0) continue;
			bool contains = false;
			for (int j = 0; j < remaining; j++) {
				int ip = indices[j];
				if (ip != ia && ip != ib && ip != ic &&
						point_in_triangle(points[ip * 2], points[ip * 2 + 1],
							ax, ay, bx, by, cx, cy)) {
					contains = true;
					break;
				}
			}
			if (contains) continue;
			wlf_shape_add_triangle(vertices, ax + ox, ay + oy,
				bx + ox, by + oy, cx + ox, cy + oy);
			memmove(&indices[i], &indices[i + 1],
				(size_t)(remaining - i - 1) * sizeof(*indices));
			remaining--;
			clipped = true;
			break;
		}
		if (!clipped) break;
	}
	free(indices);
}

void wlf_shape_add_polygon_stroke(struct wlf_shape_vertices *vertices,
		const float *points, int count, bool closed,
		double width, double ox, double oy) {
	if (points == NULL || count < 2) return;
	int edges = closed ? count : count - 1;
	for (int i = 0; i < edges; i++) {
		int j = (i + 1) % count;
		wlf_shape_add_segment(vertices, points[i * 2] + ox,
			points[i * 2 + 1] + oy, points[j * 2] + ox,
			points[j * 2 + 1] + oy, width);
	}
}

void wlf_shape_add_polygon_fringe(struct wlf_shape_vertices *vertices,
		const float *points, int count, double ox, double oy, double width) {
	if (points == NULL || count < 3 || width <= 0) return;
	bool ccw = polygon_area(points, count) > 0;
	double *outer = malloc((size_t)count * 2 * sizeof(*outer));
	if (outer == NULL) {
		vertices->failed = true;
		return;
	}
	for (int i = 0; i < count; i++) {
		int prev = (i + count - 1) % count;
		int next = (i + 1) % count;
		double p1x = points[i * 2] - points[prev * 2];
		double p1y = points[i * 2 + 1] - points[prev * 2 + 1];
		double p2x = points[next * 2] - points[i * 2];
		double p2y = points[next * 2 + 1] - points[i * 2 + 1];
		double l1 = hypot(p1x, p1y), l2 = hypot(p2x, p2y);
		if (l1 <= 0 || l2 <= 0) {
			outer[i * 2] = points[i * 2];
			outer[i * 2 + 1] = points[i * 2 + 1];
			continue;
		}
		double sign = ccw ? 1 : -1;
		double n1x = sign * p1y / l1, n1y = -sign * p1x / l1;
		double n2x = sign * p2y / l2, n2y = -sign * p2x / l2;
		double mx = n1x + n2x, my = n1y + n2y;
		double ml = hypot(mx, my);
		if (ml <= 0) { mx = n2x; my = n2y; ml = 1; }
		mx /= ml; my /= ml;
		double denom = mx * n2x + my * n2y;
		double scale = denom > 0.25 ? width / denom : width * 4;
		outer[i * 2] = points[i * 2] + mx * scale;
		outer[i * 2 + 1] = points[i * 2 + 1] + my * scale;
	}
	for (int i = 0; i < count; i++) {
		int next = (i + 1) % count;
		wlf_shape_add_quad_coverage(vertices,
			points[i * 2] + ox, points[i * 2 + 1] + oy, 1,
			points[next * 2] + ox, points[next * 2 + 1] + oy, 1,
			outer[next * 2] + ox, outer[next * 2 + 1] + oy, 0,
			outer[i * 2] + ox, outer[i * 2 + 1] + oy, 0);
	}
	free(outer);
}

int wlf_shape_rounded_rect_points(const struct wlf_rect_shape *rect,
		float *points, int corner_segments) {
	double rx = fmin(fabs(rect->rx), fabs(rect->width) / 2);
	double ry = fmin(fabs(rect->ry), fabs(rect->height) / 2);
	if (rx <= 0 || ry <= 0) {
		float plain[] = { rect->x, rect->y, rect->x + rect->width, rect->y,
			rect->x + rect->width, rect->y + rect->height,
			rect->x, rect->y + rect->height };
		memcpy(points, plain, sizeof(plain));
		return 4;
	}
	double centers[4][2] = {
		{ rect->x + rect->width - rx, rect->y + ry },
		{ rect->x + rect->width - rx, rect->y + rect->height - ry },
		{ rect->x + rx, rect->y + rect->height - ry },
		{ rect->x + rx, rect->y + ry },
	};
	double starts[4] = { -WLF_PI / 2, 0, WLF_PI / 2, WLF_PI };
	int count = 0;
	for (int corner = 0; corner < 4; corner++) {
		for (int i = 0; i <= corner_segments; i++) {
			double angle = starts[corner] +
				(double)i / corner_segments * WLF_PI / 2;
			points[count * 2] = centers[corner][0] + cos(angle) * rx;
			points[count * 2 + 1] = centers[corner][1] + sin(angle) * ry;
			count++;
		}
	}
	return count;
}

void wlf_shape_submit(struct wlf_vector_pass *pass,
		struct wlf_render_target_info *target,
		const struct wlf_shape_vertices *vertices, struct wlf_color color,
		float alpha, const pixman_region32_t *clip,
		enum wlf_render_blend_mode blend_mode) {
	if (vertices->failed || vertices->len == 0) return;
	color.a *= alpha;
	wlf_render_pass_add_triangles(pass, target,
		&(struct wlf_render_vector_options){
			.vertices = vertices->data,
			.vertex_count = vertices->len,
			.color = color,
			.clip = clip,
			.blend_mode = blend_mode,
		});
}
