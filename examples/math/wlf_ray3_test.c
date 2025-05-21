/**
 * @file        wlf_ray3_test.c
 * @brief       Comprehensive test suite for wlf_ray3 functionality.
 * @details     This file provides complete testing coverage for all wlf_ray3
 *              operations including creation, point calculation, equality
 *              checks, string conversion, and geometric properties.
 * @author      Test Suite
 * @date        2024-12-17
 * @version     v1.0
 */

#include "wlf/math/wlf_ray3.h"
#include "wlf/math/wlf_vector3.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Test configuration
static const double EPSILON = 1e-9;
static int test_count = 0;
static int passed_tests = 0;

// Function prototypes
static void print_test_header(const char* test_name);
static void print_test_summary(void);
static void test_ray_creation(void);
static void test_ray_unit_axes(void);
static void test_ray_point_at_parameter(void);
static void test_ray_point_at_parameter_complex(void);
static void test_ray_equality(void);
static void test_ray_nearly_equal(void);
static void test_ray_string_conversion(void);
static void test_ray_geometric_properties(void);
static void test_ray_edge_cases(void);
static void test_ray_normalization_considerations(void);

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

#define ASSERT_VECTOR3_EQ(a, b, message) \
	ASSERT_TRUE(wlf_vector3_nearly_equal(&(a), &(b), EPSILON), message)

void print_test_header(const char* test_name) {
	printf("\n=== %s ===\n", test_name);
}

void print_test_summary(void) {
	printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "\n");
	printf("Test Summary: %d/%d tests passed (%.1f%%)\n",
			passed_tests, test_count,
			test_count > 0 ? (100.0 * passed_tests / test_count) : 0.0);
	printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "\n");
}

// Test functions
static void test_ray_creation(void) {
	print_test_header("Ray Creation Tests");

	// Test basic ray creation
	struct wlf_vector3 origin = wlf_vector3_make(1.0, 2.0, 3.0);
	struct wlf_vector3 direction = wlf_vector3_make(0.0, 1.0, 0.0);
	struct wlf_ray3 ray = wlf_ray3_make(origin, direction);

	ASSERT_VECTOR3_EQ(ray.origin, origin, "Ray origin should match input");
	ASSERT_VECTOR3_EQ(ray.direction, direction, "Ray direction should match input");

	// Test zero ray creation
	struct wlf_vector3 zero = WLF_VECTOR3_ZERO;
	struct wlf_ray3 zero_ray = wlf_ray3_make(zero, zero);

	ASSERT_VECTOR3_EQ(zero_ray.origin, zero, "Zero ray origin should be zero");
	ASSERT_VECTOR3_EQ(zero_ray.direction, zero, "Zero ray direction should be zero");

	// Test WLF_RAY_ZERO constant
	ASSERT_VECTOR3_EQ(WLF_RAY_ZERO.origin, zero, "WLF_RAY_ZERO origin should be zero");
	ASSERT_VECTOR3_EQ(WLF_RAY_ZERO.direction, zero, "WLF_RAY_ZERO direction should be zero");
}

void test_ray_unit_axes(void) {
	print_test_header("Ray Unit Axes Tests");

	// Test rays along primary axes
	struct wlf_vector3 origin = WLF_VECTOR3_ZERO;

	// X-axis ray
	struct wlf_ray3 x_ray = wlf_ray3_make(origin, WLF_VECTOR3_UNIT_X);
	ASSERT_VECTOR3_EQ(x_ray.direction, WLF_VECTOR3_UNIT_X, "X-axis ray direction");

	// Y-axis ray
	struct wlf_ray3 y_ray = wlf_ray3_make(origin, WLF_VECTOR3_UNIT_Y);
	ASSERT_VECTOR3_EQ(y_ray.direction, WLF_VECTOR3_UNIT_Y, "Y-axis ray direction");

	// Z-axis ray
	struct wlf_ray3 z_ray = wlf_ray3_make(origin, WLF_VECTOR3_UNIT_Z);
	ASSERT_VECTOR3_EQ(z_ray.direction, WLF_VECTOR3_UNIT_Z, "Z-axis ray direction");
}

void test_ray_point_at_parameter(void) {
	print_test_header("Ray Point At Parameter Tests");

	// Test ray from origin along x-axis
	struct wlf_vector3 origin = WLF_VECTOR3_ZERO;
	struct wlf_vector3 direction = WLF_VECTOR3_UNIT_X;
	struct wlf_ray3 ray = wlf_ray3_make(origin, direction);

	// Test various parameter values
	struct wlf_vector3 point_0 = wlf_ray3_point_at_parameter(&ray, 0.0);
	ASSERT_VECTOR3_EQ(point_0, origin, "Point at t=0 should be origin");

	struct wlf_vector3 point_1 = wlf_ray3_point_at_parameter(&ray, 1.0);
	struct wlf_vector3 expected_1 = wlf_vector3_make(1.0, 0.0, 0.0);
	ASSERT_VECTOR3_EQ(point_1, expected_1, "Point at t=1 should be (1,0,0)");

	struct wlf_vector3 point_5 = wlf_ray3_point_at_parameter(&ray, 5.0);
	struct wlf_vector3 expected_5 = wlf_vector3_make(5.0, 0.0, 0.0);
	ASSERT_VECTOR3_EQ(point_5, expected_5, "Point at t=5 should be (5,0,0)");

	// Test negative parameter (behind origin)
	struct wlf_vector3 point_neg = wlf_ray3_point_at_parameter(&ray, -2.0);
	struct wlf_vector3 expected_neg = wlf_vector3_make(-2.0, 0.0, 0.0);
	ASSERT_VECTOR3_EQ(point_neg, expected_neg, "Point at t=-2 should be (-2,0,0)");
}

void test_ray_point_at_parameter_complex(void) {
	print_test_header("Ray Point At Parameter Complex Tests");

	// Test ray with non-unit direction
	struct wlf_vector3 origin = wlf_vector3_make(1.0, 2.0, 3.0);
	struct wlf_vector3 direction = wlf_vector3_make(2.0, -1.0, 1.0);
	struct wlf_ray3 ray = wlf_ray3_make(origin, direction);

	// Test point calculation
	struct wlf_vector3 point = wlf_ray3_point_at_parameter(&ray, 2.0);
	struct wlf_vector3 expected = wlf_vector3_make(5.0, 0.0, 5.0); // (1,2,3) + 2*(2,-1,1)
	ASSERT_VECTOR3_EQ(point, expected, "Point calculation with non-unit direction");

	// Test fractional parameter
	struct wlf_vector3 point_half = wlf_ray3_point_at_parameter(&ray, 0.5);
	struct wlf_vector3 expected_half = wlf_vector3_make(2.0, 1.5, 3.5);
	ASSERT_VECTOR3_EQ(point_half, expected_half, "Point calculation with fractional parameter");
}

void test_ray_equality(void) {
	print_test_header("Ray Equality Tests");

	// Test identical rays
	struct wlf_vector3 origin1 = wlf_vector3_make(1.0, 2.0, 3.0);
	struct wlf_vector3 direction1 = wlf_vector3_make(0.0, 1.0, 0.0);
	struct wlf_ray3 ray1 = wlf_ray3_make(origin1, direction1);
	struct wlf_ray3 ray2 = wlf_ray3_make(origin1, direction1);

	ASSERT_TRUE(wlf_ray3_equal(&ray1, &ray2), "Identical rays should be equal");

	// Test different origins
	struct wlf_vector3 origin2 = wlf_vector3_make(2.0, 2.0, 3.0);
	struct wlf_ray3 ray3 = wlf_ray3_make(origin2, direction1);

	ASSERT_TRUE(!wlf_ray3_equal(&ray1, &ray3), "Rays with different origins should not be equal");

	// Test different directions
	struct wlf_vector3 direction2 = wlf_vector3_make(1.0, 0.0, 0.0);
	struct wlf_ray3 ray4 = wlf_ray3_make(origin1, direction2);

	ASSERT_TRUE(!wlf_ray3_equal(&ray1, &ray4), "Rays with different directions should not be equal");

	// Test zero rays
	ASSERT_TRUE(wlf_ray3_equal(&WLF_RAY_ZERO, &WLF_RAY_ZERO), "Zero ray should equal itself");
}

void test_ray_nearly_equal(void) {
	print_test_header("Ray Nearly Equal Tests");

	// Test nearly identical rays
	struct wlf_vector3 origin1 = wlf_vector3_make(1.0, 2.0, 3.0);
	struct wlf_vector3 direction1 = wlf_vector3_make(0.0, 1.0, 0.0);
	struct wlf_ray3 ray1 = wlf_ray3_make(origin1, direction1);

	struct wlf_vector3 origin2 = wlf_vector3_make(1.0 + 1e-10, 2.0, 3.0);
	struct wlf_vector3 direction2 = wlf_vector3_make(0.0, 1.0 + 1e-10, 0.0);
	struct wlf_ray3 ray2 = wlf_ray3_make(origin2, direction2);

	ASSERT_TRUE(wlf_ray3_nearly_equal(&ray1, &ray2, 1e-9), "Nearly identical rays should be nearly equal");
	ASSERT_TRUE(!wlf_ray3_nearly_equal(&ray1, &ray2, 1e-11), "Rays outside epsilon should not be nearly equal");

	// Test exact equality with nearly_equal
	ASSERT_TRUE(wlf_ray3_nearly_equal(&ray1, &ray1, 1e-15), "Ray should be nearly equal to itself");
}

void test_ray_string_conversion(void) {
	print_test_header("Ray String Conversion Tests");

	// Test basic string conversion
	struct wlf_vector3 origin = wlf_vector3_make(1.0, 2.0, 3.0);
	struct wlf_vector3 direction = wlf_vector3_make(0.0, 1.0, 0.0);
	struct wlf_ray3 ray = wlf_ray3_make(origin, direction);

	char* ray_str = wlf_ray3_to_str(&ray);

	ASSERT_TRUE(ray_str != NULL, "String conversion should not return NULL");
	ASSERT_TRUE(strstr(ray_str, "Ray") != NULL, "String should contain 'Ray'");
	ASSERT_TRUE(strstr(ray_str, "Origin") != NULL, "String should contain 'Origin'");
	ASSERT_TRUE(strstr(ray_str, "Direction") != NULL, "String should contain 'Direction'");

	printf("Ray string: %s\n", ray_str);

	// Clean up allocated memory
	free(ray_str);

	// Test zero ray string conversion
	char* zero_str = wlf_ray3_to_str(&WLF_RAY_ZERO);
	ASSERT_TRUE(zero_str != NULL, "Zero ray string conversion should not return NULL");
	printf("Zero ray string: %s\n", zero_str);
	free(zero_str);
}

void test_ray_geometric_properties(void) {
	print_test_header("Ray Geometric Properties Tests");

	// Test ray parameterization property: P(t) = O + t*D
	struct wlf_vector3 origin = wlf_vector3_make(2.0, 3.0, 4.0);
	struct wlf_vector3 direction = wlf_vector3_make(1.0, -1.0, 2.0);
	struct wlf_ray3 ray = wlf_ray3_make(origin, direction);

	// Verify parameterization for multiple points
	for (int i = 0; i < 5; i++) {
		double t = i * 0.5;
		struct wlf_vector3 point = wlf_ray3_point_at_parameter(&ray, t);

		// Manual calculation: P(t) = O + t*D
		struct wlf_vector3 scaled_direction = wlf_vector3_multiply(&direction, t);
		struct wlf_vector3 expected = wlf_vector3_add(&origin, &scaled_direction);

		char msg[100];
		snprintf(msg, sizeof(msg), "Parameterization should hold for t=%.1f", t);
		ASSERT_VECTOR3_EQ(point, expected, msg);
	}

	// Test linearity: P(t1) + P(t2) should relate to P(t1+t2) through direction
	double t1 = 1.5, t2 = 2.5;
	struct wlf_vector3 p1 = wlf_ray3_point_at_parameter(&ray, t1);
	struct wlf_vector3 p_sum = wlf_ray3_point_at_parameter(&ray, t1 + t2);

	// The vector from p1 to p_sum should be t2 * direction
	struct wlf_vector3 diff = wlf_vector3_subtract(&p_sum, &p1);
	struct wlf_vector3 expected_diff = wlf_vector3_multiply(&direction, t2);
	ASSERT_VECTOR3_EQ(diff, expected_diff, "Ray parameterization linearity property");
}

void test_ray_edge_cases(void) {
	print_test_header("Ray Edge Cases Tests");

	// Test ray with very large coordinates
	struct wlf_vector3 large_origin = wlf_vector3_make(1e6, -1e6, 1e8);
	struct wlf_vector3 small_direction = wlf_vector3_make(1e-6, 1e-8, 1e-10);
	struct wlf_ray3 large_ray = wlf_ray3_make(large_origin, small_direction);

	struct wlf_vector3 point = wlf_ray3_point_at_parameter(&large_ray, 1e6);
	ASSERT_TRUE(isfinite(point.x) && isfinite(point.y) && isfinite(point.z),
				"Point calculation should remain finite for large coordinates");

	// Test ray with zero direction
	struct wlf_vector3 origin = wlf_vector3_make(1.0, 2.0, 3.0);
	struct wlf_vector3 zero_direction = WLF_VECTOR3_ZERO;
	struct wlf_ray3 zero_dir_ray = wlf_ray3_make(origin, zero_direction);

	struct wlf_vector3 point_zero_dir = wlf_ray3_point_at_parameter(&zero_dir_ray, 10.0);
	ASSERT_VECTOR3_EQ(point_zero_dir, origin, "Ray with zero direction should always return origin");

	// Test very small epsilon
	struct wlf_ray3 ray1 = wlf_ray3_make(origin, WLF_VECTOR3_UNIT_X);
	struct wlf_vector3 slightly_different = wlf_vector3_make(1.0 + 1e-15, 2.0, 3.0);
	struct wlf_ray3 ray2 = wlf_ray3_make(slightly_different, WLF_VECTOR3_UNIT_X);

	ASSERT_TRUE(!wlf_ray3_nearly_equal(&ray1, &ray2, 1e-16),
				"Very small differences should be detected with tiny epsilon");
}

void test_ray_normalization_considerations(void) {
	print_test_header("Ray Normalization Considerations Tests");

	// Note: The API doesn't enforce normalized directions, but we test behavior
	struct wlf_vector3 origin = WLF_VECTOR3_ZERO;

	// Test with normalized direction
	struct wlf_vector3 temp_vector = wlf_vector3_make(3.0, 4.0, 0.0);
	struct wlf_vector3 normalized_dir = wlf_vector3_normalize(&temp_vector);
	struct wlf_ray3 normalized_ray = wlf_ray3_make(origin, normalized_dir);

	struct wlf_vector3 point_norm = wlf_ray3_point_at_parameter(&normalized_ray, 5.0);
	double distance_norm = wlf_vector3_magnitude(&point_norm);
	ASSERT_DOUBLE_EQ(distance_norm, 5.0, "Normalized ray should give expected distance");

	// Test with non-normalized direction
	struct wlf_vector3 non_normalized_dir = wlf_vector3_make(3.0, 4.0, 0.0); // magnitude = 5
	struct wlf_ray3 non_normalized_ray = wlf_ray3_make(origin, non_normalized_dir);

	struct wlf_vector3 point_non_norm = wlf_ray3_point_at_parameter(&non_normalized_ray, 1.0);
	double distance_non_norm = wlf_vector3_magnitude(&point_non_norm);
	ASSERT_DOUBLE_EQ(distance_non_norm, 5.0, "Non-normalized ray parameter relates to direction magnitude");
}

int main(void) {
	printf("Starting wlf_ray3 comprehensive test suite...\n");

	test_ray_creation();
	test_ray_unit_axes();
	test_ray_point_at_parameter();
	test_ray_point_at_parameter_complex();
	test_ray_equality();
	test_ray_nearly_equal();
	test_ray_string_conversion();
	test_ray_geometric_properties();
	test_ray_edge_cases();
	test_ray_normalization_considerations();

	print_test_summary();

	return (passed_tests == test_count) ? 0 : 1;
}
