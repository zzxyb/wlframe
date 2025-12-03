#include "wlf/math/wlf_vector2.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define WLF_VECTOR2_STRLEN 128

char* wlf_vector2_to_str(const struct wlf_vector2 *vector) {
	return wlf_vector2_to_str_prec(vector, 3);
}

char *wlf_vector2_to_str_prec(const struct wlf_vector2 *vector, uint8_t precision) {
	if (vector == NULL) {
		return strdup("(NULL)");
	}

	if (precision > 15) {
		precision = 15;
	}

	char fmt[32];
	snprintf(fmt, sizeof(fmt), "(%%.%df, %%.%df)", precision, precision);

	char *buffer = malloc(WLF_VECTOR2_STRLEN);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate buffer");
		return NULL;
	}

	snprintf(buffer, WLF_VECTOR2_STRLEN, fmt, vector->u, vector->v);

	return buffer;
}

struct wlf_vector2 wlf_vector2_add(const struct wlf_vector2 *a, const struct wlf_vector2 *b) {
	return (struct wlf_vector2) {.u = a->u + b->u, .v = a->v + b->v};
}

struct wlf_vector2 wlf_vector2_subtract(const struct wlf_vector2 *a, const struct wlf_vector2 *b) {
	return (struct wlf_vector2) {.u = a->u - b->u, .v = a->v - b->v};
}

struct wlf_vector2 wlf_vector2_multiply(const struct wlf_vector2 *vec, double scalar) {
	return (struct wlf_vector2) {.u = vec->u * scalar, .v = vec->v * scalar};
}

struct wlf_vector2 wlf_vector2_divide(const struct wlf_vector2 *vec, double scalar) {
	assert(scalar != 0.0);
	return (struct wlf_vector2) {.u = vec->u / scalar, .v = vec->v / scalar};
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
		return WLF_VECTOR2_ZERO;
	}

	return (struct wlf_vector2) {.u = vec->u / mag, .v = vec->v / mag};
}

bool wlf_vector2_equal(const struct wlf_vector2 *a, const struct wlf_vector2 *b) {
	return (a->u == b->u) && (a->v == b->v);
}

bool wlf_vector2_nearly_equal(const struct wlf_vector2 *a, const struct wlf_vector2 *b, double epsilon) {
	return (fabs(a->u - b->u) < epsilon) && (fabs(a->v - b->v) < epsilon);
}
