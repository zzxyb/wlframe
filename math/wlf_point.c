#include "wlf/math/wlf_point.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct wlf_point wlf_point_create(int x, int y) {
	struct wlf_point p = {x, y};
	return p;
}

char* wlf_point_to_str(const struct wlf_point *point) {
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
	return wlf_point_create(a->x + b->x, a->y + b->y);
}

struct wlf_point wlf_point_subtract(const struct wlf_point *a, const struct wlf_point *b) {
	return wlf_point_create(a->x - b->x, a->y - b->y);
}

struct wlf_point wlf_point_multiply(const struct wlf_point *p, int scalar) {
	return wlf_point_create(p->x * scalar, p->y * scalar);
}

int wlf_point_manhattan_distance(const struct wlf_point *p1, const struct wlf_point *p2) {
	return abs(p1->x - p2->x) + abs(p1->y - p2->y);
}

double wlf_point_euclidean_distance(const struct wlf_point *p1, const struct wlf_point *p2) {
	return sqrt(pow(p1->x - p2->x, 2) + pow(p1->y - p2->y, 2));
}

struct wlf_fpoint wlf_fpoint_create(double x, double y) {
	struct wlf_fpoint p = {x, y};
	return p;
}

char* wlf_fpoint_to_str(const struct wlf_fpoint *point) {
	char *buffer = malloc(64);
	if (buffer) {
		snprintf(buffer, 64, "FPoint(%.2f, %.2f)", point->x, point->y);
	}

	return buffer;
}

bool wlf_fpoint_equal(const struct wlf_fpoint *a, const struct wlf_fpoint *b) {
	return (a->x == b->x) && (a->y == b->y);
}

bool wlf_fpoint_nearly_equal(const struct wlf_fpoint *a, const struct wlf_fpoint *b, double epsilon) {
	return (fabs(a->x - b->x) < epsilon) && (fabs(a->y - b->y) < epsilon);
}

bool wlf_fpoint_is_zero(const struct wlf_fpoint *p) {
	return (p->x == 0.0) && (p->y == 0.0);
}

struct wlf_fpoint wlf_fpoint_add(const struct wlf_fpoint *a, const struct wlf_fpoint *b) {
	return wlf_fpoint_create(a->x + b->x, a->y + b->y);
}

struct wlf_fpoint wlf_fpoint_subtract(const struct wlf_fpoint *a, const struct wlf_fpoint *b) {
	return wlf_fpoint_create(a->x - b->x, a->y - b->y);
}

struct wlf_fpoint wlf_fpoint_multiply(const struct wlf_fpoint *p, double scalar) {
	return wlf_fpoint_create(p->x * scalar, p->y * scalar);
}

struct wlf_fpoint wlf_fpoint_divide(const struct wlf_fpoint *p, double scalar) {
	return wlf_fpoint_create(p->x / scalar, p->y / scalar);
}

struct wlf_fpoint wlf_fpoint_negate(const struct wlf_fpoint *p) {
	return wlf_fpoint_create(-p->x, -p->y);
}

double wlf_fpoint_manhattan_distance(const struct wlf_fpoint *p1, const struct wlf_fpoint *p2) {
	return fabs(p1->x - p2->x) + fabs(p1->y - p2->y);
}

double wlf_fpoint_euclidean_distance(const struct wlf_fpoint *p1, const struct wlf_fpoint *p2) {
	return sqrt(pow(p1->x - p2->x, 2) + pow(p1->y - p2->y, 2));
}

double wlf_fpoint_dot_product(const struct wlf_fpoint *a, const struct wlf_fpoint *b) {
	return (a->x * b->x) + (a->y * b->y);
}

double wlf_fpoint_angle(const struct wlf_fpoint *p) {
	return atan2(p->y, p->x);
}

double wlf_fpoint_angle_between(const struct wlf_fpoint *a, const struct wlf_fpoint *b) {
	return wlf_fpoint_angle(b) - wlf_fpoint_angle(a);
}

struct wlf_fpoint wlf_fpoint_rotate(const struct wlf_fpoint *p, double angle_radians) {
	double cos_angle = cos(angle_radians);
	double sin_angle = sin(angle_radians);
	return wlf_fpoint_create(p->x * cos_angle - p->y * sin_angle,
									p->x * sin_angle + p->y * cos_angle);
}

double wlf_fpoint_length(const struct wlf_fpoint *p) {
	return sqrt(p->x * p->x + p->y * p->y);
}

double wlf_fpoint_length_squared(const struct wlf_fpoint *p) {
	return (p->x * p->x + p->y * p->y);
}

bool wlf_fpoint_in_circle(const struct wlf_fpoint *p, const struct wlf_fpoint *center, double radius) {
	return wlf_fpoint_euclidean_distance(p, center) <= radius;
}

struct wlf_point wlf_fpoint_round(const struct wlf_fpoint *p) {
	return wlf_point_create(round(p->x), round(p->y));
}

struct wlf_point wlf_fpoint_floor(const struct wlf_fpoint *p) {
	return wlf_point_create(floor(p->x), floor(p->y));
}

struct wlf_point wlf_fpoint_ceil(const struct wlf_fpoint *p) {
	return wlf_point_create(ceil(p->x), ceil(p->y));
}

struct wlf_fpoint wlf_fpoint_normalize(const struct wlf_fpoint *p) {
	double length = wlf_fpoint_length(p);
	if (length == 0) {
		return wlf_fpoint_create(0, 0);
	}

	return wlf_fpoint_create(p->x / length, p->y / length);
}

struct wlf_fpoint wlf_fpoint_lerp(const struct wlf_fpoint *a, const struct wlf_fpoint *b, double t) {
	return wlf_fpoint_create(a->x + (b->x - a->x) * t, a->y + (b->y - a->y) * t);
}

struct wlf_fpoint wlf_fpoint_bezier(const struct wlf_fpoint *p0, const struct wlf_fpoint *p1, 
		const struct wlf_fpoint *p2, double t) {
	double u = 1 - t;
	return wlf_fpoint_create(u * u * p0->x + 2 * u * t * p1->x + t * t * p2->x,
									u * u * p0->y + 2 * u * t * p1->y + t * t * p2->y);
}

struct wlf_fpoint wlf_point_to_fpoint(const struct wlf_point *p) {
	return wlf_fpoint_create((double)p->x, (double)p->y);
}

struct wlf_point wlf_fpoint_to_point(const struct wlf_fpoint *p) {
	return wlf_point_create((int)p->x, (int)p->y);
}