/**
 * @file        wlf_quaternion_test.c
 * @brief       Comprehensive test suite for wlf_quaternion functionality.
 * @details     This file provides complete testing coverage for all wlf_quaternion
 *              operations including creation, arithmetic, normalization, conjugation,
 *              inversion, string conversion, and mathematical properties.
 * @author      Test Suite
 * @date        2024-12-17
 * @version     v1.0
 */

#include "wlf/math/wlf_quaternion.h"

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
static void test_quaternion_creation(void);
static void test_quaternion_constants(void);
static void test_quaternion_arithmetic(void);
static void test_quaternion_conjugate(void);
static void test_quaternion_norm_and_normalize(void);
static void test_quaternion_inverse(void);
static void test_quaternion_equality(void);
static void test_quaternion_string_conversion(void);
static void test_quaternion_mathematical_properties(void);
static void test_quaternion_rotation_properties(void);
static void test_quaternion_edge_cases(void);
static void test_quaternion_unit_quaternions(void);

// Helper macros for testing
#define ASSERT_TRUE(condition, message) \
	do { \
		test_count++; \
		if (condition) { \
			passed_tests++; \
			printf("✓ PASS: %s\n", message); \
		} else { \
			printf("✗ FAIL: %s\n", message); \
		} \
	} while(0)

#define ASSERT_DOUBLE_EQ(a, b, message) \
	ASSERT_TRUE(fabs((a) - (b)) < EPSILON, message)

#define ASSERT_QUATERNION_EQ(a, b, message) \
	ASSERT_TRUE(wlf_quaternion_nearly_equal(&(a), &(b), EPSILON), message)

// Test helper functions
static void print_test_header(const char* test_name) {
	printf("\n=== %s ===\n", test_name);
}

static void print_test_summary(void) {
	printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "\n");
	printf("Test Summary: %d/%d tests passed (%.1f%%)\n",
			passed_tests, test_count,
			test_count > 0 ? (100.0 * passed_tests / test_count) : 0.0);
	printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "\n");
}

// Test functions
static void test_quaternion_creation(void) {
	print_test_header("Quaternion Creation Tests");

	// Test basic quaternion creation
	struct wlf_quaternion q1 = wlf_quaternion_make(1.0, 2.0, 3.0, 4.0);
	ASSERT_DOUBLE_EQ(q1.w, 1.0, "Quaternion w component should match input");
	ASSERT_DOUBLE_EQ(q1.x, 2.0, "Quaternion x component should match input");
	ASSERT_DOUBLE_EQ(q1.y, 3.0, "Quaternion y component should match input");
	ASSERT_DOUBLE_EQ(q1.z, 4.0, "Quaternion z component should match input");

	// Test zero quaternion creation
	struct wlf_quaternion zero = wlf_quaternion_make(0.0, 0.0, 0.0, 0.0);
	ASSERT_DOUBLE_EQ(zero.w, 0.0, "Zero quaternion w should be 0");
	ASSERT_DOUBLE_EQ(zero.x, 0.0, "Zero quaternion x should be 0");
	ASSERT_DOUBLE_EQ(zero.y, 0.0, "Zero quaternion y should be 0");
	ASSERT_DOUBLE_EQ(zero.z, 0.0, "Zero quaternion z should be 0");

	// Test unit quaternion creation
	struct wlf_quaternion unit = wlf_quaternion_make(1.0, 0.0, 0.0, 0.0);
	ASSERT_DOUBLE_EQ(unit.w, 1.0, "Unit quaternion w should be 1");
	ASSERT_DOUBLE_EQ(unit.x, 0.0, "Unit quaternion x should be 0");
	ASSERT_DOUBLE_EQ(unit.y, 0.0, "Unit quaternion y should be 0");
	ASSERT_DOUBLE_EQ(unit.z, 0.0, "Unit quaternion z should be 0");
}

static void test_quaternion_constants(void) {
	print_test_header("Quaternion Constants Tests");

	// Test identity quaternion constant
	ASSERT_DOUBLE_EQ(WLF_QUATERNION_IDENTITY.w, 1.0, "Identity quaternion w should be 1");
	ASSERT_DOUBLE_EQ(WLF_QUATERNION_IDENTITY.x, 0.0, "Identity quaternion x should be 0");
	ASSERT_DOUBLE_EQ(WLF_QUATERNION_IDENTITY.y, 0.0, "Identity quaternion y should be 0");
	ASSERT_DOUBLE_EQ(WLF_QUATERNION_IDENTITY.z, 0.0, "Identity quaternion z should be 0");

	// Test that identity has norm 1
	double identity_norm = wlf_quaternion_norm(&WLF_QUATERNION_IDENTITY);
	ASSERT_DOUBLE_EQ(identity_norm, 1.0, "Identity quaternion should have norm 1");
}

static void test_quaternion_arithmetic(void) {
	print_test_header("Quaternion Arithmetic Tests");

	struct wlf_quaternion q1 = wlf_quaternion_make(1.0, 2.0, 3.0, 4.0);
	struct wlf_quaternion q2 = wlf_quaternion_make(0.5, 1.5, 2.5, 3.5);

	// Test addition
	struct wlf_quaternion sum = wlf_quaternion_add(&q1, &q2);
	struct wlf_quaternion expected_sum = wlf_quaternion_make(1.5, 3.5, 5.5, 7.5);
	ASSERT_QUATERNION_EQ(sum, expected_sum, "Quaternion addition should be component-wise");

	// Test subtraction
	struct wlf_quaternion diff = wlf_quaternion_subtract(&q1, &q2);
	struct wlf_quaternion expected_diff = wlf_quaternion_make(0.5, 0.5, 0.5, 0.5);
	ASSERT_QUATERNION_EQ(diff, expected_diff, "Quaternion subtraction should be component-wise");

	// Test multiplication (Hamilton product)
	struct wlf_quaternion prod = wlf_quaternion_multiply(&q1, &q2);
	// Manual calculation: (1,2,3,4) * (0.5,1.5,2.5,3.5)
	// w = 1*0.5 - 2*1.5 - 3*2.5 - 4*3.5 = 0.5 - 3 - 7.5 - 14 = -24
	// x = 1*1.5 + 2*0.5 + 3*3.5 - 4*2.5 = 1.5 + 1 + 10.5 - 10 = 3
	// y = 1*2.5 - 2*3.5 + 3*0.5 + 4*1.5 = 2.5 - 7 + 1.5 + 6 = 3
	// z = 1*3.5 + 2*2.5 - 3*1.5 + 4*0.5 = 3.5 + 5 - 4.5 + 2 = 6
	struct wlf_quaternion expected_prod = wlf_quaternion_make(-24.0, 3.0, 3.0, 6.0);
	ASSERT_QUATERNION_EQ(prod, expected_prod, "Quaternion multiplication should follow Hamilton product rules");

	// Test multiplication with identity
	struct wlf_quaternion identity_prod = wlf_quaternion_multiply(&q1, &WLF_QUATERNION_IDENTITY);
	ASSERT_QUATERNION_EQ(identity_prod, q1, "Multiplication with identity should return original quaternion");

	struct wlf_quaternion identity_prod2 = wlf_quaternion_multiply(&WLF_QUATERNION_IDENTITY, &q1);
	ASSERT_QUATERNION_EQ(identity_prod2, q1, "Identity multiplication should be commutative");
}

static void test_quaternion_conjugate(void) {
	print_test_header("Quaternion Conjugate Tests");

	struct wlf_quaternion q = wlf_quaternion_make(1.0, 2.0, 3.0, 4.0);
	struct wlf_quaternion conjugate = wlf_quaternion_conjugate(&q);

	// Conjugate should negate x, y, z components
	struct wlf_quaternion expected_conj = wlf_quaternion_make(1.0, -2.0, -3.0, -4.0);
	ASSERT_QUATERNION_EQ(conjugate, expected_conj, "Conjugate should negate vector components");

	// Test double conjugate
	struct wlf_quaternion double_conj = wlf_quaternion_conjugate(&conjugate);
	ASSERT_QUATERNION_EQ(double_conj, q, "Double conjugate should return original quaternion");

	// Test conjugate of identity
	struct wlf_quaternion identity_conj = wlf_quaternion_conjugate(&WLF_QUATERNION_IDENTITY);
	ASSERT_QUATERNION_EQ(identity_conj, WLF_QUATERNION_IDENTITY, "Conjugate of identity should be identity");

	// Test conjugate property: conj(q1 * q2) = conj(q2) * conj(q1)
	struct wlf_quaternion q1 = wlf_quaternion_make(1.0, 1.0, 0.0, 0.0);
	struct wlf_quaternion q2 = wlf_quaternion_make(0.0, 0.0, 1.0, 1.0);
	struct wlf_quaternion prod = wlf_quaternion_multiply(&q1, &q2);
	struct wlf_quaternion conj_prod = wlf_quaternion_conjugate(&prod);

	struct wlf_quaternion conj_q1 = wlf_quaternion_conjugate(&q1);
	struct wlf_quaternion conj_q2 = wlf_quaternion_conjugate(&q2);
	struct wlf_quaternion conj_q2_times_conj_q1 = wlf_quaternion_multiply(&conj_q2, &conj_q1);

	ASSERT_QUATERNION_EQ(conj_prod, conj_q2_times_conj_q1, "Conjugate distribution property should hold");
}

static void test_quaternion_norm_and_normalize(void) {
	print_test_header("Quaternion Norm and Normalize Tests");

	// Test norm calculation
	struct wlf_quaternion q = wlf_quaternion_make(1.0, 2.0, 3.0, 4.0);
	double norm = wlf_quaternion_norm(&q);
	double expected_norm = sqrt(1*1 + 2*2 + 3*3 + 4*4); // sqrt(30)
	ASSERT_DOUBLE_EQ(norm, expected_norm, "Norm should be sqrt of sum of squares");

	// Test normalization
	struct wlf_quaternion normalized = wlf_quaternion_normalize(&q);
	double normalized_norm = wlf_quaternion_norm(&normalized);
	ASSERT_DOUBLE_EQ(normalized_norm, 1.0, "Normalized quaternion should have norm 1");

	// Test normalization preserves direction
	struct wlf_quaternion scaled_back = wlf_quaternion_make(
		normalized.w * expected_norm,
		normalized.x * expected_norm,
		normalized.y * expected_norm,
		normalized.z * expected_norm
	);
	ASSERT_QUATERNION_EQ(scaled_back, q, "Normalization should preserve direction");

	// Test identity normalization
	struct wlf_quaternion identity_normalized = wlf_quaternion_normalize(&WLF_QUATERNION_IDENTITY);
	ASSERT_QUATERNION_EQ(identity_normalized, WLF_QUATERNION_IDENTITY, "Identity normalization should return identity");

	// Test zero quaternion normalization
	struct wlf_quaternion zero = wlf_quaternion_make(0.0, 0.0, 0.0, 0.0);
	struct wlf_quaternion zero_normalized = wlf_quaternion_normalize(&zero);
	ASSERT_QUATERNION_EQ(zero_normalized, WLF_QUATERNION_IDENTITY, "Zero quaternion normalization should return identity");
}

static void test_quaternion_inverse(void) {
	print_test_header("Quaternion Inverse Tests");

	// Test inverse of unit quaternion
	struct wlf_quaternion temp_q = wlf_quaternion_make(1.0, 1.0, 1.0, 1.0);
	struct wlf_quaternion unit_q = wlf_quaternion_normalize(&temp_q);
	struct wlf_quaternion inverse = wlf_quaternion_inverse(&unit_q);

	// For unit quaternions, inverse equals conjugate
	struct wlf_quaternion conjugate = wlf_quaternion_conjugate(&unit_q);
	ASSERT_QUATERNION_EQ(inverse, conjugate, "Inverse of unit quaternion should equal conjugate");

	// Test q * q^(-1) = identity
	struct wlf_quaternion prod_with_inverse = wlf_quaternion_multiply(&unit_q, &inverse);
	ASSERT_QUATERNION_EQ(prod_with_inverse, WLF_QUATERNION_IDENTITY, "Quaternion times its inverse should equal identity");

	// Test q^(-1) * q = identity
	struct wlf_quaternion inverse_prod = wlf_quaternion_multiply(&inverse, &unit_q);
	ASSERT_QUATERNION_EQ(inverse_prod, WLF_QUATERNION_IDENTITY, "Inverse times quaternion should equal identity");

	// Test identity inverse
	struct wlf_quaternion identity_inverse = wlf_quaternion_inverse(&WLF_QUATERNION_IDENTITY);
	ASSERT_QUATERNION_EQ(identity_inverse, WLF_QUATERNION_IDENTITY, "Inverse of identity should be identity");

	// Test non-unit quaternion inverse
	struct wlf_quaternion q = wlf_quaternion_make(2.0, 1.0, 0.0, 0.0);
	struct wlf_quaternion q_inverse = wlf_quaternion_inverse(&q);
	struct wlf_quaternion q_times_inverse = wlf_quaternion_multiply(&q, &q_inverse);
	ASSERT_QUATERNION_EQ(q_times_inverse, WLF_QUATERNION_IDENTITY, "Non-unit quaternion times its inverse should equal identity");
}

static void test_quaternion_equality(void) {
	print_test_header("Quaternion Equality Tests");

	// Test exact equality
	struct wlf_quaternion q1 = wlf_quaternion_make(1.0, 2.0, 3.0, 4.0);
	struct wlf_quaternion q2 = wlf_quaternion_make(1.0, 2.0, 3.0, 4.0);
	ASSERT_TRUE(wlf_quaternion_equal(&q1, &q2), "Identical quaternions should be equal");

	// Test inequality
	struct wlf_quaternion q3 = wlf_quaternion_make(1.1, 2.0, 3.0, 4.0);
	ASSERT_TRUE(!wlf_quaternion_equal(&q1, &q3), "Different quaternions should not be equal");

	// Test nearly equal
	struct wlf_quaternion q4 = wlf_quaternion_make(1.0 + 1e-10, 2.0, 3.0, 4.0);
	ASSERT_TRUE(wlf_quaternion_nearly_equal(&q1, &q4, 1e-9), "Nearly identical quaternions should be nearly equal");
	ASSERT_TRUE(!wlf_quaternion_nearly_equal(&q1, &q4, 1e-11), "Quaternions outside epsilon should not be nearly equal");

	// Test self equality
	ASSERT_TRUE(wlf_quaternion_equal(&q1, &q1), "Quaternion should equal itself");
	ASSERT_TRUE(wlf_quaternion_nearly_equal(&q1, &q1, 1e-15), "Quaternion should be nearly equal to itself");
}

static void test_quaternion_string_conversion(void) {
	print_test_header("Quaternion String Conversion Tests");

	struct wlf_quaternion q = wlf_quaternion_make(1.0, 2.0, 3.0, 4.0);
	char* q_str = wlf_quaternion_to_str(&q);

	ASSERT_TRUE(q_str != NULL, "String conversion should not return NULL");
	ASSERT_TRUE(strstr(q_str, "Quaternion") != NULL, "String should contain 'Quaternion'");
	ASSERT_TRUE(strstr(q_str, "1.00") != NULL, "String should contain w component");
	ASSERT_TRUE(strstr(q_str, "2.00") != NULL, "String should contain x component");
	ASSERT_TRUE(strstr(q_str, "3.00") != NULL, "String should contain y component");
	ASSERT_TRUE(strstr(q_str, "4.00") != NULL, "String should contain z component");

	printf("Quaternion string: %s\n", q_str);
	free(q_str);

	// Test identity string conversion
	char* identity_str = wlf_quaternion_to_str(&WLF_QUATERNION_IDENTITY);
	ASSERT_TRUE(identity_str != NULL, "Identity string conversion should not return NULL");
	printf("Identity quaternion string: %s\n", identity_str);
	free(identity_str);
}

static void test_quaternion_mathematical_properties(void) {
	print_test_header("Quaternion Mathematical Properties Tests");

	struct wlf_quaternion q1 = wlf_quaternion_make(1.0, 1.0, 0.0, 0.0);
	struct wlf_quaternion q2 = wlf_quaternion_make(0.0, 0.0, 1.0, 1.0);
	struct wlf_quaternion q3 = wlf_quaternion_make(0.5, 0.5, 0.5, 0.5);

	// Test associativity: (q1 * q2) * q3 = q1 * (q2 * q3)
	struct wlf_quaternion temp_prod_12 = wlf_quaternion_multiply(&q1, &q2);
	struct wlf_quaternion left_assoc = wlf_quaternion_multiply(&temp_prod_12, &q3);
	struct wlf_quaternion temp_prod_23 = wlf_quaternion_multiply(&q2, &q3);
	struct wlf_quaternion right_assoc = wlf_quaternion_multiply(&q1, &temp_prod_23);
	ASSERT_QUATERNION_EQ(left_assoc, right_assoc, "Quaternion multiplication should be associative");

	// Test distributivity of addition over multiplication: q1 * (q2 + q3) = q1*q2 + q1*q3
	struct wlf_quaternion sum_23 = wlf_quaternion_add(&q2, &q3);
	struct wlf_quaternion left_dist = wlf_quaternion_multiply(&q1, &sum_23);
	struct wlf_quaternion prod_12 = wlf_quaternion_multiply(&q1, &q2);
	struct wlf_quaternion prod_13 = wlf_quaternion_multiply(&q1, &q3);
	struct wlf_quaternion right_dist = wlf_quaternion_add(&prod_12, &prod_13);
	ASSERT_QUATERNION_EQ(left_dist, right_dist, "Multiplication should be distributive over addition");

	// Test norm property: |q1 * q2| = |q1| * |q2|
	struct wlf_quaternion prod_12_test = wlf_quaternion_multiply(&q1, &q2);
	double norm_prod = wlf_quaternion_norm(&prod_12_test);
	double prod_norms = wlf_quaternion_norm(&q1) * wlf_quaternion_norm(&q2);
	ASSERT_DOUBLE_EQ(norm_prod, prod_norms, "Norm of product should equal product of norms");

	// Test that quaternion multiplication is generally non-commutative
	struct wlf_quaternion prod_21 = wlf_quaternion_multiply(&q2, &q1);
	ASSERT_TRUE(!wlf_quaternion_nearly_equal(&prod_12_test, &prod_21, EPSILON),
				"Quaternion multiplication should generally be non-commutative");
}

static void test_quaternion_rotation_properties(void) {
	print_test_header("Quaternion Rotation Properties Tests");

	// Test unit quaternions for rotation
	// Rotation of 90 degrees around z-axis: q = (cos(45°), 0, 0, sin(45°))
	double angle = M_PI / 4; // 45 degrees (half angle for quaternion)
	struct wlf_quaternion rot_z = wlf_quaternion_make(cos(angle), 0.0, 0.0, sin(angle));

	double norm = wlf_quaternion_norm(&rot_z);
	ASSERT_DOUBLE_EQ(norm, 1.0, "Rotation quaternion should be unit quaternion");

	// Test 180-degree rotation: q = (0, 0, 0, 1) for z-axis
	struct wlf_quaternion rot_180_z = wlf_quaternion_make(0.0, 0.0, 0.0, 1.0);
	double norm_180 = wlf_quaternion_norm(&rot_180_z);
	ASSERT_DOUBLE_EQ(norm_180, 1.0, "180-degree rotation quaternion should be unit");

	// Test that rotation by 360 degrees equals identity (up to sign)
	// 360 degree rotation = q^4 for 90-degree rotation
	struct wlf_quaternion rot_z_2 = wlf_quaternion_multiply(&rot_z, &rot_z);
	struct wlf_quaternion rot_z_4 = wlf_quaternion_multiply(&rot_z_2, &rot_z_2);

	// 360-degree rotation should be ±identity
	struct wlf_quaternion neg_identity_rot = wlf_quaternion_make(-1, 0, 0, 0);
	ASSERT_TRUE(wlf_quaternion_nearly_equal(&rot_z_4, &WLF_QUATERNION_IDENTITY, EPSILON) ||
				wlf_quaternion_nearly_equal(&rot_z_4, &neg_identity_rot, EPSILON),
				"360-degree rotation should equal ±identity");
}

static void test_quaternion_edge_cases(void) {
	print_test_header("Quaternion Edge Cases Tests");

	// Test very small quaternions
	struct wlf_quaternion tiny = wlf_quaternion_make(1e-10, 1e-10, 1e-10, 1e-10);
	double tiny_norm = wlf_quaternion_norm(&tiny);
	ASSERT_TRUE(tiny_norm < 1e-9, "Tiny quaternion should have small norm");

	struct wlf_quaternion tiny_normalized = wlf_quaternion_normalize(&tiny);
	double tiny_norm_after = wlf_quaternion_norm(&tiny_normalized);
	ASSERT_DOUBLE_EQ(tiny_norm_after, 1.0, "Normalized tiny quaternion should have unit norm");

	// Test very large quaternions
	struct wlf_quaternion large = wlf_quaternion_make(1e6, 1e6, 1e6, 1e6);
	double large_norm = wlf_quaternion_norm(&large);
	ASSERT_TRUE(large_norm > 1e6, "Large quaternion should have large norm");

	struct wlf_quaternion large_normalized = wlf_quaternion_normalize(&large);
	double large_norm_after = wlf_quaternion_norm(&large_normalized);
	ASSERT_DOUBLE_EQ(large_norm_after, 1.0, "Normalized large quaternion should have unit norm");

	// Test zero quaternion edge cases
	struct wlf_quaternion zero = wlf_quaternion_make(0.0, 0.0, 0.0, 0.0);
	double zero_norm = wlf_quaternion_norm(&zero);
	ASSERT_DOUBLE_EQ(zero_norm, 0.0, "Zero quaternion should have zero norm");

	struct wlf_quaternion zero_inverse = wlf_quaternion_inverse(&zero);
	ASSERT_QUATERNION_EQ(zero_inverse, WLF_QUATERNION_IDENTITY, "Zero quaternion inverse should return identity");
}

static void test_quaternion_unit_quaternions(void) {
	print_test_header("Unit Quaternion Tests");

	// Test some common unit quaternions
	struct wlf_quaternion unit_quaternions[] = {
		{1.0, 0.0, 0.0, 0.0},  // Identity
		{0.0, 1.0, 0.0, 0.0},  // i
		{0.0, 0.0, 1.0, 0.0},  // j
		{0.0, 0.0, 0.0, 1.0},  // k
		{0.7071067812, 0.7071067812, 0.0, 0.0},  // Normalized (1,1,0,0)
		{0.5, 0.5, 0.5, 0.5}   // Normalized (1,1,1,1)
	};

	for (int i = 0; i < 6; i++) {
		double norm = wlf_quaternion_norm(&unit_quaternions[i]);
		char msg[100];
		snprintf(msg, sizeof(msg), "Unit quaternion %d should have norm 1", i);
		ASSERT_DOUBLE_EQ(norm, 1.0, msg);

		// Test that inverse equals conjugate for unit quaternions
		struct wlf_quaternion inverse = wlf_quaternion_inverse(&unit_quaternions[i]);
		struct wlf_quaternion conjugate = wlf_quaternion_conjugate(&unit_quaternions[i]);
		snprintf(msg, sizeof(msg), "Unit quaternion %d inverse should equal conjugate", i);
		ASSERT_QUATERNION_EQ(inverse, conjugate, msg);
	}

	// Test fundamental quaternion relations: i^2 = j^2 = k^2 = ijk = -1
	struct wlf_quaternion i = {0.0, 1.0, 0.0, 0.0};
	struct wlf_quaternion j = {0.0, 0.0, 1.0, 0.0};
	struct wlf_quaternion k = {0.0, 0.0, 0.0, 1.0};
	struct wlf_quaternion neg_identity = {-1.0, 0.0, 0.0, 0.0};

	struct wlf_quaternion i_squared = wlf_quaternion_multiply(&i, &i);
	ASSERT_QUATERNION_EQ(i_squared, neg_identity, "i^2 should equal -1");

	struct wlf_quaternion j_squared = wlf_quaternion_multiply(&j, &j);
	ASSERT_QUATERNION_EQ(j_squared, neg_identity, "j^2 should equal -1");

	struct wlf_quaternion k_squared = wlf_quaternion_multiply(&k, &k);
	ASSERT_QUATERNION_EQ(k_squared, neg_identity, "k^2 should equal -1");

	// Test ijk = -1
	struct wlf_quaternion ij = wlf_quaternion_multiply(&i, &j);
	struct wlf_quaternion ijk = wlf_quaternion_multiply(&ij, &k);
	ASSERT_QUATERNION_EQ(ijk, neg_identity, "ijk should equal -1");

	// Test fundamental relations: ij = k, ji = -k, etc.
	ASSERT_QUATERNION_EQ(ij, k, "ij should equal k");

	struct wlf_quaternion ji = wlf_quaternion_multiply(&j, &i);
	struct wlf_quaternion neg_k = {0.0, 0.0, 0.0, -1.0};
	ASSERT_QUATERNION_EQ(ji, neg_k, "ji should equal -k");
}

int main(void) {
	printf("Starting wlf_quaternion comprehensive test suite...\n");

	test_quaternion_creation();
	test_quaternion_constants();
	test_quaternion_arithmetic();
	test_quaternion_conjugate();
	test_quaternion_norm_and_normalize();
	test_quaternion_inverse();
	test_quaternion_equality();
	test_quaternion_string_conversion();
	test_quaternion_mathematical_properties();
	test_quaternion_rotation_properties();
	test_quaternion_edge_cases();
	test_quaternion_unit_quaternions();

	print_test_summary();

	return (passed_tests == test_count) ? 0 : 1;
}
