/**
 * @file        wlf_matrix3x3_test.c
 * @brief       Comprehensive test suite for wlf_matrix3x3 functionality.
 * @details     This file provides complete testing coverage for all wlf_matrix3x3
 *              operations including creation, arithmetic, matrix multiplication,
 *              transpose, determinant, inversion, and mathematical properties.
 * @author      Test Suite
 * @date        2024-12-17
 * @version     v1.0
 */

#include "wlf/math/wlf_matrix3x3.h"

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
static void test_matrix3x3_creation(void);
static void test_matrix3x3_basic_operations(void);
static void test_matrix3x3_arithmetic(void);
static void test_matrix3x3_matrix_multiplication(void);
static void test_matrix3x3_transpose(void);
static void test_matrix3x3_determinant(void);
static void test_matrix3x3_inverse(void);
static void test_matrix3x3_equality(void);
static void test_matrix3x3_edge_cases(void);
static void test_matrix3x3_mathematical_properties(void);
static void test_matrix3x3_string_representation(void);

// Test helper functions
static bool assert_matrix_equal(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b, double epsilon) {
	return wlf_matrix3x3_nearly_equal(a, b, epsilon);
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

static void test_matrix3x3_creation(void) {
	print_test_header("Matrix3x3 Creation Tests");

	// Test zero matrix creation
	test_count++;
	struct wlf_matrix3x3 zero_matrix = wlf_matrix3x3_create_zero();
	bool zero_correct = true;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
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
	struct wlf_matrix3x3 identity = wlf_matrix3x3_identity();
	bool identity_correct = true;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
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

static void test_matrix3x3_basic_operations(void) {
	print_test_header("Matrix3x3 Basic Operations Tests");

	// Test get/set operations
	test_count++;
	struct wlf_matrix3x3 test_matrix = wlf_matrix3x3_create_zero();
	wlf_matrix3x3_set(&test_matrix, 0, 0, 1.5);
	wlf_matrix3x3_set(&test_matrix, 1, 1, 2.5);
	wlf_matrix3x3_set(&test_matrix, 2, 2, 3.5);

	bool get_set_correct = true;
	if (!assert_double_equal(wlf_matrix3x3_get(&test_matrix, 0, 0), 1.5, EPSILON) ||
		!assert_double_equal(wlf_matrix3x3_get(&test_matrix, 1, 1), 2.5, EPSILON) ||
		!assert_double_equal(wlf_matrix3x3_get(&test_matrix, 2, 2), 3.5, EPSILON)) {
		get_set_correct = false;
	}

	if (get_set_correct) {
		printf("✓ Get/Set operations test passed\n");
		passed_tests++;
	} else {
		printf("✗ Get/Set operations test failed\n");
	}
}

static void test_matrix3x3_arithmetic(void) {
	print_test_header("Matrix3x3 Arithmetic Tests");

	// Create test matrices
	struct wlf_matrix3x3 matrix_a = wlf_matrix3x3_create_zero();
	struct wlf_matrix3x3 matrix_b = wlf_matrix3x3_create_zero();

	// Initialize matrix A
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			matrix_a.elements[i][j] = i * 3 + j + 1; // 1,2,3; 4,5,6; 7,8,9
		}
	}

	// Initialize matrix B
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			matrix_b.elements[i][j] = (i + 1) * (j + 1); // 1,2,3; 2,4,6; 3,6,9
		}
	}

	// Test addition
	test_count++;
	struct wlf_matrix3x3 sum = wlf_matrix3x3_add(&matrix_a, &matrix_b);
	struct wlf_matrix3x3 expected_sum = wlf_matrix3x3_create_zero();
	expected_sum.elements[0][0] = 2; expected_sum.elements[0][1] = 4; expected_sum.elements[0][2] = 6;
	expected_sum.elements[1][0] = 6; expected_sum.elements[1][1] = 9; expected_sum.elements[1][2] = 12;
	expected_sum.elements[2][0] = 10; expected_sum.elements[2][1] = 14; expected_sum.elements[2][2] = 18;

	if (assert_matrix_equal(&sum, &expected_sum, EPSILON)) {
		printf("✓ Matrix addition test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix addition test failed\n");
	}

	// Test subtraction
	test_count++;
	struct wlf_matrix3x3 diff = wlf_matrix3x3_subtract(&matrix_a, &matrix_b);
	struct wlf_matrix3x3 expected_diff = wlf_matrix3x3_create_zero();
	expected_diff.elements[0][0] = 0; expected_diff.elements[0][1] = 0; expected_diff.elements[0][2] = 0;
	expected_diff.elements[1][0] = 2; expected_diff.elements[1][1] = 1; expected_diff.elements[1][2] = 0;
	expected_diff.elements[2][0] = 4; expected_diff.elements[2][1] = 2; expected_diff.elements[2][2] = 0;

	if (assert_matrix_equal(&diff, &expected_diff, EPSILON)) {
		printf("✓ Matrix subtraction test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix subtraction test failed\n");
	}

	// Test scalar multiplication
	test_count++;
	struct wlf_matrix3x3 scaled = wlf_matrix3x3_multiply_scalar(&matrix_a, 2.0);
	struct wlf_matrix3x3 expected_scaled = wlf_matrix3x3_create_zero();
	expected_scaled.elements[0][0] = 2; expected_scaled.elements[0][1] = 4; expected_scaled.elements[0][2] = 6;
	expected_scaled.elements[1][0] = 8; expected_scaled.elements[1][1] = 10; expected_scaled.elements[1][2] = 12;
	expected_scaled.elements[2][0] = 14; expected_scaled.elements[2][1] = 16; expected_scaled.elements[2][2] = 18;

	if (assert_matrix_equal(&scaled, &expected_scaled, EPSILON)) {
		printf("✓ Scalar multiplication test passed\n");
		passed_tests++;
	} else {
		printf("✗ Scalar multiplication test failed\n");
	}
}

static void test_matrix3x3_matrix_multiplication(void) {
	print_test_header("Matrix3x3 Matrix Multiplication Tests");

	// Test identity multiplication
	test_count++;
	struct wlf_matrix3x3 identity = wlf_matrix3x3_identity();
	struct wlf_matrix3x3 test_matrix = wlf_matrix3x3_create_zero();
	test_matrix.elements[0][0] = 1; test_matrix.elements[0][1] = 2; test_matrix.elements[0][2] = 3;
	test_matrix.elements[1][0] = 4; test_matrix.elements[1][1] = 5; test_matrix.elements[1][2] = 6;
	test_matrix.elements[2][0] = 7; test_matrix.elements[2][1] = 8; test_matrix.elements[2][2] = 9;

	struct wlf_matrix3x3 result = wlf_matrix3x3_multiply(&identity, &test_matrix);

	if (assert_matrix_equal(&result, &test_matrix, EPSILON)) {
		printf("✓ Identity multiplication test passed\n");
		passed_tests++;
	} else {
		printf("✗ Identity multiplication test failed\n");
	}

	// Test specific matrix multiplication
	test_count++;
	struct wlf_matrix3x3 matrix_a = wlf_matrix3x3_create_zero();
	struct wlf_matrix3x3 matrix_b = wlf_matrix3x3_create_zero();

	// A = [[1,2,3], [4,5,6], [7,8,9]]
	matrix_a.elements[0][0] = 1; matrix_a.elements[0][1] = 2; matrix_a.elements[0][2] = 3;
	matrix_a.elements[1][0] = 4; matrix_a.elements[1][1] = 5; matrix_a.elements[1][2] = 6;
	matrix_a.elements[2][0] = 7; matrix_a.elements[2][1] = 8; matrix_a.elements[2][2] = 9;

	// B = [[1,0,0], [0,1,0], [0,0,2]]
	matrix_b.elements[0][0] = 1; matrix_b.elements[0][1] = 0; matrix_b.elements[0][2] = 0;
	matrix_b.elements[1][0] = 0; matrix_b.elements[1][1] = 1; matrix_b.elements[1][2] = 0;
	matrix_b.elements[2][0] = 0; matrix_b.elements[2][1] = 0; matrix_b.elements[2][2] = 2;

	struct wlf_matrix3x3 product = wlf_matrix3x3_multiply(&matrix_a, &matrix_b);
	struct wlf_matrix3x3 expected_product = wlf_matrix3x3_create_zero();
	expected_product.elements[0][0] = 1; expected_product.elements[0][1] = 2; expected_product.elements[0][2] = 6;
	expected_product.elements[1][0] = 4; expected_product.elements[1][1] = 5; expected_product.elements[1][2] = 12;
	expected_product.elements[2][0] = 7; expected_product.elements[2][1] = 8; expected_product.elements[2][2] = 18;

	if (assert_matrix_equal(&product, &expected_product, EPSILON)) {
		printf("✓ Matrix multiplication test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix multiplication test failed\n");
	}
}

static void test_matrix3x3_transpose(void) {
	print_test_header("Matrix3x3 Transpose Tests");

	// Test transpose
	test_count++;
	struct wlf_matrix3x3 matrix = wlf_matrix3x3_create_zero();
	matrix.elements[0][0] = 1; matrix.elements[0][1] = 2; matrix.elements[0][2] = 3;
	matrix.elements[1][0] = 4; matrix.elements[1][1] = 5; matrix.elements[1][2] = 6;
	matrix.elements[2][0] = 7; matrix.elements[2][1] = 8; matrix.elements[2][2] = 9;

	struct wlf_matrix3x3 transposed = wlf_matrix3x3_transpose(&matrix);
	struct wlf_matrix3x3 expected_transpose = wlf_matrix3x3_create_zero();
	expected_transpose.elements[0][0] = 1; expected_transpose.elements[0][1] = 4; expected_transpose.elements[0][2] = 7;
	expected_transpose.elements[1][0] = 2; expected_transpose.elements[1][1] = 5; expected_transpose.elements[1][2] = 8;
	expected_transpose.elements[2][0] = 3; expected_transpose.elements[2][1] = 6; expected_transpose.elements[2][2] = 9;

	if (assert_matrix_equal(&transposed, &expected_transpose, EPSILON)) {
		printf("✓ Matrix transpose test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix transpose test failed\n");
	}

	// Test double transpose property: (A^T)^T = A
	test_count++;
	struct wlf_matrix3x3 double_transposed = wlf_matrix3x3_transpose(&transposed);

	if (assert_matrix_equal(&double_transposed, &matrix, EPSILON)) {
		printf("✓ Double transpose property test passed\n");
		passed_tests++;
	} else {
		printf("✗ Double transpose property test failed\n");
	}
}

static void test_matrix3x3_determinant(void) {
	print_test_header("Matrix3x3 Determinant Tests");

	// Test identity matrix determinant
	test_count++;
	struct wlf_matrix3x3 identity = wlf_matrix3x3_identity();
	double det_identity = wlf_matrix3x3_determinant(&identity);

	if (assert_double_equal(det_identity, 1.0, EPSILON)) {
		printf("✓ Identity matrix determinant test passed\n");
		passed_tests++;
	} else {
		printf("✗ Identity matrix determinant test failed: expected 1.0, got %f\n", det_identity);
	}

	// Test specific matrix determinant
	test_count++;
	struct wlf_matrix3x3 matrix = wlf_matrix3x3_create_zero();
	matrix.elements[0][0] = 1; matrix.elements[0][1] = 2; matrix.elements[0][2] = 3;
	matrix.elements[1][0] = 0; matrix.elements[1][1] = 1; matrix.elements[1][2] = 4;
	matrix.elements[2][0] = 5; matrix.elements[2][1] = 6; matrix.elements[2][2] = 0;

	double det = wlf_matrix3x3_determinant(&matrix);
	// Expected determinant: 1*(1*0 - 4*6) - 2*(0*0 - 4*5) + 3*(0*6 - 1*5)
	//                    = 1*(-24) - 2*(-20) + 3*(-5) = -24 + 40 - 15 = 1

	if (assert_double_equal(det, 1.0, EPSILON)) {
		printf("✓ Matrix determinant test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix determinant test failed: expected 1.0, got %f\n", det);
	}

	// Test zero determinant (singular matrix)
	test_count++;
	struct wlf_matrix3x3 singular = wlf_matrix3x3_create_zero();
	singular.elements[0][0] = 1; singular.elements[0][1] = 2; singular.elements[0][2] = 3;
	singular.elements[1][0] = 2; singular.elements[1][1] = 4; singular.elements[1][2] = 6;
	singular.elements[2][0] = 3; singular.elements[2][1] = 6; singular.elements[2][2] = 9;

	double det_singular = wlf_matrix3x3_determinant(&singular);

	if (assert_double_equal(det_singular, 0.0, EPSILON)) {
		printf("✓ Singular matrix determinant test passed\n");
		passed_tests++;
	} else {
		printf("✗ Singular matrix determinant test failed: expected 0.0, got %f\n", det_singular);
	}
}

static void test_matrix3x3_inverse(void) {
	print_test_header("Matrix3x3 Inverse Tests");

	// Test identity matrix inverse
	test_count++;
	struct wlf_matrix3x3 identity = wlf_matrix3x3_identity();
	struct wlf_matrix3x3 inv_identity = wlf_matrix3x3_inverse(&identity);

	if (assert_matrix_equal(&inv_identity, &identity, EPSILON)) {
		printf("✓ Identity matrix inverse test passed\n");
		passed_tests++;
	} else {
		printf("✗ Identity matrix inverse test failed\n");
	}

	// Test specific matrix inverse
	test_count++;
	struct wlf_matrix3x3 matrix = wlf_matrix3x3_create_zero();
	matrix.elements[0][0] = 1; matrix.elements[0][1] = 0; matrix.elements[0][2] = 0;
	matrix.elements[1][0] = 0; matrix.elements[1][1] = 2; matrix.elements[1][2] = 0;
	matrix.elements[2][0] = 0; matrix.elements[2][1] = 0; matrix.elements[2][2] = 3;

	struct wlf_matrix3x3 inverse = wlf_matrix3x3_inverse(&matrix);
	struct wlf_matrix3x3 expected_inverse = wlf_matrix3x3_create_zero();
	expected_inverse.elements[0][0] = 1; expected_inverse.elements[0][1] = 0; expected_inverse.elements[0][2] = 0;
	expected_inverse.elements[1][0] = 0; expected_inverse.elements[1][1] = 0.5; expected_inverse.elements[1][2] = 0;
	expected_inverse.elements[2][0] = 0; expected_inverse.elements[2][1] = 0; expected_inverse.elements[2][2] = 1.0/3.0;

	if (assert_matrix_equal(&inverse, &expected_inverse, EPSILON)) {
		printf("✓ Matrix inverse test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix inverse test failed\n");
	}

	// Test A * A^-1 = I
	test_count++;
	struct wlf_matrix3x3 product = wlf_matrix3x3_multiply(&matrix, &inverse);

	if (assert_matrix_equal(&product, &identity, EPSILON)) {
		printf("✓ Matrix inverse property test passed\n");
		passed_tests++;
	} else {
		printf("✗ Matrix inverse property test failed\n");
	}

	// Test singular matrix inverse (should return zero matrix)
	test_count++;
	struct wlf_matrix3x3 singular = wlf_matrix3x3_create_zero();
	singular.elements[0][0] = 1; singular.elements[0][1] = 2; singular.elements[0][2] = 3;
	singular.elements[1][0] = 2; singular.elements[1][1] = 4; singular.elements[1][2] = 6;
	singular.elements[2][0] = 3; singular.elements[2][1] = 6; singular.elements[2][2] = 9;

	struct wlf_matrix3x3 inv_singular = wlf_matrix3x3_inverse(&singular);
	struct wlf_matrix3x3 zero_matrix = wlf_matrix3x3_create_zero();

	if (assert_matrix_equal(&inv_singular, &zero_matrix, EPSILON)) {
		printf("✓ Singular matrix inverse test passed\n");
		passed_tests++;
	} else {
		printf("✗ Singular matrix inverse test failed\n");
	}
}

static void test_matrix3x3_equality(void) {
	print_test_header("Matrix3x3 Equality Tests");

	// Test exact equality
	test_count++;
	struct wlf_matrix3x3 matrix_a = wlf_matrix3x3_identity();
	struct wlf_matrix3x3 matrix_b = wlf_matrix3x3_identity();

	if (wlf_matrix3x3_equal(&matrix_a, &matrix_b)) {
		printf("✓ Exact equality test passed\n");
		passed_tests++;
	} else {
		printf("✗ Exact equality test failed\n");
	}

	// Test inequality
	test_count++;
	matrix_b.elements[0][0] = 2.0;

	if (!wlf_matrix3x3_equal(&matrix_a, &matrix_b)) {
		printf("✓ Inequality test passed\n");
		passed_tests++;
	} else {
		printf("✗ Inequality test failed\n");
	}

	// Test nearly equal
	test_count++;
	matrix_b.elements[0][0] = 1.0 + EPSILON / 2;

	if (wlf_matrix3x3_nearly_equal(&matrix_a, &matrix_b, EPSILON)) {
		printf("✓ Nearly equal test passed\n");
		passed_tests++;
	} else {
		printf("✗ Nearly equal test failed\n");
	}

	// Test not nearly equal
	test_count++;
	matrix_b.elements[0][0] = 1.0 + EPSILON * 2;

	if (!wlf_matrix3x3_nearly_equal(&matrix_a, &matrix_b, EPSILON)) {
		printf("✓ Not nearly equal test passed\n");
		passed_tests++;
	} else {
		printf("✗ Not nearly equal test failed\n");
	}
}

static void test_matrix3x3_mathematical_properties(void) {
	print_test_header("Matrix3x3 Mathematical Properties Tests");

	// Create test matrices
	struct wlf_matrix3x3 a = wlf_matrix3x3_create_zero();
	struct wlf_matrix3x3 b = wlf_matrix3x3_create_zero();
	struct wlf_matrix3x3 c = wlf_matrix3x3_create_zero();

	// Initialize matrices with non-trivial values
	a.elements[0][0] = 1; a.elements[0][1] = 2; a.elements[0][2] = 0;
	a.elements[1][0] = 3; a.elements[1][1] = 1; a.elements[1][2] = 1;
	a.elements[2][0] = 0; a.elements[2][1] = 1; a.elements[2][2] = 2;

	b.elements[0][0] = 2; b.elements[0][1] = 1; b.elements[0][2] = 1;
	b.elements[1][0] = 0; b.elements[1][1] = 1; b.elements[1][2] = 0;
	b.elements[2][0] = 1; b.elements[2][1] = 0; b.elements[2][2] = 1;

	c.elements[0][0] = 1; c.elements[0][1] = 0; c.elements[0][2] = 2;
	c.elements[1][0] = 0; c.elements[1][1] = 2; c.elements[1][2] = 1;
	c.elements[2][0] = 1; c.elements[2][1] = 1; c.elements[2][2] = 0;

	// Test commutative property of addition: A + B = B + A
	test_count++;
	struct wlf_matrix3x3 ab_sum = wlf_matrix3x3_add(&a, &b);
	struct wlf_matrix3x3 ba_sum = wlf_matrix3x3_add(&b, &a);

	if (assert_matrix_equal(&ab_sum, &ba_sum, EPSILON)) {
		printf("✓ Addition commutativity test passed\n");
		passed_tests++;
	} else {
		printf("✗ Addition commutativity test failed\n");
	}

	// Test associative property of addition: (A + B) + C = A + (B + C)
	test_count++;
	struct wlf_matrix3x3 ab_c = wlf_matrix3x3_add(&ab_sum, &c);
	struct wlf_matrix3x3 bc_sum = wlf_matrix3x3_add(&b, &c);
	struct wlf_matrix3x3 a_bc = wlf_matrix3x3_add(&a, &bc_sum);

	if (assert_matrix_equal(&ab_c, &a_bc, EPSILON)) {
		printf("✓ Addition associativity test passed\n");
		passed_tests++;
	} else {
		printf("✗ Addition associativity test failed\n");
	}

	// Test associative property of multiplication: (A * B) * C = A * (B * C)
	test_count++;
	struct wlf_matrix3x3 ab_mult = wlf_matrix3x3_multiply(&a, &b);
	struct wlf_matrix3x3 ab_c_mult = wlf_matrix3x3_multiply(&ab_mult, &c);
	struct wlf_matrix3x3 bc_mult = wlf_matrix3x3_multiply(&b, &c);
	struct wlf_matrix3x3 a_bc_mult = wlf_matrix3x3_multiply(&a, &bc_mult);

	if (assert_matrix_equal(&ab_c_mult, &a_bc_mult, EPSILON)) {
		printf("✓ Multiplication associativity test passed\n");
		passed_tests++;
	} else {
		printf("✗ Multiplication associativity test failed\n");
	}

	// Test distributive property: A * (B + C) = A * B + A * C
	test_count++;
	struct wlf_matrix3x3 bc_add = wlf_matrix3x3_add(&b, &c);
	struct wlf_matrix3x3 a_bc_add = wlf_matrix3x3_multiply(&a, &bc_add);
	struct wlf_matrix3x3 ac_mult = wlf_matrix3x3_multiply(&a, &c);
	struct wlf_matrix3x3 ab_ac_add = wlf_matrix3x3_add(&ab_mult, &ac_mult);

	if (assert_matrix_equal(&a_bc_add, &ab_ac_add, EPSILON)) {
		printf("✓ Distributive property test passed\n");
		passed_tests++;
	} else {
		printf("✗ Distributive property test failed\n");
	}

	// Test transpose of product: (A * B)^T = B^T * A^T
	test_count++;
	struct wlf_matrix3x3 ab_transpose = wlf_matrix3x3_transpose(&ab_mult);
	struct wlf_matrix3x3 b_transpose = wlf_matrix3x3_transpose(&b);
	struct wlf_matrix3x3 a_transpose = wlf_matrix3x3_transpose(&a);
	struct wlf_matrix3x3 bt_at = wlf_matrix3x3_multiply(&b_transpose, &a_transpose);

	if (assert_matrix_equal(&ab_transpose, &bt_at, EPSILON)) {
		printf("✓ Transpose of product property test passed\n");
		passed_tests++;
	} else {
		printf("✗ Transpose of product property test failed\n");
	}
}

static void test_matrix3x3_edge_cases(void) {
	print_test_header("Matrix3x3 Edge Cases Tests");

	// Test with very small numbers
	test_count++;
	struct wlf_matrix3x3 small_matrix = wlf_matrix3x3_create_zero();
	small_matrix.elements[0][0] = 1e-10;
	small_matrix.elements[1][1] = 1e-10;
	small_matrix.elements[2][2] = 1e-10;

	double det_small = wlf_matrix3x3_determinant(&small_matrix);

	if (assert_double_equal(det_small, 1e-30, 1e-35)) {
		printf("✓ Small numbers test passed\n");
		passed_tests++;
	} else {
		printf("✗ Small numbers test failed: expected ~1e-30, got %e\n", det_small);
	}

	// Test with large numbers
	test_count++;
	struct wlf_matrix3x3 large_matrix = wlf_matrix3x3_create_zero();
	large_matrix.elements[0][0] = 1e6;
	large_matrix.elements[1][1] = 1e6;
	large_matrix.elements[2][2] = 1e6;

	double det_large = wlf_matrix3x3_determinant(&large_matrix);

	if (assert_double_equal(det_large, 1e18, 1e12)) {
		printf("✓ Large numbers test passed\n");
		passed_tests++;
	} else {
		printf("✗ Large numbers test failed: expected ~1e18, got %e\n", det_large);
	}

	// Test negative values
	test_count++;
	struct wlf_matrix3x3 negative_matrix = wlf_matrix3x3_create_zero();
	negative_matrix.elements[0][0] = -1; negative_matrix.elements[0][1] = 2; negative_matrix.elements[0][2] = -3;
	negative_matrix.elements[1][0] = 4; negative_matrix.elements[1][1] = -5; negative_matrix.elements[1][2] = 6;
	negative_matrix.elements[2][0] = -7; negative_matrix.elements[2][1] = 8; negative_matrix.elements[2][2] = -9;

	struct wlf_matrix3x3 scaled_negative = wlf_matrix3x3_multiply_scalar(&negative_matrix, -1.0);
	struct wlf_matrix3x3 positive_matrix = wlf_matrix3x3_create_zero();
	positive_matrix.elements[0][0] = 1; positive_matrix.elements[0][1] = -2; positive_matrix.elements[0][2] = 3;
	positive_matrix.elements[1][0] = -4; positive_matrix.elements[1][1] = 5; positive_matrix.elements[1][2] = -6;
	positive_matrix.elements[2][0] = 7; positive_matrix.elements[2][1] = -8; positive_matrix.elements[2][2] = 9;

	if (assert_matrix_equal(&scaled_negative, &positive_matrix, EPSILON)) {
		printf("✓ Negative values test passed\n");
		passed_tests++;
	} else {
		printf("✗ Negative values test failed\n");
	}
}

static void test_matrix3x3_string_representation(void) {
	print_test_header("Matrix3x3 String Representation Tests");

	test_count++;
	struct wlf_matrix3x3 matrix = wlf_matrix3x3_identity();
	char* str_repr = wlf_matrix3x3_to_str(&matrix);

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
	printf("Starting comprehensive wlf_matrix3x3 test suite...\n");

	test_matrix3x3_creation();
	test_matrix3x3_basic_operations();
	test_matrix3x3_arithmetic();
	test_matrix3x3_matrix_multiplication();
	test_matrix3x3_transpose();
	test_matrix3x3_determinant();
	test_matrix3x3_inverse();
	test_matrix3x3_equality();
	test_matrix3x3_mathematical_properties();
	test_matrix3x3_edge_cases();
	test_matrix3x3_string_representation();

	print_test_summary();

	return (passed_tests == test_count) ? 0 : 1;
}
