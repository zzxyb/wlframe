#include "wlf/math/wlf_vector4.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define WLF_VECTOR4_STRLEN 256

struct wlf_vector4 wlf_vector4_make(double x, double y, double z, double w) {
	struct wlf_vector4 vec = {x, y, z, w};

	return vec;
}

char* wlf_vector4_to_str(const struct wlf_vector4 *vector) {
	return wlf_vector4_to_str_prec(vector, 3);
}

char *wlf_vector4_to_str_prec(const struct wlf_vector4 *vector, uint8_t precision) {
	if (vector == NULL) {
		return strdup("(NULL)");
	}

	if (precision > 15) {
		precision = 15;
	}

	char fmt[64];
	snprintf(fmt, sizeof(fmt), "(%%.%df, %%.%df, %%.%df, %%.%df)", precision, precision, precision, precision);

	char *buffer = malloc(WLF_VECTOR4_STRLEN);
	if (buffer == NULL) {
		wlf_log(WLF_ERROR, "Memory allocation failed for wlf_vector4_to_str_prec");
		return NULL;
	}

	snprintf(buffer, WLF_VECTOR4_STRLEN, fmt, vector->x, vector->y, vector->z, vector->w);

	return buffer;
}

struct wlf_vector4 wlf_vector4_add(const struct wlf_vector4 *a, const struct wlf_vector4 *b) {
	return wlf_vector4_make(a->x + b->x, a->y + b->y, a->z + b->z, a->w + b->w);
}

struct wlf_vector4 wlf_vector4_subtract(const struct wlf_vector4 *a, const struct wlf_vector4 *b) {
	return wlf_vector4_make(a->x - b->x, a->y - b->y, a->z - b->z, a->w - b->w);
}

struct wlf_vector4 wlf_vector4_multiply(const struct wlf_vector4 *vec, double scalar) {
	return wlf_vector4_make(vec->x * scalar, vec->y * scalar, vec->z * scalar, vec->w * scalar);
}

struct wlf_vector4 wlf_vector4_divide(const struct wlf_vector4 *vec, double scalar) {
	assert(scalar != 0.0);
	return wlf_vector4_make(vec->x / scalar, vec->y / scalar, vec->z / scalar, vec->w / scalar);
}

double wlf_vector4_dot(const struct wlf_vector4 *a, const struct wlf_vector4 *b) {
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z) + (a->w * b->w);
}

double wlf_vector4_magnitude(const struct wlf_vector4 *vec) {
	return sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z + vec->w * vec->w);
}

struct wlf_vector4 wlf_vector4_normalize(const struct wlf_vector4 *vec) {
	double mag = wlf_vector4_magnitude(vec);
	if (mag == 0) {
		// Return zero vector if magnitude is zero
		return WLF_VECTOR4_ZERO;
	}

	return wlf_vector4_make(vec->x / mag, vec->y / mag, vec->z / mag, vec->w / mag);
}

bool wlf_vector4_equal(const struct wlf_vector4 *a, const struct wlf_vector4 *b) {
	return (a->x == b->x) && (a->y == b->y) && (a->z == b->z) && (a->w == b->w);
}

bool wlf_vector4_nearly_equal(const struct wlf_vector4 *a, const struct wlf_vector4 *b, double epsilon) {
	return (fabs(a->x - b->x) < epsilon) && (fabs(a->y - b->y) < epsilon) &&
			(fabs(a->z - b->z) < epsilon) && (fabs(a->w - b->w) < epsilon);
}
