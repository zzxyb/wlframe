#include "wlf/math/wlf_vector2.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct wlf_vector2 wlf_vector2_create(double u, double v) {
	struct wlf_vector2 vec = {u, v};
	return vec;
}

char* wlf_vector2_to_str(const struct wlf_vector2 *vector) {
	char *buffer = malloc(64);
	if (buffer) {
		snprintf(buffer, 64, "Vector2(%.2f, %.2f)", vector->u, vector->v);
	}

	return buffer;
}

struct wlf_vector2 wlf_vector2_add(const struct wlf_vector2 *a, const struct wlf_vector2 *b) {
	return wlf_vector2_create(a->u + b->u, a->v + b->v);
}

struct wlf_vector2 wlf_vector2_subtract(const struct wlf_vector2 *a, const struct wlf_vector2 *b) {
	return wlf_vector2_create(a->u - b->u, a->v - b->v);
}

struct wlf_vector2 wlf_vector2_multiply(const struct wlf_vector2 *vec, double scalar) {
	return wlf_vector2_create(vec->u * scalar, vec->v * scalar);
}

struct wlf_vector2 wlf_vector2_divide(const struct wlf_vector2 *vec, double scalar) {
	if (scalar != 0) {
		return wlf_vector2_create(vec->u / scalar, vec->v / scalar);
	}

	// Return original vector if scalar is zero (error handling)
	return *vec;
}

double wlf_vector2_dot(const struct wlf_vector2 *a, const struct wlf_vector2 *b) {
	return (a->u * b->u) + (a->v * b->v);
}

double wlf_vector2_magnitude(const struct wlf_vector2 *vec) {
	return sqrt(vec->u * vec->u + vec->v * vec->v);
}

struct wlf_vector2 wlf_vector2_normalize(const struct wlf_vector2 *vec) {
	double mag = wlf_vector2_magnitude(vec);
	if (mag == 0) {
		// Return zero vector if magnitude is zero
		return WLF_VECTOR2_ZERO;
	}

	return wlf_vector2_create(vec->u / mag, vec->v / mag);
}

bool wlf_vector2_equal(const struct wlf_vector2 *a, const struct wlf_vector2 *b) {
	return (a->u == b->u) && (a->v == b->v);
}

bool wlf_vector2_nearly_equal(const struct wlf_vector2 *a, const struct wlf_vector2 *b, double epsilon) {
	return (fabs(a->u - b->u) < epsilon) && (fabs(a->v - b->v) < epsilon);
}