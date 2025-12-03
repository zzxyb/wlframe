#include "wlf/math/wlf_matrix4x4.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define WLF_MATRIX4x4_STRLEN 256

struct wlf_matrix4x4 wlf_matrix4x4_create_zero(void) {
	struct wlf_matrix4x4 matrix = {0};

	return matrix;
}

struct wlf_matrix4x4 wlf_matrix4x4_identity(void) {
	struct wlf_matrix4x4 matrix = {0};
	matrix.elements[0][0] = 1.0;
	matrix.elements[1][1] = 1.0;
	matrix.elements[2][2] = 1.0;
	matrix.elements[3][3] = 1.0;

	return matrix;
}

char* wlf_matrix4x4_to_str(const struct wlf_matrix4x4 *matrix) {
	if (matrix == NULL) {
		return strdup("(NULL)");
	}

	char *buffer = malloc(WLF_MATRIX4x4_STRLEN);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate buffer");
		return NULL;
	}

	snprintf(buffer, WLF_MATRIX4x4_STRLEN,
			"[[%.3f, %.3f, %.3f, %.3f],\n"
			" [%.3f, %.3f, %.3f, %.3f],\n"
			" [%.3f, %.3f, %.3f, %.3f],\n"
			" [%.3f, %.3f, %.3f, %.3f]]",
			matrix->elements[0][0], matrix->elements[0][1], matrix->elements[0][2], matrix->elements[0][3],
			matrix->elements[1][0], matrix->elements[1][1], matrix->elements[1][2], matrix->elements[1][3],
			matrix->elements[2][0], matrix->elements[2][1], matrix->elements[2][2], matrix->elements[2][3],
			matrix->elements[3][0], matrix->elements[3][1], matrix->elements[3][2], matrix->elements[3][3]);

	return buffer;
}

double wlf_matrix4x4_get(const struct wlf_matrix4x4 *matrix, int row, int col) {
	return matrix->elements[row][col];
}

void wlf_matrix4x4_set(struct wlf_matrix4x4 *matrix, int row, int col, double value) {
	matrix->elements[row][col] = value;
}

struct wlf_matrix4x4 wlf_matrix4x4_add(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b) {
	struct wlf_matrix4x4 result = {0};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.elements[i][j] = a->elements[i][j] + b->elements[i][j];
		}
	}

	return result;
}

struct wlf_matrix4x4 wlf_matrix4x4_subtract(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b) {
	struct wlf_matrix4x4 result = {0};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.elements[i][j] = a->elements[i][j] - b->elements[i][j];
		}
	}

	return result;
}

struct wlf_matrix4x4 wlf_matrix4x4_multiply_scalar(const struct wlf_matrix4x4 *matrix, double scalar) {
	struct wlf_matrix4x4 result = {0};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.elements[i][j] = matrix->elements[i][j] * scalar;
		}
	}

	return result;
}

struct wlf_matrix4x4 wlf_matrix4x4_multiply(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b) {
	struct wlf_matrix4x4 result = {0};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				result.elements[i][j] += a->elements[i][k] * b->elements[k][j];
			}
		}
	}

	return result;
}

struct wlf_matrix4x4 wlf_matrix4x4_transpose(const struct wlf_matrix4x4 *matrix) {
	struct wlf_matrix4x4 result = {0};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.elements[j][i] = matrix->elements[i][j];
		}
	}

	return result;
}

double wlf_matrix4x4_determinant(const struct wlf_matrix4x4 *matrix) {
	double det = 0.0;
	for (int i = 0; i < 4; i++) {
		det += (matrix->elements[0][i] *
				(matrix->elements[1][(i+1)%4] * matrix->elements[2][(i+2)%4] * matrix->elements[3][(i+3)%4] -
				matrix->elements[1][(i+2)%4] * matrix->elements[2][(i+1)%4] * matrix->elements[3][(i+3)%4]));
	}

	return det;
}

struct wlf_matrix4x4 wlf_matrix4x4_inverse(const struct wlf_matrix4x4 *matrix) {
	struct wlf_matrix4x4 result = {0};
	double det = wlf_matrix4x4_determinant(matrix);

	if (det == 0) {
		// The matrix is not invertible, return a zero matrix
		return result;
	}

	result.elements[0][0] = (matrix->elements[1][1] * (matrix->elements[2][2] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][2]) -
								matrix->elements[1][2] * (matrix->elements[2][1] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][1]) +
								matrix->elements[1][3] * (matrix->elements[2][1] * matrix->elements[3][2] - matrix->elements[2][2] * matrix->elements[3][1])) / det;

	result.elements[0][1] = -(matrix->elements[1][0] * (matrix->elements[2][2] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][2]) -
								matrix->elements[1][2] * (matrix->elements[2][0] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][0]) +
								matrix->elements[1][3] * (matrix->elements[2][0] * matrix->elements[3][2] - matrix->elements[2][2] * matrix->elements[3][0])) / det;

	result.elements[0][2] = (matrix->elements[1][0] * (matrix->elements[2][1] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][1]) -
								matrix->elements[1][1] * (matrix->elements[2][0] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][0]) +
								matrix->elements[1][3] * (matrix->elements[2][0] * matrix->elements[3][1] - matrix->elements[2][1] * matrix->elements[3][0])) / det;

	result.elements[0][3] = -(matrix->elements[1][0] * (matrix->elements[2][1] * matrix->elements[3][2] - matrix->elements[2][2] * matrix->elements[3][1]) -
								matrix->elements[1][1] * (matrix->elements[2][0] * matrix->elements[3][2] - matrix->elements[2][2] * matrix->elements[3][0]) +
								matrix->elements[1][2] * (matrix->elements[2][0] * matrix->elements[3][1] - matrix->elements[2][1] * matrix->elements[3][0])) / det;

	result.elements[1][0] = -(matrix->elements[0][1] * (matrix->elements[2][2] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][2]) -
								matrix->elements[0][2] * (matrix->elements[2][1] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][1]) +
								matrix->elements[0][3] * (matrix->elements[2][1] * matrix->elements[3][2] - matrix->elements[2][2] * matrix->elements[3][1])) / det;

	result.elements[1][1] = (matrix->elements[0][0] * (matrix->elements[2][2] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][2]) -
								matrix->elements[0][2] * (matrix->elements[2][0] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][0]) +
								matrix->elements[0][3] * (matrix->elements[2][0] * matrix->elements[3][2] - matrix->elements[2][2] * matrix->elements[3][0])) / det;

	result.elements[1][2] = -(matrix->elements[0][0] * (matrix->elements[2][1] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][1]) -
								matrix->elements[0][1] * (matrix->elements[2][0] * matrix->elements[3][3] - matrix->elements[2][3] * matrix->elements[3][0]) +
								matrix->elements[0][3] * (matrix->elements[2][0] * matrix->elements[3][1] - matrix->elements[2][1] * matrix->elements[3][0])) / det;

	result.elements[1][3] = (matrix->elements[0][0] * (matrix->elements[2][1] * matrix->elements[3][2] - matrix->elements[2][2] * matrix->elements[3][1]) -
								matrix->elements[0][1] * (matrix->elements[2][0] * matrix->elements[3][2] - matrix->elements[2][2] * matrix->elements[3][0]) +
								matrix->elements[0][2] * (matrix->elements[2][0] * matrix->elements[3][1] - matrix->elements[2][1] * matrix->elements[3][0])) / det;

	result.elements[2][0] = (matrix->elements[0][1] * (matrix->elements[1][2] * matrix->elements[3][3] - matrix->elements[1][3] * matrix->elements[3][2]) -
								matrix->elements[0][2] * (matrix->elements[1][1] * matrix->elements[3][3] - matrix->elements[1][3] * matrix->elements[3][1]) +
								matrix->elements[0][3] * (matrix->elements[1][1] * matrix->elements[3][2] - matrix->elements[1][2] * matrix->elements[3][1])) / det;

	result.elements[2][1] = -(matrix->elements[0][0] * (matrix->elements[1][2] * matrix->elements[3][3] - matrix->elements[1][3] * matrix->elements[3][2]) -
								matrix->elements[0][2] * (matrix->elements[1][0] * matrix->elements[3][3] - matrix->elements[1][3] * matrix->elements[3][0]) +
								matrix->elements[0][3] * (matrix->elements[1][0] * matrix->elements[3][2] - matrix->elements[1][2] * matrix->elements[3][0])) / det;

	result.elements[2][2] = (matrix->elements[0][0] * (matrix->elements[1][1] * matrix->elements[3][3] - matrix->elements[1][3] * matrix->elements[3][1]) -
								matrix->elements[0][1] * (matrix->elements[1][0] * matrix->elements[3][3] - matrix->elements[1][3] * matrix->elements[3][0]) +
								matrix->elements[0][3] * (matrix->elements[1][0] * matrix->elements[3][1] - matrix->elements[1][1] * matrix->elements[3][0])) / det;

	result.elements[2][3] = -(matrix->elements[0][0] * (matrix->elements[1][1] * matrix->elements[3][2] - matrix->elements[1][2] * matrix->elements[3][1]) -
								matrix->elements[0][1] * (matrix->elements[1][0] * matrix->elements[3][2] - matrix->elements[1][2] * matrix->elements[3][0]) +
								matrix->elements[0][2] * (matrix->elements[1][0] * matrix->elements[3][1] - matrix->elements[1][1] * matrix->elements[3][0])) / det;

	result.elements[3][0] = -(matrix->elements[0][1] * (matrix->elements[1][2] * matrix->elements[2][3] - matrix->elements[1][3] * matrix->elements[2][2]) -
								matrix->elements[0][2] * (matrix->elements[1][1] * matrix->elements[2][3] - matrix->elements[1][3] * matrix->elements[2][1]) +
								matrix->elements[0][3] * (matrix->elements[1][1] * matrix->elements[2][2] - matrix->elements[1][2] * matrix->elements[2][1])) / det;

	result.elements[3][1] = (matrix->elements[0][0] * (matrix->elements[1][2] * matrix->elements[2][3] - matrix->elements[1][3] * matrix->elements[2][2]) -
								matrix->elements[0][2] * (matrix->elements[1][0] * matrix->elements[2][3] - matrix->elements[1][3] * matrix->elements[2][0]) +
								matrix->elements[0][3] * (matrix->elements[1][0] * matrix->elements[2][2] - matrix->elements[1][2] * matrix->elements[2][0])) / det;

	result.elements[3][2] = -(matrix->elements[0][0] * (matrix->elements[1][1] * matrix->elements[2][3] - matrix->elements[1][3] * matrix->elements[2][1]) -
								matrix->elements[0][1] * (matrix->elements[1][0] * matrix->elements[2][3] - matrix->elements[1][3] * matrix->elements[2][0]) +
								matrix->elements[0][3] * (matrix->elements[1][0] * matrix->elements[2][1] - matrix->elements[1][1] * matrix->elements[2][0])) / det;

	result.elements[3][3] = (matrix->elements[0][0] * (matrix->elements[1][1] * matrix->elements[2][2] - matrix->elements[1][2] * matrix->elements[2][1]) -
								matrix->elements[0][1] * (matrix->elements[1][0] * matrix->elements[2][2] - matrix->elements[1][2] * matrix->elements[2][0]) +
								matrix->elements[0][2] * (matrix->elements[1][0] * matrix->elements[2][1] - matrix->elements[1][1] * matrix->elements[2][0])) / det;

	return result;
}

bool wlf_matrix4x4_equal(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (a->elements[i][j] != b->elements[i][j]) {
				return false;
			}
		}
	}

	return true;
}

bool wlf_matrix4x4_nearly_equal(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b, double epsilon) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (fabs(a->elements[i][j] - b->elements[i][j]) > epsilon) {
				return false;
			}
		}
	}

	return true;
}
