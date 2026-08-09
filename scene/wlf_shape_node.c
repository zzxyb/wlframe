#include "wlf/scene/wlf_shape_node.h"

#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CURVE_SEGMENTS 48
#define CORNER_SEGMENTS 8
#define WLF_PI 3.14159265358979323846

struct vertex_buffer {
	struct wlf_vector_vertex *data;
	size_t len, capacity;
};

static bool vertices_reserve(struct vertex_buffer *buffer, size_t count) {
	if (buffer->len + count <= buffer->capacity) {
		return true;
	}
	size_t capacity = buffer->capacity > 0 ? buffer->capacity * 2 : 96;
	while (capacity < buffer->len + count) {
		capacity *= 2;
	}
	void *data = realloc(buffer->data, capacity * sizeof(*buffer->data));
	if (data == NULL) {
		return false;
	}
	buffer->data = data;
	buffer->capacity = capacity;
	return true;
}

static void add_triangle(struct vertex_buffer *buffer,
		double ax, double ay, double bx, double by, double cx, double cy) {
	if (!vertices_reserve(buffer, 3)) {
		return;
	}
	buffer->data[buffer->len++] = (struct wlf_vector_vertex){ ax, ay };
	buffer->data[buffer->len++] = (struct wlf_vector_vertex){ bx, by };
	buffer->data[buffer->len++] = (struct wlf_vector_vertex){ cx, cy };
}

static void add_quad(struct vertex_buffer *buffer,
		double ax, double ay, double bx, double by,
		double cx, double cy, double dx, double dy) {
	add_triangle(buffer, ax, ay, bx, by, cx, cy);
	add_triangle(buffer, ax, ay, cx, cy, dx, dy);
}

static void add_segment(struct vertex_buffer *buffer,
		double x1, double y1, double x2, double y2, double width) {
	double dx = x2 - x1, dy = y2 - y1;
	double length = hypot(dx, dy);
	if (length <= 0 || width <= 0) {
		return;
	}
	double nx = -dy / length * width / 2;
	double ny = dx / length * width / 2;
	add_quad(buffer, x1 + nx, y1 + ny, x2 + nx, y2 + ny,
		x2 - nx, y2 - ny, x1 - nx, y1 - ny);
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

static void add_polygon_fill(struct vertex_buffer *buffer,
		const float *points, int count, double ox, double oy) {
	if (count < 3) {
		return;
	}
	int *indices = malloc((size_t)count * sizeof(*indices));
	if (indices == NULL) {
		return;
	}
	bool ccw = polygon_area(points, count) > 0;
	for (int i = 0; i < count; i++) {
		indices[i] = ccw ? i : count - 1 - i;
	}

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
			if ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax) <= 0) {
				continue;
			}
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
			if (contains) {
				continue;
			}
			add_triangle(buffer, ax + ox, ay + oy, bx + ox, by + oy,
				cx + ox, cy + oy);
			memmove(&indices[i], &indices[i + 1],
				(size_t)(remaining - i - 1) * sizeof(*indices));
			remaining--;
			clipped = true;
			break;
		}
		if (!clipped) {
			break;
		}
	}
	free(indices);
}

static void add_polygon_stroke(struct vertex_buffer *buffer,
		const float *points, int count, bool closed,
		double width, double ox, double oy) {
	int edges = closed ? count : count - 1;
	for (int i = 0; i < edges; i++) {
		int j = (i + 1) % count;
		add_segment(buffer, points[i * 2] + ox, points[i * 2 + 1] + oy,
			points[j * 2] + ox, points[j * 2 + 1] + oy, width);
	}
}

static int rounded_rect_points(const struct wlf_rect_shape *rect,
		float *points) {
	double rx = fmin(fabs(rect->rx), fabs(rect->width) / 2);
	double ry = fmin(fabs(rect->ry), fabs(rect->height) / 2);
	if (rx <= 0 || ry <= 0) {
		float plain[] = {
			rect->x, rect->y,
			rect->x + rect->width, rect->y,
			rect->x + rect->width, rect->y + rect->height,
			rect->x, rect->y + rect->height,
		};
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
		for (int i = 0; i <= CORNER_SEGMENTS; i++) {
			double angle = starts[corner] +
				(double)i / CORNER_SEGMENTS * WLF_PI / 2;
			points[count * 2] = centers[corner][0] + cos(angle) * rx;
			points[count * 2 + 1] = centers[corner][1] + sin(angle) * ry;
			count++;
		}
	}
	return count;
}

static const struct wlf_shape_state *get_state(struct wlf_shape_node *node) {
	switch (node->type) {
	case WLF_SHAPE_NODE_RECT:
		return &wlf_rect_shape_from_shape(node->shape)->state;
	case WLF_SHAPE_NODE_CIRCLE:
		return &wlf_circle_shape_from_shape(node->shape)->state;
	case WLF_SHAPE_NODE_ELLIPSE:
		return &wlf_ellipse_shape_from_shape(node->shape)->state;
	case WLF_SHAPE_NODE_LINE:
		return &wlf_line_shape_from_shape(node->shape)->state;
	case WLF_SHAPE_NODE_POLY:
		return &wlf_poly_shape_from_shape(node->shape)->state;
	case WLF_SHAPE_NODE_PATH:
		return &wlf_path_shape_from_shape(node->shape)->state;
	}
	return NULL;
}

static bool shape_bounds(struct wlf_shape_node *node,
		double *minx, double *miny, double *maxx, double *maxy) {
	*minx = *miny = INFINITY;
	*maxx = *maxy = -INFINITY;
#define ADD_POINT(px, py) do { \
	*minx = fmin(*minx, (px)); *miny = fmin(*miny, (py)); \
	*maxx = fmax(*maxx, (px)); *maxy = fmax(*maxy, (py)); \
} while (0)
	switch (node->type) {
	case WLF_SHAPE_NODE_RECT: {
		struct wlf_rect_shape *s = wlf_rect_shape_from_shape(node->shape);
		ADD_POINT(s->x, s->y); ADD_POINT(s->x + s->width, s->y + s->height);
		break;
	}
	case WLF_SHAPE_NODE_CIRCLE: {
		struct wlf_circle_shape *s = wlf_circle_shape_from_shape(node->shape);
		ADD_POINT(s->cx - s->r, s->cy - s->r); ADD_POINT(s->cx + s->r, s->cy + s->r);
		break;
	}
	case WLF_SHAPE_NODE_ELLIPSE: {
		struct wlf_ellipse_shape *s = wlf_ellipse_shape_from_shape(node->shape);
		ADD_POINT(s->cx - s->rx, s->cy - s->ry); ADD_POINT(s->cx + s->rx, s->cy + s->ry);
		break;
	}
	case WLF_SHAPE_NODE_LINE: {
		struct wlf_line_shape *s = wlf_line_shape_from_shape(node->shape);
		ADD_POINT(s->x1, s->y1); ADD_POINT(s->x2, s->y2);
		break;
	}
	case WLF_SHAPE_NODE_POLY: {
		struct wlf_poly_shape *s = wlf_poly_shape_from_shape(node->shape);
		for (int i = 0; i < s->count; i++) ADD_POINT(s->points[i * 2], s->points[i * 2 + 1]);
		break;
	}
	case WLF_SHAPE_NODE_PATH: {
		struct wlf_path_shape *s = wlf_path_shape_from_shape(node->shape);
		for (struct wlf_path *path = s->paths; path != NULL; path = path->next)
			for (int i = 0; i < path->npts; i++) ADD_POINT(path->pts[i * 2], path->pts[i * 2 + 1]);
		break;
	}
	}
#undef ADD_POINT
	if (!isfinite(*minx)) {
		return false;
	}
	const struct wlf_shape_state *state = get_state(node);
	if (state != NULL && state->has_stroke && state->stroke_width > 0) {
		double pad = state->stroke_width / 2;
		*minx -= pad; *miny -= pad; *maxx += pad; *maxy += pad;
	}
	return *maxx > *minx && *maxy > *miny;
}

static bool shape_node_invisible(struct wlf_scene_node *base) {
	return !base->state.enabled || base->state.opacity <= 0 ||
		base->state.width <= 0 || base->state.height <= 0;
}

static void shape_node_get_size(struct wlf_scene_node *base,
		double *width, double *height) {
	*width = base->state.width;
	*height = base->state.height;
}

static void shape_node_visibility(struct wlf_scene_node *base,
		pixman_region32_t *visible) {
	if (shape_node_invisible(base)) return;
	double x, y;
	if (wlf_scene_node_coords(base, &x, &y))
		pixman_region32_union_rect(visible, visible, x, y,
			base->state.width, base->state.height);
}

static struct wlf_scene_node *shape_node_at(struct wlf_scene_node *base,
		double lx, double ly, double *nx, double *ny) {
	if (shape_node_invisible(base) || lx < 0 || ly < 0 ||
			lx >= base->state.width || ly >= base->state.height) return NULL;
	if (nx) *nx = lx;
	if (ny) *ny = ly;
	return base;
}

static void shape_node_bounds_cb(struct wlf_scene_node *base,
		double x, double y, pixman_region32_t *visible) {
	if (!shape_node_invisible(base))
		pixman_region32_union_rect(visible, visible, x, y,
			base->state.width, base->state.height);
}

static bool shape_node_in_box(struct wlf_scene_node *base,
		struct wlf_frect *box, scene_node_box_iterator_func_t iterator, void *data) {
	if (shape_node_invisible(base)) return false;
	double x, y;
	if (!wlf_scene_node_coords(base, &x, &y)) return false;
	if (x >= box->x + box->width || x + base->state.width <= box->x ||
			y >= box->y + box->height || y + base->state.height <= box->y) return false;
	return iterator(base, x, y, data);
}

static void shape_node_destroy(struct wlf_scene_node *base) {
	struct wlf_shape_node *node = wlf_shape_node_from_node(base);
	wlf_shape_destroy(node->shape);
	free(node);
}

static const struct wlf_scene_node_impl shape_node_impl = {
	.destroy = shape_node_destroy,
	.get_size = shape_node_get_size,
	.invisible = shape_node_invisible,
	.visibility = shape_node_visibility,
	.at = shape_node_at,
	.bounds = shape_node_bounds_cb,
	.in_box = shape_node_in_box,
};

static struct wlf_shape_node *shape_node_create(struct wlf_scene_node *parent,
		double x, double y, struct wlf_shape *shape,
		enum wlf_shape_node_type type, size_t node_size) {
	if (parent == NULL || shape == NULL) return NULL;
	struct wlf_shape_node *node = calloc(1, node_size);
	if (node == NULL) return NULL;
	node->shape = shape;
	node->type = type;
	double minx, miny, maxx, maxy;
	if (!shape_bounds(node, &minx, &miny, &maxx, &maxy)) {
		free(node);
		return NULL;
	}
	wlf_scene_node_init(&node->base, &shape_node_impl, parent);
	node->base.state.x = x;
	node->base.state.y = y;
	node->base.state.width = ceil(maxx - minx);
	node->base.state.height = ceil(maxy - miny);
	node->geometry_x = minx;
	node->geometry_y = miny;
	return node;
}

static bool shape_node_refresh_bounds(struct wlf_shape_node *node) {
	double minx, miny, maxx, maxy;
	if (!shape_bounds(node, &minx, &miny, &maxx, &maxy)) {
		return false;
	}
	node->geometry_x = minx;
	node->geometry_y = miny;
	node->base.state.width = ceil(maxx - minx);
	node->base.state.height = ceil(maxy - miny);
	return true;
}

#define CREATE_NODE(name, node_type, shape_type, enum_type) \
struct node_type *name(struct wlf_scene_node *parent, double x, double y, \
		struct shape_type *shape) { \
	if (shape == NULL) return NULL; \
	return (struct node_type *)shape_node_create(parent, x, y, &shape->base, \
		enum_type, sizeof(struct node_type)); \
}
CREATE_NODE(wlf_rect_shape_node_create, wlf_rect_shape_node,
	wlf_rect_shape, WLF_SHAPE_NODE_RECT)
CREATE_NODE(wlf_circle_node_create, wlf_circle_node,
	wlf_circle_shape, WLF_SHAPE_NODE_CIRCLE)
CREATE_NODE(wlf_ellipse_node_create, wlf_ellipse_node,
	wlf_ellipse_shape, WLF_SHAPE_NODE_ELLIPSE)
CREATE_NODE(wlf_line_node_create, wlf_line_node,
	wlf_line_shape, WLF_SHAPE_NODE_LINE)
CREATE_NODE(wlf_poly_node_create, wlf_poly_node,
	wlf_poly_shape, WLF_SHAPE_NODE_POLY)
CREATE_NODE(wlf_path_node_create, wlf_path_node,
	wlf_path_shape, WLF_SHAPE_NODE_PATH)
#undef CREATE_NODE

bool wlf_scene_node_is_shape(const struct wlf_scene_node *node) {
	return node != NULL && node->impl == &shape_node_impl;
}

struct wlf_shape_node *wlf_shape_node_from_node(struct wlf_scene_node *node) {
	assert(wlf_scene_node_is_shape(node));
	struct wlf_shape_node *shape_node = wlf_container_of(node, shape_node, base);
	return shape_node;
}

#define DEFINE_NODE_CASTS(kind, node_type, enum_type) \
bool wlf_scene_node_is_##kind(const struct wlf_scene_node *node) { \
	return wlf_scene_node_is_shape(node) && \
		((const struct wlf_shape_node *)node)->type == enum_type; \
} \
struct node_type *wlf_##kind##_node_from_node(struct wlf_scene_node *node) { \
	assert(wlf_scene_node_is_##kind(node)); \
	return (struct node_type *)wlf_shape_node_from_node(node); \
}

DEFINE_NODE_CASTS(rect_shape, wlf_rect_shape_node, WLF_SHAPE_NODE_RECT)
DEFINE_NODE_CASTS(circle, wlf_circle_node, WLF_SHAPE_NODE_CIRCLE)
DEFINE_NODE_CASTS(ellipse, wlf_ellipse_node, WLF_SHAPE_NODE_ELLIPSE)
DEFINE_NODE_CASTS(line, wlf_line_node, WLF_SHAPE_NODE_LINE)
DEFINE_NODE_CASTS(poly, wlf_poly_node, WLF_SHAPE_NODE_POLY)
DEFINE_NODE_CASTS(path, wlf_path_node, WLF_SHAPE_NODE_PATH)
#undef DEFINE_NODE_CASTS

static void build_fill(struct wlf_shape_node *node, struct vertex_buffer *vertices,
		double ox, double oy) {
	switch (node->type) {
	case WLF_SHAPE_NODE_RECT: {
		struct wlf_rect_shape *s = wlf_rect_shape_from_shape(node->shape);
		float points[4 * (CORNER_SEGMENTS + 1) * 2];
		int count = rounded_rect_points(s, points);
		add_polygon_fill(vertices, points, count, ox, oy);
		break;
	}
	case WLF_SHAPE_NODE_CIRCLE:
	case WLF_SHAPE_NODE_ELLIPSE: {
		double cx, cy, rx, ry;
		if (node->type == WLF_SHAPE_NODE_CIRCLE) {
			struct wlf_circle_shape *s = wlf_circle_shape_from_shape(node->shape);
			cx = s->cx; cy = s->cy; rx = ry = s->r;
		} else {
			struct wlf_ellipse_shape *s = wlf_ellipse_shape_from_shape(node->shape);
			cx = s->cx; cy = s->cy; rx = s->rx; ry = s->ry;
		}
		for (int i = 0; i < CURVE_SEGMENTS; i++) {
			double a = i * 2 * WLF_PI / CURVE_SEGMENTS;
			double b = (i + 1) * 2 * WLF_PI / CURVE_SEGMENTS;
			add_triangle(vertices, cx + ox, cy + oy,
				cx + cos(a) * rx + ox, cy + sin(a) * ry + oy,
				cx + cos(b) * rx + ox, cy + sin(b) * ry + oy);
		}
		break;
	}
	case WLF_SHAPE_NODE_POLY: {
		struct wlf_poly_shape *s = wlf_poly_shape_from_shape(node->shape);
		if (s->closed) add_polygon_fill(vertices, s->points, s->count, ox, oy);
		break;
	}
	case WLF_SHAPE_NODE_PATH: {
		struct wlf_path_shape *s = wlf_path_shape_from_shape(node->shape);
		for (struct wlf_path *p = s->paths; p; p = p->next)
			if (p->closed) add_polygon_fill(vertices, p->pts, p->npts, ox, oy);
		break;
	}
	case WLF_SHAPE_NODE_LINE:
		break;
	}
}

static void build_stroke(struct wlf_shape_node *node, struct vertex_buffer *vertices,
		double width, double ox, double oy) {
	switch (node->type) {
	case WLF_SHAPE_NODE_RECT: {
		struct wlf_rect_shape *s = wlf_rect_shape_from_shape(node->shape);
		float points[4 * (CORNER_SEGMENTS + 1) * 2];
		int count = rounded_rect_points(s, points);
		add_polygon_stroke(vertices, points, count, true, width, ox, oy);
		break;
	}
	case WLF_SHAPE_NODE_CIRCLE:
	case WLF_SHAPE_NODE_ELLIPSE: {
		double cx, cy, rx, ry;
		if (node->type == WLF_SHAPE_NODE_CIRCLE) {
			struct wlf_circle_shape *s = wlf_circle_shape_from_shape(node->shape);
			cx = s->cx; cy = s->cy; rx = ry = s->r;
		} else {
			struct wlf_ellipse_shape *s = wlf_ellipse_shape_from_shape(node->shape);
			cx = s->cx; cy = s->cy; rx = s->rx; ry = s->ry;
		}
		for (int i = 0; i < CURVE_SEGMENTS; i++) {
			double a = i * 2 * WLF_PI / CURVE_SEGMENTS;
			double b = (i + 1) * 2 * WLF_PI / CURVE_SEGMENTS;
			double orx = rx + width / 2, ory = ry + width / 2;
			double irx = fmax(0, rx - width / 2), iry = fmax(0, ry - width / 2);
			add_quad(vertices, cx + cos(a) * orx + ox, cy + sin(a) * ory + oy,
				cx + cos(b) * orx + ox, cy + sin(b) * ory + oy,
				cx + cos(b) * irx + ox, cy + sin(b) * iry + oy,
				cx + cos(a) * irx + ox, cy + sin(a) * iry + oy);
		}
		break;
	}
	case WLF_SHAPE_NODE_LINE: {
		struct wlf_line_shape *s = wlf_line_shape_from_shape(node->shape);
		add_segment(vertices, s->x1 + ox, s->y1 + oy, s->x2 + ox, s->y2 + oy, width);
		break;
	}
	case WLF_SHAPE_NODE_POLY: {
		struct wlf_poly_shape *s = wlf_poly_shape_from_shape(node->shape);
		add_polygon_stroke(vertices, s->points, s->count, s->closed, width, ox, oy);
		break;
	}
	case WLF_SHAPE_NODE_PATH: {
		struct wlf_path_shape *s = wlf_path_shape_from_shape(node->shape);
		for (struct wlf_path *p = s->paths; p; p = p->next)
			add_polygon_stroke(vertices, p->pts, p->npts, p->closed, width, ox, oy);
		break;
	}
	}
}

void wlf_shape_node_render(struct wlf_shape_node *node,
		struct wlf_vector_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const pixman_region32_t *clip) {
	if (node == NULL || pass == NULL || !shape_node_refresh_bounds(node) ||
			shape_node_invisible(&node->base)) return;
	double x, y;
	if (!wlf_scene_node_coords(&node->base, &x, &y)) return;
	double ox = x - node->geometry_x;
	double oy = y - node->geometry_y;
	const struct wlf_shape_state *state = get_state(node);
	struct vertex_buffer vertices = {0};
	if (state->has_fill) {
		build_fill(node, &vertices, ox, oy);
		struct wlf_color color = state->fill_color;
		color.a *= wlf_shape_state_fill_alpha(state) * node->base.state.opacity;
		if (vertices.len > 0) wlf_render_pass_add_triangles(pass, render_target_info,
			&(struct wlf_render_vector_options){ .vertices = vertices.data,
				.vertex_count = vertices.len, .color = color, .clip = clip,
				.blend_mode = WLF_RENDER_BLEND_MODE_PREMULTIPLIED });
	}
	vertices.len = 0;
	if (state->has_stroke && state->stroke_width > 0) {
		build_stroke(node, &vertices, state->stroke_width, ox, oy);
		struct wlf_color color = state->stroke_color;
		color.a *= wlf_shape_state_stroke_alpha(state) * node->base.state.opacity;
		if (vertices.len > 0) wlf_render_pass_add_triangles(pass, render_target_info,
			&(struct wlf_render_vector_options){ .vertices = vertices.data,
				.vertex_count = vertices.len, .color = color, .clip = clip,
				.blend_mode = WLF_RENDER_BLEND_MODE_PREMULTIPLIED });
	}
	free(vertices.data);
}

#define DEFINE_NODE_RENDER(name, node_type) \
void name(struct node_type *node, struct wlf_vector_pass *pass, \
		struct wlf_render_target_info *render_target_info, \
		const pixman_region32_t *clip) { \
	wlf_shape_node_render(node != NULL ? &node->shape_node : NULL, pass, \
		render_target_info, clip); \
}

DEFINE_NODE_RENDER(wlf_rect_shape_node_render, wlf_rect_shape_node)
DEFINE_NODE_RENDER(wlf_circle_node_render, wlf_circle_node)
DEFINE_NODE_RENDER(wlf_ellipse_node_render, wlf_ellipse_node)
DEFINE_NODE_RENDER(wlf_line_node_render, wlf_line_node)
DEFINE_NODE_RENDER(wlf_poly_node_render, wlf_poly_node)
DEFINE_NODE_RENDER(wlf_path_node_render, wlf_path_node)
#undef DEFINE_NODE_RENDER
