#include "wlf/math/wlf_vector3.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define WLF_VECTOR3_STRLEN 192

struct wlf_vector3 wlf_vector3_make(double x, double y, double z) {
	return (struct wlf_vector3) {.x = x, .y = y, .z = z};
}

char* wlf_vector3_to_str(const struct wlf_vector3 *vector) {
	return wlf_vector3_to_str_prec(vector, 3);
}

char *wlf_vector3_to_str_prec(const struct wlf_vector3 *vector, uint8_t precision) {
	if (vector == NULL) {
		return strdup("(NULL)");
	}

	if (precision > 15) {
		precision = 15;
	}

	char fmt[48];
	snprintf(fmt, sizeof(fmt), "(%%.%df, %%.%df, %%.%df)", precision, precision, precision);

	char *buffer = malloc(WLF_VECTOR3_STRLEN);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate buffer");
		return NULL;
	}

	snprintf(buffer, WLF_VECTOR3_STRLEN, fmt, vector->x, vector->y, vector->z);

	return buffer;
}

struct wlf_vector3 wlf_vector3_add(const struct wlf_vector3 *a, const struct wlf_vector3 *b) {
	return wlf_vector3_make(a->x + b->x, a->y + b->y, a->z + b->z);
}

struct wlf_vector3 wlf_vector3_subtract(const struct wlf_vector3 *a, const struct wlf_vector3 *b) {
	return wlf_vector3_make(a->x - b->x, a->y - b->y, a->z - b->z);
}

struct wlf_vector3 wlf_vector3_multiply(const struct wlf_vector3 *vec, double scalar) {
	return wlf_vector3_make(vec->x * scalar, vec->y * scalar, vec->z * scalar);
}

struct wlf_vector3 wlf_vector3_divide(const struct wlf_vector3 *vec, double scalar) {
	assert(scalar != 0.0);
	return wlf_vector3_make(vec->x / scalar, vec->y / scalar, vec->z / scalar);
}

double wlf_vector3_dot(const struct wlf_vector3 *a, const struct wlf_vector3 *b) {
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

struct wlf_vector3 wlf_vector3_cross(const struct wlf_vector3 *a, const struct wlf_vector3 *b) {
	return wlf_vector3_make(
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

	return wlf_vector3_make(vec->x / mag, vec->y / mag, vec->z / mag);
}

bool wlf_vector3_equal(const struct wlf_vector3 *a, const struct wlf_vector3 *b) {
	return (a->x == b->x) && (a->y == b->y) && (a->z == b->z);
}

bool wlf_vector3_nearly_equal(const struct wlf_vector3 *a, const struct wlf_vector3 *b, double epsilon) {
	return (fabs(a->x - b->x) < epsilon) && (fabs(a->y - b->y) < epsilon) && (fabs(a->z - b->z) < epsilon);
}
