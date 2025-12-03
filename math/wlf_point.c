#include "wlf/math/wlf_point.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define WLF_POINT_STRLEN 32

char *wlf_point_to_str(const struct wlf_point *point) {
	if (point == NULL) {
		return strdup("(NULL)");
	}

	char *buffer = malloc(WLF_POINT_STRLEN);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate buffer");
		return NULL;
	}

	snprintf(buffer, WLF_POINT_STRLEN, "(%d, %d)", point->x, point->y);

	return buffer;
}

bool wlf_point_equal(const struct wlf_point *a, const struct wlf_point *b) {
	return (a->x == b->x) && (a->y == b->y);
}

bool wlf_point_is_zero(const struct wlf_point *p) {
	return (p->x == 0) && (p->y == 0);
}

struct wlf_point wlf_point_add(const struct wlf_point *a, const struct wlf_point *b) {
	return (struct wlf_point){.x = a->x + b->x, .y = a->y + b->y};
}

struct wlf_point wlf_point_subtract(const struct wlf_point *a, const struct wlf_point *b) {
	return (struct wlf_point){.x = a->x - b->x, .y = a->y - b->y};
}

struct wlf_point wlf_point_multiply(const struct wlf_point *p, double scalar) {
	return (struct wlf_point){.x = (int)round(p->x * scalar), .y = (int)round(p->y * scalar)};
}

int wlf_point_manhattan_distance(const struct wlf_point *p1, const struct wlf_point *p2) {
	return abs(p1->x - p2->x) + abs(p1->y - p2->y);
}

double wlf_point_euclidean_distance(const struct wlf_point *p1, const struct wlf_point *p2) {
	return sqrt(pow(p1->x - p2->x, 2) + pow(p1->y - p2->y, 2));
}

bool wlf_point_from_str(const char *str, struct wlf_point *point) {
	if (str == NULL || point == NULL) {
		return false;
	}

	while (*str == ' ' || *str == '\t') {
		str++;
	}

	if (*str != '(') {
		return false;
	}
	str++;

	char *endptr;
	long x = strtol(str, &endptr, 10);
	if (endptr == str) {
		return false;
	}
	str = endptr;

	while (*str == ' ' || *str == '\t') {
		str++;
	}

	if (*str != ',') {
		return false;
	}
	str++;

	while (*str == ' ' || *str == '\t') {
		str++;
	}

	long y = strtol(str, &endptr, 10);
	if (endptr == str) {
		return false;
	}
	str = endptr;

	while (*str == ' ' || *str == '\t') {
		str++;
	}

	if (*str != ')') {
		return false;
	}

	point->x = (int)x;
	point->y = (int)y;

	return true;
}
