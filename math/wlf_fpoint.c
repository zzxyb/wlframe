#include "wlf/math/wlf_fpoint.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define WLF_FPOINT_STRLEN 128

char *wlf_fpoint_to_str(const struct wlf_fpoint *point) {
	return wlf_fpoint_to_str_prec(point, 3);
}

char *wlf_fpoint_to_str_prec(const struct wlf_fpoint *point, uint8_t precision) {
	if (point == NULL) {
		return strdup("(NULL)");
	}

	if (precision > 15) {
		precision = 15;
	}

	char fmt[32];
	snprintf(fmt, sizeof(fmt), "(%%.%df, %%.%df)", precision, precision);

	char *buffer = malloc(WLF_FPOINT_STRLEN);
	if (buffer == NULL) {
		wlf_log(WLF_ERROR, "Memory allocation failed for wlf_fpoint_to_str_prec");
		return NULL;
	}

	snprintf(buffer, WLF_FPOINT_STRLEN, fmt, point->x, point->y);

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
	return (struct wlf_fpoint){.x = a->x + b->x, .y = a->y + b->y};
}

struct wlf_fpoint wlf_fpoint_subtract(const struct wlf_fpoint *a, const struct wlf_fpoint *b) {
	return (struct wlf_fpoint){.x = a->x - b->x, .y = a->y - b->y};
}

struct wlf_fpoint wlf_fpoint_multiply(const struct wlf_fpoint *p, double scalar) {
	return (struct wlf_fpoint){.x = p->x * scalar, .y = p->y * scalar};
}

struct wlf_fpoint wlf_fpoint_divide(const struct wlf_fpoint *p, double scalar) {
	assert(scalar != 0.0);
	return (struct wlf_fpoint){.x = p->x / scalar, .y = p->y / scalar};
}

struct wlf_fpoint wlf_fpoint_negate(const struct wlf_fpoint *p) {
	return (struct wlf_fpoint){.x = -p->x, .y = -p->y};
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
	return (struct wlf_fpoint){
			.x = p->x * cos_angle - p->y * sin_angle,
			.y = p->x * sin_angle + p->y * cos_angle
		};
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
	return (struct wlf_point){.x = round(p->x), .y = round(p->y)};
}

struct wlf_point wlf_fpoint_floor(const struct wlf_fpoint *p) {
	return (struct wlf_point){.x = floor(p->x), .y = floor(p->y)};
}

struct wlf_point wlf_fpoint_ceil(const struct wlf_fpoint *p) {
	return (struct wlf_point){.x = ceil(p->x), .y = ceil(p->y)};
}

struct wlf_fpoint wlf_fpoint_normalize(const struct wlf_fpoint *p) {
	double length = wlf_fpoint_length(p);
	if (length == 0) {
		return (struct wlf_fpoint){.x = 0, .y = 0};
	}

	return (struct wlf_fpoint){.x = p->x / length, .y = p->y / length};
}

struct wlf_fpoint wlf_fpoint_lerp(const struct wlf_fpoint *a, const struct wlf_fpoint *b, double t) {
	return (struct wlf_fpoint){.x = a->x + (b->x - a->x) * t, .y = a->y + (b->y - a->y) * t};
}

struct wlf_fpoint wlf_fpoint_bezier(const struct wlf_fpoint *p0, const struct wlf_fpoint *p1,
		const struct wlf_fpoint *p2, double t) {
	double u = 1 - t;
	return (struct wlf_fpoint){.x = u * u * p0->x + 2 * u * t * p1->x + t * t * p2->x,
								.y = u * u * p0->y + 2 * u * t * p1->y + t * t * p2->y};
}

struct wlf_fpoint wlf_point_to_fpoint(const struct wlf_point *p) {
	return (struct wlf_fpoint){.x = (double)p->x, .y = (double)p->y};
}

struct wlf_point wlf_fpoint_to_point(const struct wlf_fpoint *p) {
	return (struct wlf_point){.x = (int)p->x, .y = (int)p->y};
}

bool wlf_fpoint_from_str(const char *str, struct wlf_fpoint *point) {
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
	double x = strtod(str, &endptr);
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

	double y = strtod(str, &endptr);
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

	point->x = x;
	point->y = y;
	return true;
}
