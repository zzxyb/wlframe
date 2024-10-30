#include "wlf/math/wlf_vector3.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct wlf_vector3 wlf_vector3_create(double x, double y, double z) {
	struct wlf_vector3 vec = {x, y, z};

	return vec;
}

char* wlf_vector3_to_str(const struct wlf_vector3 *vector) {
	char *buffer = malloc(64);
	if (buffer) {
		snprintf(buffer, 64, "Vector3(%.2f, %.2f, %.2f)", vector->x, vector->y, vector->z);
	}

	return buffer;
}

struct wlf_vector3 wlf_vector3_add(const struct wlf_vector3 *a, const struct wlf_vector3 *b) {
	return wlf_vector3_create(a->x + b->x, a->y + b->y, a->z + b->z);
}

struct wlf_vector3 wlf_vector3_subtract(const struct wlf_vector3 *a, const struct wlf_vector3 *b) {
	return wlf_vector3_create(a->x - b->x, a->y - b->y, a->z - b->z);
}

struct wlf_vector3 wlf_vector3_multiply(const struct wlf_vector3 *vec, double scalar) {
	return wlf_vector3_create(vec->x * scalar, vec->y * scalar, vec->z * scalar);
}

struct wlf_vector3 wlf_vector3_divide(const struct wlf_vector3 *vec, double scalar) {
	if (scalar != 0) {
		return wlf_vector3_create(vec->x / scalar, vec->y / scalar, vec->z / scalar);
	}

	// Return original vector if scalar is zero (error handling)
	return *vec;
}

double wlf_vector3_dot(const struct wlf_vector3 *a, const struct wlf_vector3 *b) {
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

struct wlf_vector3 wlf_vector3_cross(const struct wlf_vector3 *a, const struct wlf_vector3 *b) {
	return wlf_vector3_create(
			a->y * b->z - a->z * b->y,
			a->z * b->x - a->x * b->z,
			a->x * b->y - a->y * b->x
	);
}

double wlf_vector3_magnitude(const struct wlf_vector3 *vec) {
	return sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
}

struct wlf_vector3 wlf_vector3_normalize(const struct wlf_vector3 *vec) {
	double mag = wlf_vector3_magnitude(vec);
	if (mag == 0) {
		// Return zero vector if magnitude is zero
		return WLF_VECTOR3_ZERO;
    }

	return wlf_vector3_create(vec->x / mag, vec->y / mag, vec->z / mag);
}

bool wlf_vector3_equal(const struct wlf_vector3 *a, const struct wlf_vector3 *b) {
	return (a->x == b->x) && (a->y == b->y) && (a->z == b->z);
}

bool wlf_vector3_nearly_equal(const struct wlf_vector3 *a, const struct wlf_vector3 *b, double epsilon) {
	return (fabs(a->x - b->x) < epsilon) && (fabs(a->y - b->y) < epsilon) && (fabs(a->z - b->z) < epsilon);
}