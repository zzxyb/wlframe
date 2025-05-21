#include "wlf/math/wlf_matrix3x3.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define WLF_MATRIX3x3_STRLEN 128

struct wlf_matrix3x3 wlf_matrix3x3_create_zero(void) {
	struct wlf_matrix3x3 matrix = {0};

	return matrix;
}

struct wlf_matrix3x3 wlf_matrix3x3_identity(void) {
	struct wlf_matrix3x3 matrix = {0};
	matrix.elements[0][0] = 1.0;
	matrix.elements[1][1] = 1.0;
	matrix.elements[2][2] = 1.0;

	return matrix;
}

char* wlf_matrix3x3_to_str(const struct wlf_matrix3x3 *matrix) {
	if (matrix == NULL) {
		return strdup("(NULL)");
	}

	char *buffer = malloc(WLF_MATRIX3x3_STRLEN);
	if (buffer == NULL) {
		wlf_log(WLF_ERROR, "Memory allocation failed for wlf_matrix3x3_to_str");
		return NULL;
	}

	snprintf(buffer, WLF_MATRIX3x3_STRLEN,
		"[[%.3f, %.3f, %.3f],\n"
		" [%.3f, %.3f, %.3f],\n"
		" [%.3f, %.3f, %.3f]]",
		matrix->elements[0][0], matrix->elements[0][1], matrix->elements[0][2],
		matrix->elements[1][0], matrix->elements[1][1], matrix->elements[1][2],
		matrix->elements[2][0], matrix->elements[2][1], matrix->elements[2][2]);

	return buffer;
}

double wlf_matrix3x3_get(const struct wlf_matrix3x3 *matrix, int row, int col) {
	return matrix->elements[row][col];
}

void wlf_matrix3x3_set(struct wlf_matrix3x3 *matrix, int row, int col, double value) {
	matrix->elements[row][col] = value;
}

struct wlf_matrix3x3 wlf_matrix3x3_add(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b) {
	struct wlf_matrix3x3 result = {0};
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				result.elements[i][j] = a->elements[i][j] + b->elements[i][j];
		}
	}

	return result;
}

struct wlf_matrix3x3 wlf_matrix3x3_subtract(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b) {
	struct wlf_matrix3x3 result = {0};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.elements[i][j] = a->elements[i][j] - b->elements[i][j];
		}
	}

	return result;
}

struct wlf_matrix3x3 wlf_matrix3x3_multiply_scalar(const struct wlf_matrix3x3 *matrix, double scalar) {
	struct wlf_matrix3x3 result = {0};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.elements[i][j] = matrix->elements[i][j] * scalar;
		}
	}

	return result;
}

struct wlf_matrix3x3 wlf_matrix3x3_multiply(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b) {
	struct wlf_matrix3x3 result = {0};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				result.elements[i][j] += a->elements[i][k] * b->elements[k][j];
			}
		}
	}

	return result;
}

struct wlf_matrix3x3 wlf_matrix3x3_transpose(const struct wlf_matrix3x3 *matrix) {
	struct wlf_matrix3x3 result = {0};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.elements[j][i] = matrix->elements[i][j];
		}
	}

	return result;
}

double wlf_matrix3x3_determinant(const struct wlf_matrix3x3 *matrix) {
	return matrix->elements[0][0] * (matrix->elements[1][1] * matrix->elements[2][2] - matrix->elements[1][2] * matrix->elements[2][1]) -
			matrix->elements[0][1] * (matrix->elements[1][0] * matrix->elements[2][2] - matrix->elements[1][2] * matrix->elements[2][0]) +
			matrix->elements[0][2] * (matrix->elements[1][0] * matrix->elements[2][1] - matrix->elements[1][1] * matrix->elements[2][0]);
}

struct wlf_matrix3x3 wlf_matrix3x3_inverse(const struct wlf_matrix3x3 *matrix) {
	struct wlf_matrix3x3 result = {0};
	double det = wlf_matrix3x3_determinant(matrix);

	if (det == 0) {
		// The matrix is not invertible, return a zero matrix
		return result;
	}

	result.elements[0][0] = (matrix->elements[1][1] * matrix->elements[2][2] - matrix->elements[1][2] * matrix->elements[2][1]) / det;
	result.elements[0][1] = (matrix->elements[0][2] * matrix->elements[2][1] - matrix->elements[0][1] * matrix->elements[2][2]) / det;
	result.elements[0][2] = (matrix->elements[0][1] * matrix->elements[1][2] - matrix->elements[0][2] * matrix->elements[1][1]) / det;
	result.elements[1][0] = (matrix->elements[1][2] * matrix->elements[2][0] - matrix->elements[1][0] * matrix->elements[2][2]) / det;
	result.elements[1][1] = (matrix->elements[0][0] * matrix->elements[2][2] - matrix->elements[0][2] * matrix->elements[2][0]) / det;
	result.elements[1][2] = (matrix->elements[0][2] * matrix->elements[1][0] - matrix->elements[0][0] * matrix->elements[1][2]) / det;
	result.elements[2][0] = (matrix->elements[1][0] * matrix->elements[2][1] - matrix->elements[1][1] * matrix->elements[2][0]) / det;
	result.elements[2][1] = (matrix->elements[0][1] * matrix->elements[2][0] - matrix->elements[0][0] * matrix->elements[2][1]) / det;
	result.elements[2][2] = (matrix->elements[0][0] * matrix->elements[1][1] - matrix->elements[0][1] * matrix->elements[1][0]) / det;

	return result;
}

bool wlf_matrix3x3_equal(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (a->elements[i][j] != b->elements[i][j]) {
				return false;
			}
		}
	}

	return true;
}

bool wlf_matrix3x3_nearly_equal(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b, double epsilon) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (fabs(a->elements[i][j] - b->elements[i][j]) > epsilon) {
				return false;
			}
		}
	}

	return true;
}
