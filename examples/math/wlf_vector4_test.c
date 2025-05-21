#include "wlf/math/wlf_vector4.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);

	wlf_log(WLF_INFO, "=== WLF Vector4 Test Suite ===");

	// Test Vector4 Creation
	wlf_log(WLF_INFO, "\n--- Testing Vector4 Creation ---");

	struct wlf_vector4 v1 = wlf_vector4_make(3.0, 4.0, 5.0, 6.0);
	struct wlf_vector4 v2 = wlf_vector4_make(-2.5, 1.5, -3.0, 2.0);
	struct wlf_vector4 zero_vec = WLF_VECTOR4_ZERO;
	struct wlf_vector4 unit_x = WLF_VECTOR4_UNIT_X;
	struct wlf_vector4 unit_y = WLF_VECTOR4_UNIT_Y;
	struct wlf_vector4 unit_z = WLF_VECTOR4_UNIT_Z;
	struct wlf_vector4 unit_w = WLF_VECTOR4_UNIT_W;

	char *str1 = wlf_vector4_to_str(&v1);
	char *str2 = wlf_vector4_to_str(&v2);
	char *str_zero = wlf_vector4_to_str(&zero_vec);
	char *str_unit_x = wlf_vector4_to_str(&unit_x);
	char *str_unit_y = wlf_vector4_to_str(&unit_y);
	char *str_unit_z = wlf_vector4_to_str(&unit_z);
	char *str_unit_w = wlf_vector4_to_str(&unit_w);

	wlf_log(WLF_INFO, "v1: %s", str1);
	wlf_log(WLF_INFO, "v2: %s", str2);
	wlf_log(WLF_INFO, "zero: %s", str_zero);
	wlf_log(WLF_INFO, "unit_x: %s", str_unit_x);
	wlf_log(WLF_INFO, "unit_y: %s", str_unit_y);
	wlf_log(WLF_INFO, "unit_z: %s", str_unit_z);
	wlf_log(WLF_INFO, "unit_w: %s", str_unit_w);

	free(str1); free(str2); free(str_zero);
	free(str_unit_x); free(str_unit_y); free(str_unit_z); free(str_unit_w);

	// Test Constants
	wlf_log(WLF_INFO, "\n--- Testing Constants ---");

	char *zero_str = wlf_vector4_to_str(&WLF_VECTOR4_ZERO);
	char *unit_x_str = wlf_vector4_to_str(&WLF_VECTOR4_UNIT_X);
	char *unit_y_str = wlf_vector4_to_str(&WLF_VECTOR4_UNIT_Y);
	char *unit_z_str = wlf_vector4_to_str(&WLF_VECTOR4_UNIT_Z);
	char *unit_w_str = wlf_vector4_to_str(&WLF_VECTOR4_UNIT_W);

	wlf_log(WLF_INFO, "WLF_VECTOR4_ZERO: %s", zero_str);
	wlf_log(WLF_INFO, "WLF_VECTOR4_UNIT_X: %s", unit_x_str);
	wlf_log(WLF_INFO, "WLF_VECTOR4_UNIT_Y: %s", unit_y_str);
	wlf_log(WLF_INFO, "WLF_VECTOR4_UNIT_Z: %s", unit_z_str);
	wlf_log(WLF_INFO, "WLF_VECTOR4_UNIT_W: %s", unit_w_str);

	free(zero_str); free(unit_x_str); free(unit_y_str); free(unit_z_str); free(unit_w_str);

	// Test Equality
	wlf_log(WLF_INFO, "\n--- Testing Equality ---");

	struct wlf_vector4 v1_copy = wlf_vector4_make(3.0, 4.0, 5.0, 6.0);
	struct wlf_vector4 v1_approx = wlf_vector4_make(3.00001, 4.00001, 5.00001, 6.00001);

	wlf_log(WLF_INFO, "v1 == v1_copy (exact): %s", wlf_vector4_equal(&v1, &v1_copy) ? "true" : "false");
	wlf_log(WLF_INFO, "v1 == v2 (exact): %s", wlf_vector4_equal(&v1, &v2) ? "true" : "false");
	wlf_log(WLF_INFO, "v1 ≈ v1_approx (ε=0.001): %s", wlf_vector4_nearly_equal(&v1, &v1_approx, 0.001) ? "true" : "false");
	wlf_log(WLF_INFO, "v1 ≈ v1_approx (ε=0.000001): %s", wlf_vector4_nearly_equal(&v1, &v1_approx, 0.000001) ? "true" : "false");

	// Test Arithmetic Operations
	wlf_log(WLF_INFO, "\n--- Testing Arithmetic Operations ---");

	struct wlf_vector4 add_result = wlf_vector4_add(&v1, &v2);
	struct wlf_vector4 sub_result = wlf_vector4_subtract(&v1, &v2);
	struct wlf_vector4 mul_result = wlf_vector4_multiply(&v1, 2.5);
	struct wlf_vector4 div_result = wlf_vector4_divide(&v1, 2.0);

	char *add_str = wlf_vector4_to_str(&add_result);
	char *sub_str = wlf_vector4_to_str(&sub_result);
	char *mul_str = wlf_vector4_to_str(&mul_result);
	char *div_str = wlf_vector4_to_str(&div_result);

	wlf_log(WLF_INFO, "v1 + v2 = %s", add_str);
	wlf_log(WLF_INFO, "v1 - v2 = %s", sub_str);
	wlf_log(WLF_INFO, "v1 * 2.5 = %s", mul_str);
	wlf_log(WLF_INFO, "v1 / 2.0 = %s", div_str);

	free(add_str); free(sub_str); free(mul_str); free(div_str);

	// Test Vector Properties
	wlf_log(WLF_INFO, "\n--- Testing Vector Properties ---");

	double v1_magnitude = wlf_vector4_magnitude(&v1);
	double v2_magnitude = wlf_vector4_magnitude(&v2);
	double zero_magnitude = wlf_vector4_magnitude(&WLF_VECTOR4_ZERO);
	double unit_x_magnitude = wlf_vector4_magnitude(&WLF_VECTOR4_UNIT_X);

	wlf_log(WLF_INFO, "v1 magnitude: %.3f (expected: 9.274)", v1_magnitude);
	wlf_log(WLF_INFO, "v2 magnitude: %.3f", v2_magnitude);
	wlf_log(WLF_INFO, "zero magnitude: %.3f", zero_magnitude);
	wlf_log(WLF_INFO, "unit_x magnitude: %.3f (expected: 1.000)", unit_x_magnitude);

	// Test Dot Product
	wlf_log(WLF_INFO, "\n--- Testing Dot Product ---");

	double dot_v1_v2 = wlf_vector4_dot(&v1, &v2);
	double dot_v1_unit_x = wlf_vector4_dot(&v1, &WLF_VECTOR4_UNIT_X);
	double dot_v1_unit_y = wlf_vector4_dot(&v1, &WLF_VECTOR4_UNIT_Y);
	double dot_v1_unit_z = wlf_vector4_dot(&v1, &WLF_VECTOR4_UNIT_Z);
	double dot_v1_unit_w = wlf_vector4_dot(&v1, &WLF_VECTOR4_UNIT_W);
	double dot_v1_self = wlf_vector4_dot(&v1, &v1);

	wlf_log(WLF_INFO, "v1 · v2 = %.3f", dot_v1_v2);
	wlf_log(WLF_INFO, "v1 · unit_x = %.3f (should equal v1.x = 3.000)", dot_v1_unit_x);
	wlf_log(WLF_INFO, "v1 · unit_y = %.3f (should equal v1.y = 4.000)", dot_v1_unit_y);
	wlf_log(WLF_INFO, "v1 · unit_z = %.3f (should equal v1.z = 5.000)", dot_v1_unit_z);
	wlf_log(WLF_INFO, "v1 · unit_w = %.3f (should equal v1.w = 6.000)", dot_v1_unit_w);
	wlf_log(WLF_INFO, "v1 · v1 = %.3f (should equal |v1|² = 86.000)", dot_v1_self);

	// Test Normalization
	wlf_log(WLF_INFO, "\n--- Testing Normalization ---");

	struct wlf_vector4 v1_normalized = wlf_vector4_normalize(&v1);
	struct wlf_vector4 v2_normalized = wlf_vector4_normalize(&v2);
	struct wlf_vector4 zero_normalized = wlf_vector4_normalize(&WLF_VECTOR4_ZERO);

	char *v1_norm_str = wlf_vector4_to_str(&v1_normalized);
	char *v2_norm_str = wlf_vector4_to_str(&v2_normalized);
	char *zero_norm_str = wlf_vector4_to_str(&zero_normalized);

	wlf_log(WLF_INFO, "v1 normalized: %s", v1_norm_str);
	wlf_log(WLF_INFO, "v2 normalized: %s", v2_norm_str);
	wlf_log(WLF_INFO, "zero normalized: %s", zero_norm_str);

	// Check normalized vector magnitudes
	double v1_norm_mag = wlf_vector4_magnitude(&v1_normalized);
	double v2_norm_mag = wlf_vector4_magnitude(&v2_normalized);
	wlf_log(WLF_INFO, "v1_normalized magnitude: %.6f (should be 1.000000)", v1_norm_mag);
	wlf_log(WLF_INFO, "v2_normalized magnitude: %.6f (should be 1.000000)", v2_norm_mag);

	free(v1_norm_str); free(v2_norm_str); free(zero_norm_str);

	// Test Mathematical Properties
	wlf_log(WLF_INFO, "\n--- Testing Mathematical Properties ---");

	// Test orthogonal vectors (90° angle)
	struct wlf_vector4 ortho1 = wlf_vector4_make(1.0, 0.0, 0.0, 0.0);
	struct wlf_vector4 ortho2 = wlf_vector4_make(0.0, 1.0, 0.0, 0.0);
	double ortho_dot = wlf_vector4_dot(&ortho1, &ortho2);
	wlf_log(WLF_INFO, "Orthogonal vectors dot product: %.3f (should be 0.000)", ortho_dot);

	// Test parallel vectors
	struct wlf_vector4 parallel1 = wlf_vector4_make(2.0, 3.0, 4.0, 5.0);
	struct wlf_vector4 parallel2 = wlf_vector4_make(4.0, 6.0, 8.0, 10.0);
	double parallel_dot = wlf_vector4_dot(&parallel1, &parallel2);
	double parallel1_mag = wlf_vector4_magnitude(&parallel1);
	double parallel2_mag = wlf_vector4_magnitude(&parallel2);
	double expected_parallel_dot = parallel1_mag * parallel2_mag;
	wlf_log(WLF_INFO, "Parallel vectors dot product: %.3f", parallel_dot);
	wlf_log(WLF_INFO, "Expected (|v1| * |v2|): %.3f", expected_parallel_dot);

	// Test anti-parallel vectors (180° angle)
	struct wlf_vector4 anti1 = wlf_vector4_make(1.0, 2.0, 3.0, 4.0);
	struct wlf_vector4 anti2 = wlf_vector4_make(-2.0, -4.0, -6.0, -8.0);
	double anti_dot = wlf_vector4_dot(&anti1, &anti2);
	double anti1_mag = wlf_vector4_magnitude(&anti1);
	double anti2_mag = wlf_vector4_magnitude(&anti2);
	double expected_anti_dot = -anti1_mag * anti2_mag;
	wlf_log(WLF_INFO, "Anti-parallel vectors dot product: %.3f", anti_dot);
	wlf_log(WLF_INFO, "Expected (-|v1| * |v2|): %.3f", expected_anti_dot);

	// Test Edge Cases
	wlf_log(WLF_INFO, "\n--- Testing Edge Cases ---");

	// Division by zero
	struct wlf_vector4 div_by_zero = wlf_vector4_divide(&v1, 0.0);
	char *div_zero_str = wlf_vector4_to_str(&div_by_zero);
	wlf_log(WLF_INFO, "v1 / 0.0 = %s (should equal v1)", div_zero_str);
	free(div_zero_str);

	// Very small numbers
	struct wlf_vector4 tiny = wlf_vector4_make(1e-10, 1e-10, 1e-10, 1e-10);
	double tiny_mag = wlf_vector4_magnitude(&tiny);
	struct wlf_vector4 tiny_norm = wlf_vector4_normalize(&tiny);
	char *tiny_str = wlf_vector4_to_str(&tiny);
	char *tiny_norm_str = wlf_vector4_to_str(&tiny_norm);
	wlf_log(WLF_INFO, "Tiny vector: %s", tiny_str);
	wlf_log(WLF_INFO, "Tiny magnitude: %.12e", tiny_mag);
	wlf_log(WLF_INFO, "Tiny normalized: %s", tiny_norm_str);
	free(tiny_str); free(tiny_norm_str);

	// Very large numbers
	struct wlf_vector4 large = wlf_vector4_make(1e6, 1e6, 1e6, 1e6);
	double large_mag = wlf_vector4_magnitude(&large);
	struct wlf_vector4 large_norm = wlf_vector4_normalize(&large);
	char *large_str = wlf_vector4_to_str(&large);
	char *large_norm_str = wlf_vector4_to_str(&large_norm);
	wlf_log(WLF_INFO, "Large vector: %s", large_str);
	wlf_log(WLF_INFO, "Large magnitude: %.3e", large_mag);
	wlf_log(WLF_INFO, "Large normalized: %s", large_norm_str);
	free(large_str); free(large_norm_str);

	// Test Epsilon Comparison
	wlf_log(WLF_INFO, "\n--- Testing Epsilon Comparison ---");

	struct wlf_vector4 base = wlf_vector4_make(1.0, 2.0, 3.0, 4.0);
	struct wlf_vector4 tiny_diff = wlf_vector4_make(1.0000001, 2.0000001, 3.0000001, 4.0000001);
	struct wlf_vector4 big_diff = wlf_vector4_make(1.1, 2.1, 3.1, 4.1);

	double epsilons[] = {1e-10, 1e-6, 1e-3, 0.01, 0.1, 1.0};

	for (int i = 0; i < 6; i++) {
		double eps = epsilons[i];
		bool tiny_equal = wlf_vector4_nearly_equal(&base, &tiny_diff, eps);
		bool big_equal = wlf_vector4_nearly_equal(&base, &big_diff, eps);
		wlf_log(WLF_INFO, "ε=%.0e: tiny_diff=%s, big_diff=%s", eps,
				tiny_equal ? "true" : "false", big_equal ? "true" : "false");
	}

	// Test Vector Algebra Properties
	wlf_log(WLF_INFO, "\n--- Testing Vector Algebra Properties ---");

	// Commutativity: a + b = b + a
	struct wlf_vector4 comm1 = wlf_vector4_add(&v1, &v2);
	struct wlf_vector4 comm2 = wlf_vector4_add(&v2, &v1);
	bool commutative = wlf_vector4_equal(&comm1, &comm2);
	wlf_log(WLF_INFO, "Addition commutative: %s", commutative ? "true" : "false");

	// Associativity: (a + b) + c = a + (b + c)
	struct wlf_vector4 v3 = wlf_vector4_make(1.0, -1.0, 2.0, -2.0);
	struct wlf_vector4 assoc1_temp = wlf_vector4_add(&v1, &v2);
	struct wlf_vector4 assoc1 = wlf_vector4_add(&assoc1_temp, &v3);
	struct wlf_vector4 assoc2_temp = wlf_vector4_add(&v2, &v3);
	struct wlf_vector4 assoc2 = wlf_vector4_add(&v1, &assoc2_temp);
	bool associative = wlf_vector4_nearly_equal(&assoc1, &assoc2, 1e-10);
	wlf_log(WLF_INFO, "Addition associative: %s", associative ? "true" : "false");

	// Identity: v + 0 = v
	struct wlf_vector4 identity = wlf_vector4_add(&v1, &WLF_VECTOR4_ZERO);
	bool identity_prop = wlf_vector4_equal(&v1, &identity);
	wlf_log(WLF_INFO, "Zero identity: %s", identity_prop ? "true" : "false");

	// Scalar multiplication distributivity: k(a + b) = ka + kb
	double k = 3.5;
	struct wlf_vector4 dist1_temp = wlf_vector4_add(&v1, &v2);
	struct wlf_vector4 dist1 = wlf_vector4_multiply(&dist1_temp, k);
	struct wlf_vector4 dist2_temp1 = wlf_vector4_multiply(&v1, k);
	struct wlf_vector4 dist2_temp2 = wlf_vector4_multiply(&v2, k);
	struct wlf_vector4 dist2 = wlf_vector4_add(&dist2_temp1, &dist2_temp2);
	bool distributive = wlf_vector4_nearly_equal(&dist1, &dist2, 1e-10);
	wlf_log(WLF_INFO, "Scalar multiplication distributive: %s", distributive ? "true" : "false");

	// Test Known Vector Calculations
	wlf_log(WLF_INFO, "\n--- Testing Known Vector Calculations ---");

	// Unit hypercube diagonal (1,1,1,1)
	struct wlf_vector4 unit_hypercube_diag = wlf_vector4_make(1.0, 1.0, 1.0, 1.0);
	double unit_hypercube_mag = wlf_vector4_magnitude(&unit_hypercube_diag);
	wlf_log(WLF_INFO, "Unit hypercube diagonal magnitude: %.6f (expected: %.6f)", unit_hypercube_mag, sqrt(4.0));

	// Test all unit vectors orthogonality
	wlf_log(WLF_INFO, "\n--- Testing Unit Vector Orthogonality ---");

	double dot_x_y = wlf_vector4_dot(&WLF_VECTOR4_UNIT_X, &WLF_VECTOR4_UNIT_Y);
	double dot_x_z = wlf_vector4_dot(&WLF_VECTOR4_UNIT_X, &WLF_VECTOR4_UNIT_Z);
	double dot_x_w = wlf_vector4_dot(&WLF_VECTOR4_UNIT_X, &WLF_VECTOR4_UNIT_W);
	double dot_y_z = wlf_vector4_dot(&WLF_VECTOR4_UNIT_Y, &WLF_VECTOR4_UNIT_Z);
	double dot_y_w = wlf_vector4_dot(&WLF_VECTOR4_UNIT_Y, &WLF_VECTOR4_UNIT_W);
	double dot_z_w = wlf_vector4_dot(&WLF_VECTOR4_UNIT_Z, &WLF_VECTOR4_UNIT_W);

	wlf_log(WLF_INFO, "unit_x · unit_y = %.3f (should be 0.000)", dot_x_y);
	wlf_log(WLF_INFO, "unit_x · unit_z = %.3f (should be 0.000)", dot_x_z);
	wlf_log(WLF_INFO, "unit_x · unit_w = %.3f (should be 0.000)", dot_x_w);
	wlf_log(WLF_INFO, "unit_y · unit_z = %.3f (should be 0.000)", dot_y_z);
	wlf_log(WLF_INFO, "unit_y · unit_w = %.3f (should be 0.000)", dot_y_w);
	wlf_log(WLF_INFO, "unit_z · unit_w = %.3f (should be 0.000)", dot_z_w);

	// Test homogeneous coordinates (commonly used in 3D graphics)
	wlf_log(WLF_INFO, "\n--- Testing Homogeneous Coordinates ---");

	// Point in 3D space represented as 4D homogeneous coordinates (x, y, z, 1)
	struct wlf_vector4 point_3d = wlf_vector4_make(3.0, 4.0, 5.0, 1.0);
	// Vector in 3D space represented as 4D homogeneous coordinates (x, y, z, 0)
	struct wlf_vector4 vector_3d = wlf_vector4_make(1.0, 1.0, 1.0, 0.0);

	char *point_str = wlf_vector4_to_str(&point_3d);
	char *vector_str = wlf_vector4_to_str(&vector_3d);
	wlf_log(WLF_INFO, "3D point as homogeneous: %s", point_str);
	wlf_log(WLF_INFO, "3D vector as homogeneous: %s", vector_str);
	free(point_str); free(vector_str);

	// Test quaternion-like operations (though this is not a true quaternion)
	wlf_log(WLF_INFO, "\n--- Testing Quaternion-like Vector ---");

	// A quaternion-like vector (not actual quaternion math, just testing 4D properties)
	struct wlf_vector4 quat_like = wlf_vector4_make(0.5, 0.5, 0.5, 0.5);
	double quat_mag = wlf_vector4_magnitude(&quat_like);
	struct wlf_vector4 quat_normalized = wlf_vector4_normalize(&quat_like);

	char *quat_str = wlf_vector4_to_str(&quat_like);
	char *quat_norm_str = wlf_vector4_to_str(&quat_normalized);
	wlf_log(WLF_INFO, "Quaternion-like vector: %s", quat_str);
	wlf_log(WLF_INFO, "Magnitude: %.6f (expected: 1.000000)", quat_mag);
	wlf_log(WLF_INFO, "Normalized: %s", quat_norm_str);
	free(quat_str); free(quat_norm_str);

	// Test color vector (RGBA)
	wlf_log(WLF_INFO, "\n--- Testing Color Vector (RGBA) ---");

	struct wlf_vector4 color_red = wlf_vector4_make(1.0, 0.0, 0.0, 1.0);
	struct wlf_vector4 color_green = wlf_vector4_make(0.0, 1.0, 0.0, 0.5);
	struct wlf_vector4 color_blend = wlf_vector4_add(&color_red, &color_green);
	struct wlf_vector4 color_average = wlf_vector4_divide(&color_blend, 2.0);

	char *red_str = wlf_vector4_to_str(&color_red);
	char *green_str = wlf_vector4_to_str(&color_green);
	char *blend_str = wlf_vector4_to_str(&color_blend);
	char *avg_str = wlf_vector4_to_str(&color_average);

	wlf_log(WLF_INFO, "Red color (RGBA): %s", red_str);
	wlf_log(WLF_INFO, "Green color (RGBA): %s", green_str);
	wlf_log(WLF_INFO, "Color blend: %s", blend_str);
	wlf_log(WLF_INFO, "Color average: %s", avg_str);

	free(red_str); free(green_str); free(blend_str); free(avg_str);

	wlf_log(WLF_INFO, "\n=== Vector4 Test Suite Complete ===");

	return 0;
}
