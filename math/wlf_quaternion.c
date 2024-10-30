#include "wlf/math/wlf_quaternion.h"

#include <stdlib.h>
#include <stdio.h>

struct wlf_quaternion wlf_quaternion_create(double w, double x, double y, double z) {
	struct wlf_quaternion q = {w, x, y, z};
	return q;
}

char* wlf_quaternion_to_str(const struct wlf_quaternion *quaternion) {
	char *buffer = malloc(64);
	if (buffer) {
		snprintf(buffer, 64, "Quaternion(%.2f, %.2f, %.2f, %.2f)", quaternion->w, quaternion->x, quaternion->y, quaternion->z);
	}

	return buffer;
}

struct wlf_quaternion wlf_quaternion_add(const struct wlf_quaternion *a, const struct wlf_quaternion *b) {
	return wlf_quaternion_create(a->w + b->w, a->x + b->x, a->y + b->y, a->z + b->z);
}

struct wlf_quaternion wlf_quaternion_subtract(const struct wlf_quaternion *a, const struct wlf_quaternion *b) {
	return wlf_quaternion_create(a->w - b->w, a->x - b->x, a->y - b->y, a->z - b->z);
}

struct wlf_quaternion wlf_quaternion_multiply(const struct wlf_quaternion *a, const struct wlf_quaternion *b) {
	return wlf_quaternion_create(
				a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z,
				a->w * b->x + a->x * b->w + a->y * b->z - a->z * b->y,
				a->w * b->y - a->x * b->z + a->y * b->w + a->z * b->x,
				a->w * b->z + a->x * b->y - a->y * b->x + a->z * b->w
	);
}

struct wlf_quaternion wlf_quaternion_conjugate(const struct wlf_quaternion *q) {
	return wlf_quaternion_create(q->w, -q->x, -q->y, -q->z);
}

double wlf_quaternion_norm(const struct wlf_quaternion *q) {
	return sqrt(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
}

struct wlf_quaternion wlf_quaternion_normalize(const struct wlf_quaternion *q) {
	double norm = wlf_quaternion_norm(q);
	if (norm == 0) {
		// Return identity if norm is zero
		return WLF_QUATERNION_IDENTITY;
	}

	return wlf_quaternion_create(q->w / norm, q->x / norm, q->y / norm, q->z / norm);
}

struct wlf_quaternion wlf_quaternion_inverse(const struct wlf_quaternion *q) {
	double norm_squared = wlf_quaternion_norm(q);
	if (norm_squared == 0) {
		// Return identity if norm is zero
		return WLF_QUATERNION_IDENTITY;
	}
	struct wlf_quaternion conjugate = wlf_quaternion_conjugate(q);
	return wlf_quaternion_create(conjugate.w / norm_squared, conjugate.x / norm_squared,
				conjugate.y / norm_squared, conjugate.z / norm_squared);
}

bool wlf_quaternion_equal(const struct wlf_quaternion *a, const struct wlf_quaternion *b) {
	return (a->w == b->w) && (a->x == b->x) && (a->y == b->y) && (a->z == b->z);
}

bool wlf_quaternion_nearly_equal(const struct wlf_quaternion *a, const struct wlf_quaternion *b, double epsilon) {
	return (fabs(a->w - b->w) < epsilon) && (fabs(a->x - b->x) < epsilon) &&
			(fabs(a->y - b->y) < epsilon) && (fabs(a->z - b->z) < epsilon);
}