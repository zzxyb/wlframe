#include "wlf/math/wlf_point.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

struct wlf_point wlf_point_make(int x, int y) {
	struct wlf_point p = {x, y};
	return p;
}

char *wlf_point_to_str(const struct wlf_point *point) {
	char *buffer = malloc(64);
	if (buffer) {
		snprintf(buffer, 64, "Point(%d, %d)", point->x, point->y);
	}

	return buffer;
}

bool wlf_point_equal(const struct wlf_point *a, const struct wlf_point *b) {
	return (a->x == b->x) && (a->y == b->y);
}

bool wlf_point_is_zero(const struct wlf_point *p) {
	return (p->x == 0) && (p->y == 0);
}

struct wlf_point wlf_point_add(const struct wlf_point *a, const struct wlf_point *b) {
	return wlf_point_make(a->x + b->x, a->y + b->y);
}

struct wlf_point wlf_point_subtract(const struct wlf_point *a, const struct wlf_point *b) {
	return wlf_point_make(a->x - b->x, a->y - b->y);
}

struct wlf_point wlf_point_multiply(const struct wlf_point *p, int scalar) {
	return wlf_point_make(p->x * scalar, p->y * scalar);
}

int wlf_point_manhattan_distance(const struct wlf_point *p1, const struct wlf_point *p2) {
	return abs(p1->x - p2->x) + abs(p1->y - p2->y);
}

double wlf_point_euclidean_distance(const struct wlf_point *p1, const struct wlf_point *p2) {
	return sqrt(pow(p1->x - p2->x, 2) + pow(p1->y - p2->y, 2));
}
