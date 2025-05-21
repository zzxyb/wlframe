/**
 * @file        wlf_ray2_test.c
 * @brief       Comprehensive test suite for wlf_ray2 functionality.
 * @details     This file provides complete testing coverage for all wlf_ray2
 *              operations including creation, point calculation, equality
 *              checks, string conversion, and geometric properties.
 * @author      Test Suite
 * @date        2025-06-22
 * @version     v1.0
 */

#include "wlf/math/wlf_ray2.h"
#include "wlf/math/wlf_vector2.h"

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
	do { \
		test_count++; \
		if (fabs((a) - (b)) < EPSILON) { \
			passed_tests++; \
			printf("✓ PASS: %s\n", message); \
		} else { \
			printf("✗ FAIL: %s (expected %.9f, got %.9f)\n", message, (double)(b), (double)(a)); \
		} \
	} while(0)

#define ASSERT_VECTOR2_EQ(v1, v2, message) \
	do { \
		test_count++; \
		if (fabs((v1).u - (v2).u) < EPSILON && \
			fabs((v1).v - (v2).v) < EPSILON) { \
			passed_tests++; \
			printf("✓ PASS: %s\n", message); \
		} else { \
			printf("✗ FAIL: %s (expected (%.9f, %.9f), got (%.9f, %.9f))\n", \
				message, (v2).u, (v2).v, (v1).u, (v1).v); \
		} \
	} while(0)

static void print_test_header(const char* test_name) {
	printf("\n=== %s ===\n", test_name);
}

static void print_test_summary(void) {
	printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "\n");
	printf("TEST SUMMARY\n");
	printf("" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "\n");
	printf("Total tests: %d\n", test_count);
	printf("Passed: %d\n", passed_tests);
	printf("Failed: %d\n", test_count - passed_tests);
	printf("Success rate: %.1f%%\n", test_count > 0 ? (100.0 * passed_tests / test_count) : 0.0);
	printf("" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "\n");
}

static void test_ray_creation(void) {
	print_test_header("Ray Creation Tests");
	
	// Test basic ray creation
	struct wlf_vector2 origin = {1.0, 2.0};
	struct wlf_vector2 direction = {1.0, 0.0};
	struct wlf_ray2 ray = wlf_ray2_make(origin, direction);
	
	ASSERT_VECTOR2_EQ(ray.origin, origin, "Ray origin correctly set");
	ASSERT_VECTOR2_EQ(ray.direction, direction, "Ray direction correctly set");
	
	// Test zero ray constant
	ASSERT_VECTOR2_EQ(WLF_RAY2_ZERO.origin, ((struct wlf_vector2){0.0, 0.0}), "Zero ray origin is (0,0)");
	ASSERT_VECTOR2_EQ(WLF_RAY2_ZERO.direction, ((struct wlf_vector2){0.0, 0.0}), "Zero ray direction is (0,0)");
}

static void test_ray_unit_axes(void) {
	print_test_header("Ray Unit Axes Tests");
	
	// Test rays along unit axes
	struct wlf_vector2 origin = {0.0, 0.0};
	
	// U-axis ray (horizontal right)
	struct wlf_ray2 u_ray = wlf_ray2_make(origin, (struct wlf_vector2){1.0, 0.0});
	ASSERT_VECTOR2_EQ(u_ray.direction, ((struct wlf_vector2){1.0, 0.0}), "U-axis ray direction is (1,0)");
	
	// V-axis ray (vertical up)
	struct wlf_ray2 v_ray = wlf_ray2_make(origin, (struct wlf_vector2){0.0, 1.0});
	ASSERT_VECTOR2_EQ(v_ray.direction, ((struct wlf_vector2){0.0, 1.0}), "V-axis ray direction is (0,1)");
	
	// Negative axes
	struct wlf_ray2 neg_u_ray = wlf_ray2_make(origin, (struct wlf_vector2){-1.0, 0.0});
	ASSERT_VECTOR2_EQ(neg_u_ray.direction, ((struct wlf_vector2){-1.0, 0.0}), "Negative U-axis ray direction is (-1,0)");
	
	struct wlf_ray2 neg_v_ray = wlf_ray2_make(origin, (struct wlf_vector2){0.0, -1.0});
	ASSERT_VECTOR2_EQ(neg_v_ray.direction, ((struct wlf_vector2){0.0, -1.0}), "Negative V-axis ray direction is (0,-1)");
}

static void test_ray_point_at_parameter(void) {
	print_test_header("Ray Point at Parameter Tests");
	
	// Test basic parametric point calculation
	struct wlf_vector2 origin = {1.0, 2.0};
	struct wlf_vector2 direction = {3.0, 4.0};
	struct wlf_ray2 ray = wlf_ray2_make(origin, direction);
	
	// At t=0, should return origin
	struct wlf_vector2 point_at_0 = wlf_ray2_point_at_parameter(&ray, 0.0);
	ASSERT_VECTOR2_EQ(point_at_0, origin, "Point at t=0 equals origin");
	
	// At t=1, should return origin + direction
	struct wlf_vector2 point_at_1 = wlf_ray2_point_at_parameter(&ray, 1.0);
	struct wlf_vector2 expected_at_1 = {4.0, 6.0}; // (1+3, 2+4)
	ASSERT_VECTOR2_EQ(point_at_1, expected_at_1, "Point at t=1 equals origin + direction");
	
	// At t=0.5, should return origin + 0.5*direction
	struct wlf_vector2 point_at_half = wlf_ray2_point_at_parameter(&ray, 0.5);
	struct wlf_vector2 expected_at_half = {2.5, 4.0}; // (1+1.5, 2+2)
	ASSERT_VECTOR2_EQ(point_at_half, expected_at_half, "Point at t=0.5 equals origin + 0.5*direction");
	
	// Test negative parameter
	struct wlf_vector2 point_at_neg = wlf_ray2_point_at_parameter(&ray, -1.0);
	struct wlf_vector2 expected_at_neg = {-2.0, -2.0}; // (1-3, 2-4)
	ASSERT_VECTOR2_EQ(point_at_neg, expected_at_neg, "Point at t=-1 moves backwards along ray");
}

static void test_ray_point_at_parameter_complex(void) {
	print_test_header("Ray Point at Parameter Complex Tests");
	
	// Test with diagonal ray (45 degrees)
	struct wlf_vector2 origin = {0.0, 0.0};
	struct wlf_vector2 direction = {1.0, 1.0}; // Not normalized, but that's ok for testing
	struct wlf_ray2 diagonal_ray = wlf_ray2_make(origin, direction);
	
	struct wlf_vector2 point = wlf_ray2_point_at_parameter(&diagonal_ray, 2.0);
	struct wlf_vector2 expected = {2.0, 2.0};
	ASSERT_VECTOR2_EQ(point, expected, "Diagonal ray point calculation correct");
	
	// Test with normalized direction vector
	double sqrt2 = sqrt(2.0);
	struct wlf_vector2 norm_direction = {1.0/sqrt2, 1.0/sqrt2}; // Normalized diagonal
	struct wlf_ray2 norm_ray = wlf_ray2_make(origin, norm_direction);
	
	struct wlf_vector2 norm_point = wlf_ray2_point_at_parameter(&norm_ray, sqrt2);
	struct wlf_vector2 norm_expected = {1.0, 1.0};
	ASSERT_VECTOR2_EQ(norm_point, norm_expected, "Normalized diagonal ray point calculation correct");
}

static void test_ray_equality(void) {
	print_test_header("Ray Equality Tests");
	
	struct wlf_vector2 origin1 = {1.0, 2.0};
	struct wlf_vector2 direction1 = {3.0, 4.0};
	struct wlf_ray2 ray1 = wlf_ray2_make(origin1, direction1);
	
	struct wlf_vector2 origin2 = {1.0, 2.0};
	struct wlf_vector2 direction2 = {3.0, 4.0};
	struct wlf_ray2 ray2 = wlf_ray2_make(origin2, direction2);
	
	struct wlf_vector2 origin3 = {1.0, 2.0};
	struct wlf_vector2 direction3 = {3.0, 5.0}; // Different direction
	struct wlf_ray2 ray3 = wlf_ray2_make(origin3, direction3);
	
	struct wlf_vector2 origin4 = {2.0, 2.0}; // Different origin
	struct wlf_vector2 direction4 = {3.0, 4.0};
	struct wlf_ray2 ray4 = wlf_ray2_make(origin4, direction4);
	
	ASSERT_TRUE(wlf_ray2_equal(&ray1, &ray2), "Identical rays are equal");
	ASSERT_TRUE(!wlf_ray2_equal(&ray1, &ray3), "Rays with different directions are not equal");
	ASSERT_TRUE(!wlf_ray2_equal(&ray1, &ray4), "Rays with different origins are not equal");
	ASSERT_TRUE(wlf_ray2_equal(&ray1, &ray1), "Ray is equal to itself");
}

static void test_ray_nearly_equal(void) {
	print_test_header("Ray Nearly Equal Tests");
	
	struct wlf_vector2 origin1 = {1.0, 2.0};
	struct wlf_vector2 direction1 = {3.0, 4.0};
	struct wlf_ray2 ray1 = wlf_ray2_make(origin1, direction1);
	
	// Slightly different ray within epsilon
	struct wlf_vector2 origin2 = {1.0000000001, 2.0000000001};
	struct wlf_vector2 direction2 = {3.0000000001, 4.0000000001};
	struct wlf_ray2 ray2 = wlf_ray2_make(origin2, direction2);
	
	// Ray outside epsilon
	struct wlf_vector2 origin3 = {1.001, 2.001};
	struct wlf_vector2 direction3 = {3.001, 4.001};
	struct wlf_ray2 ray3 = wlf_ray2_make(origin3, direction3);
	
	ASSERT_TRUE(wlf_ray2_nearly_equal(&ray1, &ray2, 1e-8), 
		"Nearly identical rays are nearly equal with appropriate epsilon");
	ASSERT_TRUE(!wlf_ray2_nearly_equal(&ray1, &ray3, 1e-8), 
		"Different rays are not nearly equal with small epsilon");
	ASSERT_TRUE(wlf_ray2_nearly_equal(&ray1, &ray3, 0.01), 
		"Different rays are nearly equal with large epsilon");
}

static void test_ray_string_conversion(void) {
	print_test_header("Ray String Conversion Tests");
	
	struct wlf_vector2 origin = {1.5, 2.5};
	struct wlf_vector2 direction = {0.0, 1.0};
	struct wlf_ray2 ray = wlf_ray2_make(origin, direction);
	
	char* ray_str = wlf_ray2_to_str(&ray);
	ASSERT_TRUE(ray_str != NULL, "String conversion returns non-NULL");
	
	if (ray_str) {
		ASSERT_TRUE(strstr(ray_str, "Origin") != NULL, "String contains 'Origin'");
		ASSERT_TRUE(strstr(ray_str, "Direction") != NULL, "String contains 'Direction'");
		printf("Ray string representation: %s\n", ray_str);
		free(ray_str);
	}
	
	// Test NULL ray
	char* null_str = wlf_ray2_to_str(NULL);
	ASSERT_TRUE(null_str != NULL, "NULL ray string conversion returns non-NULL");
	if (null_str) {
		ASSERT_TRUE(strcmp(null_str, "(NULL)") == 0, "NULL ray returns '(NULL)' string");
		free(null_str);
	}
}

static void test_ray_geometric_properties(void) {
	print_test_header("Ray Geometric Properties Tests");
	
	// Test ray through specific points
	struct wlf_vector2 origin = {0.0, 0.0};
	struct wlf_vector2 direction = {1.0, 1.0}; // 45-degree angle
	struct wlf_ray2 ray = wlf_ray2_make(origin, direction);
	
	// Points along the ray should maintain the relationship: point = origin + t * direction
	for (int i = 0; i < 5; i++) {
		double t = i * 0.5;
		struct wlf_vector2 point = wlf_ray2_point_at_parameter(&ray, t);
		
		// For this specific ray, u should equal v at all points
		ASSERT_DOUBLE_EQ(point.u, point.v, "Points on 45-degree ray have equal u and v components");
		
		// Distance from origin should be t * |direction|
		double distance = sqrt(point.u * point.u + point.v * point.v);
		double expected_distance = t * sqrt(2.0); // |direction| = sqrt(1^2 + 1^2) = sqrt(2)
		ASSERT_DOUBLE_EQ(distance, expected_distance, "Distance from origin equals t * |direction|");
	}
}

static void test_ray_edge_cases(void) {
	print_test_header("Ray Edge Cases Tests");
	
	// Test zero direction vector (degenerate ray)
	struct wlf_vector2 origin = {1.0, 2.0};
	struct wlf_vector2 zero_direction = {0.0, 0.0};
	struct wlf_ray2 degenerate_ray = wlf_ray2_make(origin, zero_direction);
	
	// All points along a degenerate ray should be the origin
	struct wlf_vector2 point1 = wlf_ray2_point_at_parameter(&degenerate_ray, 0.0);
	struct wlf_vector2 point2 = wlf_ray2_point_at_parameter(&degenerate_ray, 1.0);
	struct wlf_vector2 point3 = wlf_ray2_point_at_parameter(&degenerate_ray, -5.0);
	
	ASSERT_VECTOR2_EQ(point1, origin, "Degenerate ray point at t=0 is origin");
	ASSERT_VECTOR2_EQ(point2, origin, "Degenerate ray point at t=1 is origin");
	ASSERT_VECTOR2_EQ(point3, origin, "Degenerate ray point at t=-5 is origin");
	
	// Test very small direction vector
	struct wlf_vector2 tiny_direction = {1e-10, 1e-10};
	struct wlf_ray2 tiny_ray = wlf_ray2_make(origin, tiny_direction);
	
	struct wlf_vector2 tiny_point = wlf_ray2_point_at_parameter(&tiny_ray, 1e10);
	struct wlf_vector2 expected_tiny = {1.0 + 1.0, 2.0 + 1.0}; // origin + 1e10 * 1e-10
	ASSERT_VECTOR2_EQ(tiny_point, expected_tiny, "Tiny direction vector with large parameter works");
}

static void test_ray_normalization_considerations(void) {
	print_test_header("Ray Normalization Considerations Tests");
	
	// Test that the API works with both normalized and non-normalized directions
	struct wlf_vector2 origin = {0.0, 0.0};
	
	// Non-normalized direction
	struct wlf_vector2 direction_long = {3.0, 4.0}; // Length = 5
	struct wlf_ray2 ray_long = wlf_ray2_make(origin, direction_long);
	
	// Normalized direction
	struct wlf_vector2 direction_norm = {0.6, 0.8}; // Length = 1
	struct wlf_ray2 ray_norm = wlf_ray2_make(origin, direction_norm);
	
	// At t=1, non-normalized ray goes 5 units, normalized ray goes 1 unit
	struct wlf_vector2 point_long = wlf_ray2_point_at_parameter(&ray_long, 1.0);
	struct wlf_vector2 point_norm = wlf_ray2_point_at_parameter(&ray_norm, 1.0);
	
	ASSERT_VECTOR2_EQ(point_long, direction_long, "Non-normalized ray at t=1 gives direction vector");
	ASSERT_VECTOR2_EQ(point_norm, direction_norm, "Normalized ray at t=1 gives unit direction");
	
	// But at t=5, normalized ray should reach the same point as non-normalized at t=1
	struct wlf_vector2 point_norm_scaled = wlf_ray2_point_at_parameter(&ray_norm, 5.0);
	ASSERT_VECTOR2_EQ(point_norm_scaled, direction_long, "Normalized ray at t=5 equals non-normalized at t=1");
}

int main(void) {
	printf("wlf_ray2 Test Suite\n");
	printf("===================\n");
	
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
	
	return (test_count == passed_tests) ? 0 : 1;
}
