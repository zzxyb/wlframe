#include "wlf/math/wlf_rect.h"
#include "wlf/math/wlf_point.h"
#include "wlf/math/wlf_size.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct wlf_rect wlf_rect_make(int x, int y, int width, int height) {
	struct wlf_rect rect = {
		.x = x,
		.y = y,
		.width = width,
		.height = height,
	};

	return rect;
}

char* wlf_rect_to_str(const struct wlf_rect *rect) {
	if (rect == NULL) {
		return strdup("Rect(NULL)");
	}

	char *buffer = malloc(64);
	if (buffer) {
		snprintf(buffer, 64, "Rect(%d, %d, %d, %d)", rect->x, rect->y, rect->width, rect->height);
	}
	return buffer;
}

struct wlf_rect wlf_rect_from_size(const struct wlf_point *pos, const struct wlf_size *size) {
	return wlf_rect_make(pos->x, pos->y, size->width, size->height);
}

struct wlf_rect wlf_rect_from_points(const struct wlf_point *p1, const struct wlf_point *p2) {
	int x = fmin(p1->x, p2->x);
	int y = fmin(p1->y, p2->y);
	int width = abs(p1->x - p2->x);
	int height = abs(p1->y - p2->y);
	return wlf_rect_make(x, y, width, height);
}

bool wlf_rect_equal(const struct wlf_rect *a, const struct wlf_rect *b) {
	return (a->x == b->x) && (a->y == b->y) && (a->width == b->width) && (a->height == b->height);
}

bool wlf_rect_is_empty(const struct wlf_rect *rect) {
	return (rect->width == 0) || (rect->height == 0);
}

bool wlf_rect_is_valid(const struct wlf_rect *rect) {
	return (rect->width > 0) && (rect->height > 0);
}

struct wlf_point wlf_rect_get_position(const struct wlf_rect *rect) {
	return wlf_point_make(rect->x, rect->y);
}

struct wlf_size wlf_rect_get_size(const struct wlf_rect *rect) {
	return wlf_size_make(rect->width, rect->height);
}

struct wlf_point wlf_rect_get_center(const struct wlf_rect *rect) {
	return wlf_point_make(rect->x + rect->width / 2, rect->y + rect->height / 2);
}

struct wlf_point wlf_rect_get_top_left(const struct wlf_rect *rect) {
	return wlf_point_make(rect->x, rect->y);
}

struct wlf_point wlf_rect_get_bottom_right(const struct wlf_rect *rect) {
	return wlf_point_make(rect->x + rect->width, rect->y + rect->height);
}

int wlf_rect_area(const struct wlf_rect *rect) {
	return rect->width * rect->height;
}

int wlf_rect_perimeter(const struct wlf_rect *rect) {
	return 2 * (rect->width + rect->height);
}

struct wlf_rect wlf_rect_offset(const struct wlf_rect *rect, const struct wlf_point *offset) {
	return wlf_rect_make(rect->x + offset->x, rect->y + offset->y, rect->width, rect->height);
}

struct wlf_rect wlf_rect_inflate(const struct wlf_rect *rect, int dx, int dy) {
	return wlf_rect_make(rect->x - dx, rect->y - dy, rect->width + 2 * dx, rect->height + 2 * dy);
}

struct wlf_rect wlf_rect_scale(const struct wlf_rect *rect, double sx, double sy) {
	return wlf_rect_make(rect->x, rect->y, (int)(rect->width * sx), (int)(rect->height * sy));
}

bool wlf_rect_contains_point_d(const struct wlf_rect *rect, double x, double y) {
	return (x >= rect->x) && (x <= rect->x + rect->width) &&
			(y >= rect->y) && (y <= rect->y + rect->height);
}

bool wlf_rect_contains_point(const struct wlf_rect *rect, const struct wlf_point *point) {
	return (point->x >= rect->x) && (point->x <= rect->x + rect->width) &&
			(point->y >= rect->y) && (point->y <= rect->y + rect->height);
}

bool wlf_rect_contains_rect(const struct wlf_rect *outer, const struct wlf_rect *inner) {
	struct wlf_point top_left = wlf_rect_get_top_left(inner);
	struct wlf_point bottom_right = wlf_rect_get_bottom_right(inner);

	return wlf_rect_contains_point(outer, &top_left) &&
			wlf_rect_contains_point(outer, &bottom_right);
}

bool wlf_rect_intersects(const struct wlf_rect *a, const struct wlf_rect *b) {
	return !(a->x + a->width < b->x || a->x > b->x + b->width ||
			a->y + a->height < b->y || a->y > b->y + b->height);
}

struct wlf_rect wlf_rect_intersection(const struct wlf_rect *a, const struct wlf_rect *b) {
	// Return zero rectangle if no intersection
	if (!wlf_rect_intersects(a, b)) {
		return WLF_RECT_ZERO;
	}

	int x1 = fmax(a->x, b->x);
	int y1 = fmax(a->y, b->y);
	int x2 = fmin(a->x + a->width, b->x + b->width);
	int y2 = fmin(a->y + a->height, b->y + b->height);

	return wlf_rect_make(x1, y1, x2 - x1, y2 - y1);
}

struct wlf_rect wlf_rect_union(const struct wlf_rect *a, const struct wlf_rect *b) {
	int x1 = fmin(a->x, b->x);
	int y1 = fmin(a->y, b->y);
	int x2 = fmax(a->x + a->width, b->x + b->width);
	int y2 = fmax(a->y + a->height, b->y + b->height);

	return wlf_rect_make(x1, y1, x2 - x1, y2 - y1);
}
