/**
 * @file        wlf_matrix4x4_test.c
 * @brief       Comprehensive test suite for wlf_matrix4x4 functionality.
 * @details     This file provides complete testing coverage for all wlf_matrix4x4
 *              operations including creation, arithmetic, matrix multiplication,
 *              transpose, determinant, inversion, and mathematical properties.
 * @author      Test Suite
 * @date        2024-12-17
 * @version     v1.0
 */

#include "wlf/math/wlf_matrix4x4.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Test configuration
static const double EPSILON = 1e-9;
static int test_count = 0;
static int passed_tests = 0;

// Function prototypes
static void print_test_header(const char* test_name);
static void print_test_summary(void);
static void test_matrix4x4_creation(void);
static void test_matrix4x4_basic_operations(void);
static void test_matrix4x4_arithmetic(void);
static void test_matrix4x4_matrix_multiplication(void);
static void test_matrix4x4_transpose(void);
static void test_matrix4x4_determinant(void);
static void test_matrix4x4_inverse(void);
static void test_matrix4x4_equality(void);
static void test_matrix4x4_edge_cases(void);
static void test_matrix4x4_mathematical_properties(void);
static void test_matrix4x4_string_representation(void);

// Test helper functions
static bool assert_matrix_equal(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b, double epsilon) {
	return wlf_matrix4x4_nearly_equal(a, b, epsilon);
}

// Helper function to compare doubles
static bool assert_double_equal(double a, double b, double epsilon) {
	return fabs(a - b) <= epsilon;
}

static void print_test_header(const char* test_name) {
	printf("\n=== %s ===\n", test_name);
}

static void print_test_summary(void) {
	printf("\n==================================================\n");
	printf("Test Summary: %d/%d tests passed\n", passed_tests, test_count);
	if (passed_tests == test_count) {
		printf("All tests PASSED! ✓\n");
	} else {
		printf("%d tests FAILED! ✗\n", test_count - passed_tests);
	}
	printf("==================================================\n");
}

static void test_matrix4x4_creation(void) {
	print_test_header("Matrix4x4 Creation Tests");

	// Test zero matrix creation
	test_count++;
	struct wlf_matrix4x4 zero_matrix = wlf_matrix4x4_create_zero();
	bool zero_correct = true;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (zero_matrix.elements[i][j] != 0.0) {
				zero_correct = false;
				break;
			}
		}
		if (!zero_correct) break;
	}
	if (zero_correct) {
		printf("✓ Zero matrix creation test passed\n");
		passed_tests++;
	} else {
		printf("✗ Zero matrix creation test failed\n");
	}

	// Test identity matrix creation
	test_count++;
	struct wlf_matrix4x4 identity = wlf_matrix4x4_identity();
	bool identity_correct = true;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			double expected = (i == j) ? 1.0 : 0.0;
			if (identity.elements[i][j] != expected) {
				identity_correct = false;
				break;
			}
		}
		if (!identity_correct) break;
	}
	if (identity_correct) {
		printf("✓ Identity matrix creation test passed\n");
		passed_tests++;
	} else {
		printf("✗ Identity matrix creation test failed\n");
	}
}

static void test_matrix4x4_basic_operations(void) {
	print_test_header("Matrix4x4 Basic Operations Tests");

	// Test get/set operations
	test_count++;
	struct wlf_matrix4x4 test_matrix = wlf_matrix4x4_create_zero();
	wlf_matrix4x4_set(&test_matrix, 0, 0, 1.5);
	wlf_matrix4x4_set(&test_matrix, 1, 1, 2.5);
	wlf_matrix4x4_set(&test_matrix, 2, 2, 3.5);
	wlf_matrix4x4_set(&test_matrix, 3, 3, 4.5);

	bool get_set_correct = true;
	if (!assert_double_equal(wlf_matrix4x4_get(&test_matrix, 0, 0), 1.5, EPSILON) ||
		!assert_double_equal(wlf_matrix4x4_get(&test_matrix, 1, 1), 2.5, EPSILON) ||
		!assert_double_equal(wlf_matrix4x4_get(&test_matrix, 2, 2), 3.5, EPSILON) ||
		!assert_double_equal(wlf_matrix4x4_get(&test_matrix, 3, 3), 4.5, EPSILON)) {
		get_set_correct = false;
	}

	if (get_set_correct) {
		printf("✓ Get/Set operations test passed\n");
		passed_tests++;
	} else {
		printf("✗ Get/Set operations test failed\n");
	}
}

static void test_matrix4x4_arithmetic(void) {
	print_test_header("Matrix4x4 Arithmetic Tests");

	// Create test matrices
	struct wlf_matrix4x4 matrix_a = wlf_matrix4x4_create_zero();
	struct wlf_matrix4x4 matrix_b = wlf_matrix4x4_create_zero();

	// Initialize matrix A
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			matrix_a.elements[i][j] = i * 4 + j + 1; // 1,2,3,4; 5,6,7,8; 9,10,11,12; 13,14,15,16
		}
	}

	// Initialize matrix B
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			matrix_b.elements[i][j] = (i + 1) * (j + 1); // 1,2,3,4; 2,4,6,8; 3,6,9,12; 4,8,12,16
		}
	}

	// Test addition
	test_count++;
	struct wlf_matrix4x4 sum = wlf_matrix4x4_add(&matrix_a, &matrix_b);
	struct wlf_matrix4x4 expected_sum = wlf_matrix4x4_create_zero();
	expected_sum.elements[0][0] = 2; expected_sum.elements[0][1] = 4; expected_sum.elements[0][2] = 6; expected_sum.elements[0][3] = 8;
	expected_sum.elements[1][0] = 7; expected_sum.elements[1][1] = 10; expected_sum.elements[1][2] = 13; expected_sum.elements[1][3] = 16;
	expected_sum.elements[2][0] = 12; expected_sum.elements[2][1] = 16; expected_sum.elements[2][2] = 20; expected_sum.elements[2][3] = 24;
	expected_sum.elements[3][0] = 17; expected_sum.elements[3][1] = 22; expected_sum.elements[3][2] = 27; expected_sum.elements[3][3] = 32;

	if (assert_matrix_equal(&sum, &expected_sum, EPSILON)) {
		printf("✓ Matrix addition test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix addition test failed\n");
	}

	// Test subtraction
	test_count++;
	struct wlf_matrix4x4 diff = wlf_matrix4x4_subtract(&matrix_a, &matrix_b);
	struct wlf_matrix4x4 expected_diff = wlf_matrix4x4_create_zero();
	expected_diff.elements[0][0] = 0; expected_diff.elements[0][1] = 0; expected_diff.elements[0][2] = 0; expected_diff.elements[0][3] = 0;
	expected_diff.elements[1][0] = 3; expected_diff.elements[1][1] = 2; expected_diff.elements[1][2] = 1; expected_diff.elements[1][3] = 0;
	expected_diff.elements[2][0] = 6; expected_diff.elements[2][1] = 4; expected_diff.elements[2][2] = 2; expected_diff.elements[2][3] = 0;
	expected_diff.elements[3][0] = 9; expected_diff.elements[3][1] = 6; expected_diff.elements[3][2] = 3; expected_diff.elements[3][3] = 0;

	if (assert_matrix_equal(&diff, &expected_diff, EPSILON)) {
		printf("✓ Matrix subtraction test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix subtraction test failed\n");
	}

	// Test scalar multiplication
	test_count++;
	struct wlf_matrix4x4 scaled = wlf_matrix4x4_multiply_scalar(&matrix_a, 2.0);
	struct wlf_matrix4x4 expected_scaled = wlf_matrix4x4_create_zero();
	expected_scaled.elements[0][0] = 2; expected_scaled.elements[0][1] = 4; expected_scaled.elements[0][2] = 6; expected_scaled.elements[0][3] = 8;
	expected_scaled.elements[1][0] = 10; expected_scaled.elements[1][1] = 12; expected_scaled.elements[1][2] = 14; expected_scaled.elements[1][3] = 16;
	expected_scaled.elements[2][0] = 18; expected_scaled.elements[2][1] = 20; expected_scaled.elements[2][2] = 22; expected_scaled.elements[2][3] = 24;
	expected_scaled.elements[3][0] = 26; expected_scaled.elements[3][1] = 28; expected_scaled.elements[3][2] = 30; expected_scaled.elements[3][3] = 32;

	if (assert_matrix_equal(&scaled, &expected_scaled, EPSILON)) {
		printf("✓ Scalar multiplication test passed\n");
		passed_tests++;
	} else {
		printf("✗ Scalar multiplication test failed\n");
	}
}

static void test_matrix4x4_matrix_multiplication(void) {
	print_test_header("Matrix4x4 Matrix Multiplication Tests");

	// Test identity matrix multiplication
	test_count++;
	struct wlf_matrix4x4 identity = wlf_matrix4x4_identity();
	struct wlf_matrix4x4 test_matrix = wlf_matrix4x4_create_zero();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			test_matrix.elements[i][j] = i * 4 + j + 1;
		}
	}

	struct wlf_matrix4x4 product1 = wlf_matrix4x4_multiply(&identity, &test_matrix);
	struct wlf_matrix4x4 product2 = wlf_matrix4x4_multiply(&test_matrix, &identity);

	if (assert_matrix_equal(&product1, &test_matrix, EPSILON) &&
		assert_matrix_equal(&product2, &test_matrix, EPSILON)) {
		printf("✓ Identity matrix multiplication test passed\n");
		passed_tests++;
	} else {
		printf("✗ Identity matrix multiplication test failed\n");
	}

	// Test simple matrix multiplication
	test_count++;
	struct wlf_matrix4x4 matrix_a = wlf_matrix4x4_create_zero();
	struct wlf_matrix4x4 matrix_b = wlf_matrix4x4_create_zero();

	// Create simple matrices for multiplication
	matrix_a.elements[0][0] = 1; matrix_a.elements[0][1] = 2; matrix_a.elements[0][2] = 0; matrix_a.elements[0][3] = 0;
	matrix_a.elements[1][0] = 3; matrix_a.elements[1][1] = 4; matrix_a.elements[1][2] = 0; matrix_a.elements[1][3] = 0;
	matrix_a.elements[2][0] = 0; matrix_a.elements[2][1] = 0; matrix_a.elements[2][2] = 1; matrix_a.elements[2][3] = 0;
	matrix_a.elements[3][0] = 0; matrix_a.elements[3][1] = 0; matrix_a.elements[3][2] = 0; matrix_a.elements[3][3] = 1;

	matrix_b.elements[0][0] = 5; matrix_b.elements[0][1] = 6; matrix_b.elements[0][2] = 0; matrix_b.elements[0][3] = 0;
	matrix_b.elements[1][0] = 7; matrix_b.elements[1][1] = 8; matrix_b.elements[1][2] = 0; matrix_b.elements[1][3] = 0;
	matrix_b.elements[2][0] = 0; matrix_b.elements[2][1] = 0; matrix_b.elements[2][2] = 1; matrix_b.elements[2][3] = 0;
	matrix_b.elements[3][0] = 0; matrix_b.elements[3][1] = 0; matrix_b.elements[3][2] = 0; matrix_b.elements[3][3] = 1;

	struct wlf_matrix4x4 product = wlf_matrix4x4_multiply(&matrix_a, &matrix_b);
	struct wlf_matrix4x4 expected_product = wlf_matrix4x4_create_zero();
	expected_product.elements[0][0] = 19; expected_product.elements[0][1] = 22; expected_product.elements[0][2] = 0; expected_product.elements[0][3] = 0;
	expected_product.elements[1][0] = 43; expected_product.elements[1][1] = 50; expected_product.elements[1][2] = 0; expected_product.elements[1][3] = 0;
	expected_product.elements[2][0] = 0; expected_product.elements[2][1] = 0; expected_product.elements[2][2] = 1; expected_product.elements[2][3] = 0;
	expected_product.elements[3][0] = 0; expected_product.elements[3][1] = 0; expected_product.elements[3][2] = 0; expected_product.elements[3][3] = 1;

	if (assert_matrix_equal(&product, &expected_product, EPSILON)) {
		printf("✓ Matrix multiplication test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix multiplication test failed\n");
	}
}

static void test_matrix4x4_transpose(void) {
	print_test_header("Matrix4x4 Transpose Tests");

	// Test identity matrix transpose
	test_count++;
	struct wlf_matrix4x4 identity = wlf_matrix4x4_identity();
	struct wlf_matrix4x4 transposed_identity = wlf_matrix4x4_transpose(&identity);

	if (assert_matrix_equal(&transposed_identity, &identity, EPSILON)) {
		printf("✓ Identity matrix transpose test passed\n");
		passed_tests++;
	} else {
		printf("✗ Identity matrix transpose test failed\n");
	}

	// Test general matrix transpose
	test_count++;
	struct wlf_matrix4x4 matrix = wlf_matrix4x4_create_zero();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			matrix.elements[i][j] = i * 4 + j + 1;
		}
	}

	struct wlf_matrix4x4 transposed = wlf_matrix4x4_transpose(&matrix);
	struct wlf_matrix4x4 expected_transposed = wlf_matrix4x4_create_zero();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			expected_transposed.elements[j][i] = matrix.elements[i][j];
		}
	}

	if (assert_matrix_equal(&transposed, &expected_transposed, EPSILON)) {
		printf("✓ Matrix transpose test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix transpose test failed\n");
	}

	// Test double transpose property (A^T)^T = A
	test_count++;
	struct wlf_matrix4x4 double_transposed = wlf_matrix4x4_transpose(&transposed);

	if (assert_matrix_equal(&double_transposed, &matrix, EPSILON)) {
		printf("✓ Double transpose property test passed\n");
		passed_tests++;
	} else {
		printf("✗ Double transpose property test failed\n");
	}
}

static void test_matrix4x4_determinant(void) {
	print_test_header("Matrix4x4 Determinant Tests");

	// Test identity matrix determinant
	test_count++;
	struct wlf_matrix4x4 identity = wlf_matrix4x4_identity();
	double det_identity = wlf_matrix4x4_determinant(&identity);

	if (assert_double_equal(det_identity, 1.0, EPSILON)) {
		printf("✓ Identity matrix determinant test passed\n");
		passed_tests++;
	} else {
		printf("✗ Identity matrix determinant test failed: expected 1.0, got %f\n", det_identity);
	}

	// Test diagonal matrix determinant
	test_count++;
	struct wlf_matrix4x4 diagonal = wlf_matrix4x4_create_zero();
	diagonal.elements[0][0] = 2; diagonal.elements[1][1] = 3;
	diagonal.elements[2][2] = 4; diagonal.elements[3][3] = 5;

	double det_diagonal = wlf_matrix4x4_determinant(&diagonal);
	double expected_det = 2 * 3 * 4 * 5; // 120

	if (assert_double_equal(det_diagonal, expected_det, EPSILON)) {
		printf("✓ Diagonal matrix determinant test passed\n");
		passed_tests++;
	} else {
		printf("✗ Diagonal matrix determinant test failed: expected %f, got %f\n", expected_det, det_diagonal);
	}

	// Test zero determinant (singular matrix)
	test_count++;
	struct wlf_matrix4x4 singular = wlf_matrix4x4_create_zero();
	singular.elements[0][0] = 1; singular.elements[0][1] = 2; singular.elements[0][2] = 3; singular.elements[0][3] = 4;
	singular.elements[1][0] = 2; singular.elements[1][1] = 4; singular.elements[1][2] = 6; singular.elements[1][3] = 8;
	singular.elements[2][0] = 3; singular.elements[2][1] = 6; singular.elements[2][2] = 9; singular.elements[2][3] = 12;
	singular.elements[3][0] = 4; singular.elements[3][1] = 8; singular.elements[3][2] = 12; singular.elements[3][3] = 16;

	double det_singular = wlf_matrix4x4_determinant(&singular);

	if (assert_double_equal(det_singular, 0.0, EPSILON)) {
		printf("✓ Singular matrix determinant test passed\n");
		passed_tests++;
	} else {
		printf("✗ Singular matrix determinant test failed: expected 0.0, got %f\n", det_singular);
	}
}

static void test_matrix4x4_inverse(void) {
	print_test_header("Matrix4x4 Inverse Tests");

	// Test identity matrix inverse
	test_count++;
	struct wlf_matrix4x4 identity = wlf_matrix4x4_identity();
	struct wlf_matrix4x4 inv_identity = wlf_matrix4x4_inverse(&identity);

	if (assert_matrix_equal(&inv_identity, &identity, EPSILON)) {
		printf("✓ Identity matrix inverse test passed\n");
		passed_tests++;
	} else {
		printf("✗ Identity matrix inverse test failed\n");
	}

	// Test diagonal matrix inverse
	test_count++;
	struct wlf_matrix4x4 diagonal = wlf_matrix4x4_create_zero();
	diagonal.elements[0][0] = 2; diagonal.elements[1][1] = 4;
	diagonal.elements[2][2] = 8; diagonal.elements[3][3] = 16;

	struct wlf_matrix4x4 inv_diagonal = wlf_matrix4x4_inverse(&diagonal);
	struct wlf_matrix4x4 expected_inv_diagonal = wlf_matrix4x4_create_zero();
	expected_inv_diagonal.elements[0][0] = 0.5; expected_inv_diagonal.elements[1][1] = 0.25;
	expected_inv_diagonal.elements[2][2] = 0.125; expected_inv_diagonal.elements[3][3] = 0.0625;

	if (assert_matrix_equal(&inv_diagonal, &expected_inv_diagonal, EPSILON)) {
		printf("✓ Diagonal matrix inverse test passed\n");
		passed_tests++;
	} else {
		printf("✗ Diagonal matrix inverse test failed\n");
	}

	// Test A * A^-1 = I
	test_count++;
	struct wlf_matrix4x4 product = wlf_matrix4x4_multiply(&diagonal, &inv_diagonal);

	if (assert_matrix_equal(&product, &identity, EPSILON)) {
		printf("✓ Matrix inverse property test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix inverse property test failed\n");
	}

	// Test singular matrix inverse (should return zero matrix)
	test_count++;
	struct wlf_matrix4x4 singular = wlf_matrix4x4_create_zero();
	singular.elements[0][0] = 1; singular.elements[0][1] = 2; singular.elements[0][2] = 3; singular.elements[0][3] = 4;
	singular.elements[1][0] = 2; singular.elements[1][1] = 4; singular.elements[1][2] = 6; singular.elements[1][3] = 8;
	singular.elements[2][0] = 3; singular.elements[2][1] = 6; singular.elements[2][2] = 9; singular.elements[2][3] = 12;
	singular.elements[3][0] = 4; singular.elements[3][1] = 8; singular.elements[3][2] = 12; singular.elements[3][3] = 16;

	struct wlf_matrix4x4 inv_singular = wlf_matrix4x4_inverse(&singular);
	struct wlf_matrix4x4 zero_matrix = wlf_matrix4x4_create_zero();

	if (assert_matrix_equal(&inv_singular, &zero_matrix, EPSILON)) {
		printf("✓ Singular matrix inverse test passed\n");
		passed_tests++;
	} else {
		printf("✗ Singular matrix inverse test failed\n");
	}
}

static void test_matrix4x4_equality(void) {
	print_test_header("Matrix4x4 Equality Tests");

	// Test exact equality
	test_count++;
	struct wlf_matrix4x4 matrix1 = wlf_matrix4x4_identity();
	struct wlf_matrix4x4 matrix2 = wlf_matrix4x4_identity();

	if (wlf_matrix4x4_equal(&matrix1, &matrix2)) {
		printf("✓ Exact equality test passed\n");
		passed_tests++;
	} else {
		printf("✗ Exact equality test failed\n");
	}

	// Test inequality
	test_count++;
	struct wlf_matrix4x4 matrix3 = wlf_matrix4x4_create_zero();
	matrix3.elements[0][0] = 1.0001;

	if (!wlf_matrix4x4_equal(&matrix1, &matrix3)) {
		printf("✓ Inequality test passed\n");
		passed_tests++;
	} else {
		printf("✗ Inequality test failed\n");
	}

	// Test nearly equal
	test_count++;
	struct wlf_matrix4x4 matrix4 = wlf_matrix4x4_identity();
	matrix4.elements[0][0] = 1.0 + EPSILON/2;

	if (wlf_matrix4x4_nearly_equal(&matrix1, &matrix4, EPSILON)) {
		printf("✓ Nearly equal test passed\n");
		passed_tests++;
	} else {
		printf("✗ Nearly equal test failed\n");
	}

	// Test not nearly equal
	test_count++;
	struct wlf_matrix4x4 matrix5 = wlf_matrix4x4_identity();
	matrix5.elements[0][0] = 1.0 + EPSILON * 2;

	if (!wlf_matrix4x4_nearly_equal(&matrix1, &matrix5, EPSILON)) {
		printf("✓ Not nearly equal test passed\n");
		passed_tests++;
	} else {
		printf("✗ Not nearly equal test failed\n");
	}
}

static void test_matrix4x4_edge_cases(void) {
	print_test_header("Matrix4x4 Edge Cases Tests");

	// Test zero matrix operations
	test_count++;
	struct wlf_matrix4x4 zero_matrix = wlf_matrix4x4_create_zero();
	struct wlf_matrix4x4 identity = wlf_matrix4x4_identity();

	struct wlf_matrix4x4 zero_sum = wlf_matrix4x4_add(&zero_matrix, &identity);
	struct wlf_matrix4x4 zero_diff = wlf_matrix4x4_subtract(&identity, &identity);
	struct wlf_matrix4x4 zero_product = wlf_matrix4x4_multiply(&zero_matrix, &identity);

	if (assert_matrix_equal(&zero_sum, &identity, EPSILON) &&
		assert_matrix_equal(&zero_diff, &zero_matrix, EPSILON) &&
		assert_matrix_equal(&zero_product, &zero_matrix, EPSILON)) {
		printf("✓ Zero matrix operations test passed\n");
		passed_tests++;
	} else {
		printf("✗ Zero matrix operations test failed\n");
	}

	// Test scalar zero multiplication
	test_count++;
	struct wlf_matrix4x4 scaled_zero = wlf_matrix4x4_multiply_scalar(&identity, 0.0);

	if (assert_matrix_equal(&scaled_zero, &zero_matrix, EPSILON)) {
		printf("✓ Scalar zero multiplication test passed\n");
		passed_tests++;
	} else {
		printf("✗ Scalar zero multiplication test failed\n");
	}

	// Test negative scalar multiplication
	test_count++;
	struct wlf_matrix4x4 scaled_negative = wlf_matrix4x4_multiply_scalar(&identity, -1.0);
	struct wlf_matrix4x4 expected_negative = wlf_matrix4x4_create_zero();
	expected_negative.elements[0][0] = -1; expected_negative.elements[1][1] = -1;
	expected_negative.elements[2][2] = -1; expected_negative.elements[3][3] = -1;

	if (assert_matrix_equal(&scaled_negative, &expected_negative, EPSILON)) {
		printf("✓ Negative scalar multiplication test passed\n");
		passed_tests++;
	} else {
		printf("✗ Negative scalar multiplication test failed\n");
	}
}

static void test_matrix4x4_mathematical_properties(void) {
	print_test_header("Matrix4x4 Mathematical Properties Tests");

	// Create test matrices
	struct wlf_matrix4x4 matrix_a = wlf_matrix4x4_create_zero();
	struct wlf_matrix4x4 matrix_b = wlf_matrix4x4_create_zero();
	struct wlf_matrix4x4 matrix_c = wlf_matrix4x4_create_zero();

	// Initialize matrices with different values
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			matrix_a.elements[i][j] = i + j + 1;
			matrix_b.elements[i][j] = (i + 1) * (j + 1);
			matrix_c.elements[i][j] = (i - j) + 3;
		}
	}

	// Test commutativity of addition: A + B = B + A
	test_count++;
	struct wlf_matrix4x4 sum_ab = wlf_matrix4x4_add(&matrix_a, &matrix_b);
	struct wlf_matrix4x4 sum_ba = wlf_matrix4x4_add(&matrix_b, &matrix_a);

	if (assert_matrix_equal(&sum_ab, &sum_ba, EPSILON)) {
		printf("✓ Addition commutativity test passed\n");
		passed_tests++;
	} else {
		printf("✗ Addition commutativity test failed\n");
	}

	// Test associativity of addition: (A + B) + C = A + (B + C)
	test_count++;
	struct wlf_matrix4x4 sum_ab_c = wlf_matrix4x4_add(&sum_ab, &matrix_c);
	struct wlf_matrix4x4 sum_bc = wlf_matrix4x4_add(&matrix_b, &matrix_c);
	struct wlf_matrix4x4 sum_a_bc = wlf_matrix4x4_add(&matrix_a, &sum_bc);

	if (assert_matrix_equal(&sum_ab_c, &sum_a_bc, EPSILON)) {
		printf("✓ Addition associativity test passed\n");
		passed_tests++;
	} else {
		printf("✗ Addition associativity test failed\n");
	}

	// Test distributivity of scalar multiplication: k(A + B) = kA + kB
	test_count++;
	double scalar = 2.5;
	struct wlf_matrix4x4 scaled_sum = wlf_matrix4x4_multiply_scalar(&sum_ab, scalar);
	struct wlf_matrix4x4 scaled_a = wlf_matrix4x4_multiply_scalar(&matrix_a, scalar);
	struct wlf_matrix4x4 scaled_b = wlf_matrix4x4_multiply_scalar(&matrix_b, scalar);
	struct wlf_matrix4x4 sum_scaled = wlf_matrix4x4_add(&scaled_a, &scaled_b);

	if (assert_matrix_equal(&scaled_sum, &sum_scaled, EPSILON)) {
		printf("✓ Scalar multiplication distributivity test passed\n");
		passed_tests++;
	} else {
		printf("✗ Scalar multiplication distributivity test failed\n");
	}

	// Test transpose property: (A + B)^T = A^T + B^T
	test_count++;
	struct wlf_matrix4x4 transpose_sum = wlf_matrix4x4_transpose(&sum_ab);
	struct wlf_matrix4x4 transpose_a = wlf_matrix4x4_transpose(&matrix_a);
	struct wlf_matrix4x4 transpose_b = wlf_matrix4x4_transpose(&matrix_b);
	struct wlf_matrix4x4 sum_transposes = wlf_matrix4x4_add(&transpose_a, &transpose_b);

	if (assert_matrix_equal(&transpose_sum, &sum_transposes, EPSILON)) {
		printf("✓ Transpose addition property test passed\n");
		passed_tests++;
	} else {
		printf("✗ Transpose addition property test failed\n");
	}
}

static void test_matrix4x4_string_representation(void) {
	print_test_header("Matrix4x4 String Representation Tests");

	// Test string representation
	test_count++;
	struct wlf_matrix4x4 matrix = wlf_matrix4x4_identity();
	char* str_repr = wlf_matrix4x4_to_str(&matrix);

	bool string_valid = (str_repr != NULL && strlen(str_repr) > 0);

	if (string_valid) {
		printf("✓ String representation test passed\n");
		printf("  Matrix string: %s\n", str_repr);
		passed_tests++;
	} else {
		printf("✗ String representation test failed\n");
	}

	if (str_repr) {
		free(str_repr);
	}
}

int main(void) {
	printf("Starting comprehensive wlf_matrix4x4 test suite...\n");

	test_matrix4x4_creation();
	test_matrix4x4_basic_operations();
	test_matrix4x4_arithmetic();
	test_matrix4x4_matrix_multiplication();
	test_matrix4x4_transpose();
	test_matrix4x4_determinant();
	test_matrix4x4_inverse();
	test_matrix4x4_equality();
	test_matrix4x4_mathematical_properties();
	test_matrix4x4_edge_cases();
	test_matrix4x4_string_representation();

	print_test_summary();

	return (passed_tests == test_count) ? 0 : 1;
}
