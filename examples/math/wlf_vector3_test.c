#include "wlf/math/wlf_vector3.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);

	wlf_log(WLF_INFO, "=== WLF Vector3 Test Suite ===");

	// Test Vector3 Creation
	wlf_log(WLF_INFO, "\n--- Testing Vector3 Creation ---");

	struct wlf_vector3 v1 = wlf_vector3_make(3.0, 4.0, 5.0);
	struct wlf_vector3 v2 = wlf_vector3_make(-2.5, 1.5, -3.0);
	struct wlf_vector3 zero_vec = WLF_VECTOR3_ZERO;
	struct wlf_vector3 unit_x = WLF_VECTOR3_UNIT_X;
	struct wlf_vector3 unit_y = WLF_VECTOR3_UNIT_Y;
	struct wlf_vector3 unit_z = WLF_VECTOR3_UNIT_Z;

	char *str1 = wlf_vector3_to_str(&v1);
	char *str2 = wlf_vector3_to_str(&v2);
	char *str_zero = wlf_vector3_to_str(&zero_vec);
	char *str_unit_x = wlf_vector3_to_str(&unit_x);
	char *str_unit_y = wlf_vector3_to_str(&unit_y);
	char *str_unit_z = wlf_vector3_to_str(&unit_z);

	wlf_log(WLF_INFO, "v1: %s", str1);
	wlf_log(WLF_INFO, "v2: %s", str2);
	wlf_log(WLF_INFO, "zero: %s", str_zero);
	wlf_log(WLF_INFO, "unit_x: %s", str_unit_x);
	wlf_log(WLF_INFO, "unit_y: %s", str_unit_y);
	wlf_log(WLF_INFO, "unit_z: %s", str_unit_z);

	free(str1); free(str2); free(str_zero);
	free(str_unit_x); free(str_unit_y); free(str_unit_z);

	// Test Constants
	wlf_log(WLF_INFO, "\n--- Testing Constants ---");

	char *zero_str = wlf_vector3_to_str(&WLF_VECTOR3_ZERO);
	char *unit_x_str = wlf_vector3_to_str(&WLF_VECTOR3_UNIT_X);
	char *unit_y_str = wlf_vector3_to_str(&WLF_VECTOR3_UNIT_Y);
	char *unit_z_str = wlf_vector3_to_str(&WLF_VECTOR3_UNIT_Z);

	wlf_log(WLF_INFO, "WLF_VECTOR3_ZERO: %s", zero_str);
	wlf_log(WLF_INFO, "WLF_VECTOR3_UNIT_X: %s", unit_x_str);
	wlf_log(WLF_INFO, "WLF_VECTOR3_UNIT_Y: %s", unit_y_str);
	wlf_log(WLF_INFO, "WLF_VECTOR3_UNIT_Z: %s", unit_z_str);

	free(zero_str); free(unit_x_str); free(unit_y_str); free(unit_z_str);

	// Test Equality
	wlf_log(WLF_INFO, "\n--- Testing Equality ---");

	struct wlf_vector3 v1_copy = wlf_vector3_make(3.0, 4.0, 5.0);
	struct wlf_vector3 v1_approx = wlf_vector3_make(3.00001, 4.00001, 5.00001);

	wlf_log(WLF_INFO, "v1 == v1_copy (exact): %s", wlf_vector3_equal(&v1, &v1_copy) ? "true" : "false");
	wlf_log(WLF_INFO, "v1 == v2 (exact): %s", wlf_vector3_equal(&v1, &v2) ? "true" : "false");
	wlf_log(WLF_INFO, "v1 ≈ v1_approx (ε=0.001): %s", wlf_vector3_nearly_equal(&v1, &v1_approx, 0.001) ? "true" : "false");
	wlf_log(WLF_INFO, "v1 ≈ v1_approx (ε=0.000001): %s", wlf_vector3_nearly_equal(&v1, &v1_approx, 0.000001) ? "true" : "false");

	// Test Arithmetic Operations
	wlf_log(WLF_INFO, "\n--- Testing Arithmetic Operations ---");

	struct wlf_vector3 add_result = wlf_vector3_add(&v1, &v2);
	struct wlf_vector3 sub_result = wlf_vector3_subtract(&v1, &v2);
	struct wlf_vector3 mul_result = wlf_vector3_multiply(&v1, 2.5);
	struct wlf_vector3 div_result = wlf_vector3_divide(&v1, 2.0);

	char *add_str = wlf_vector3_to_str(&add_result);
	char *sub_str = wlf_vector3_to_str(&sub_result);
	char *mul_str = wlf_vector3_to_str(&mul_result);
	char *div_str = wlf_vector3_to_str(&div_result);

	wlf_log(WLF_INFO, "v1 + v2 = %s", add_str);
	wlf_log(WLF_INFO, "v1 - v2 = %s", sub_str);
	wlf_log(WLF_INFO, "v1 * 2.5 = %s", mul_str);
	wlf_log(WLF_INFO, "v1 / 2.0 = %s", div_str);

	free(add_str); free(sub_str); free(mul_str); free(div_str);

	// Test Vector Properties
	wlf_log(WLF_INFO, "\n--- Testing Vector Properties ---");

	double v1_magnitude = wlf_vector3_magnitude(&v1);
	double v2_magnitude = wlf_vector3_magnitude(&v2);
	double zero_magnitude = wlf_vector3_magnitude(&WLF_VECTOR3_ZERO);
	double unit_x_magnitude = wlf_vector3_magnitude(&WLF_VECTOR3_UNIT_X);

	wlf_log(WLF_INFO, "v1 magnitude: %.3f (expected: 7.071)", v1_magnitude);
	wlf_log(WLF_INFO, "v2 magnitude: %.3f", v2_magnitude);
	wlf_log(WLF_INFO, "zero magnitude: %.3f", zero_magnitude);
	wlf_log(WLF_INFO, "unit_x magnitude: %.3f (expected: 1.000)", unit_x_magnitude);

	// Test Dot Product
	wlf_log(WLF_INFO, "\n--- Testing Dot Product ---");

	double dot_v1_v2 = wlf_vector3_dot(&v1, &v2);
	double dot_v1_unit_x = wlf_vector3_dot(&v1, &WLF_VECTOR3_UNIT_X);
	double dot_v1_unit_y = wlf_vector3_dot(&v1, &WLF_VECTOR3_UNIT_Y);
	double dot_v1_unit_z = wlf_vector3_dot(&v1, &WLF_VECTOR3_UNIT_Z);
	double dot_v1_self = wlf_vector3_dot(&v1, &v1);

	wlf_log(WLF_INFO, "v1 · v2 = %.3f", dot_v1_v2);
	wlf_log(WLF_INFO, "v1 · unit_x = %.3f (should equal v1.x = 3.000)", dot_v1_unit_x);
	wlf_log(WLF_INFO, "v1 · unit_y = %.3f (should equal v1.y = 4.000)", dot_v1_unit_y);
	wlf_log(WLF_INFO, "v1 · unit_z = %.3f (should equal v1.z = 5.000)", dot_v1_unit_z);
	wlf_log(WLF_INFO, "v1 · v1 = %.3f (should equal |v1|² = 50.000)", dot_v1_self);

	// Test Cross Product
	wlf_log(WLF_INFO, "\n--- Testing Cross Product ---");

	struct wlf_vector3 cross_v1_v2 = wlf_vector3_cross(&v1, &v2);
	struct wlf_vector3 cross_unit_x_unit_y = wlf_vector3_cross(&WLF_VECTOR3_UNIT_X, &WLF_VECTOR3_UNIT_Y);
	struct wlf_vector3 cross_unit_y_unit_z = wlf_vector3_cross(&WLF_VECTOR3_UNIT_Y, &WLF_VECTOR3_UNIT_Z);
	struct wlf_vector3 cross_unit_z_unit_x = wlf_vector3_cross(&WLF_VECTOR3_UNIT_Z, &WLF_VECTOR3_UNIT_X);

	char *cross_v1_v2_str = wlf_vector3_to_str(&cross_v1_v2);
	char *cross_xy_str = wlf_vector3_to_str(&cross_unit_x_unit_y);
	char *cross_yz_str = wlf_vector3_to_str(&cross_unit_y_unit_z);
	char *cross_zx_str = wlf_vector3_to_str(&cross_unit_z_unit_x);

	wlf_log(WLF_INFO, "v1 × v2 = %s", cross_v1_v2_str);
	wlf_log(WLF_INFO, "unit_x × unit_y = %s (should be unit_z)", cross_xy_str);
	wlf_log(WLF_INFO, "unit_y × unit_z = %s (should be unit_x)", cross_yz_str);
	wlf_log(WLF_INFO, "unit_z × unit_x = %s (should be unit_y)", cross_zx_str);

	free(cross_v1_v2_str); free(cross_xy_str); free(cross_yz_str); free(cross_zx_str);

	// Test Cross Product Properties
	wlf_log(WLF_INFO, "\n--- Testing Cross Product Properties ---");

	// Cross product is orthogonal to both vectors
	double dot_cross_v1 = wlf_vector3_dot(&cross_v1_v2, &v1);
	double dot_cross_v2 = wlf_vector3_dot(&cross_v1_v2, &v2);
	wlf_log(WLF_INFO, "(v1 × v2) · v1 = %.6f (should be 0.000000)", dot_cross_v1);
	wlf_log(WLF_INFO, "(v1 × v2) · v2 = %.6f (should be 0.000000)", dot_cross_v2);

	// Anti-commutativity: a × b = -(b × a)
	struct wlf_vector3 cross_v2_v1 = wlf_vector3_cross(&v2, &v1);
	struct wlf_vector3 neg_cross_v2_v1 = wlf_vector3_multiply(&cross_v2_v1, -1.0);
	bool anticommutative = wlf_vector3_nearly_equal(&cross_v1_v2, &neg_cross_v2_v1, 1e-10);
	wlf_log(WLF_INFO, "Cross product anti-commutative: %s", anticommutative ? "true" : "false");

	// Cross product with self is zero
	struct wlf_vector3 cross_self = wlf_vector3_cross(&v1, &v1);
	bool self_cross_zero = wlf_vector3_nearly_equal(&cross_self, &WLF_VECTOR3_ZERO, 1e-10);
	wlf_log(WLF_INFO, "v1 × v1 = zero: %s", self_cross_zero ? "true" : "false");

	// Test Normalization
	wlf_log(WLF_INFO, "\n--- Testing Normalization ---");

	struct wlf_vector3 v1_normalized = wlf_vector3_normalize(&v1);
	struct wlf_vector3 v2_normalized = wlf_vector3_normalize(&v2);
	struct wlf_vector3 zero_normalized = wlf_vector3_normalize(&WLF_VECTOR3_ZERO);

	char *v1_norm_str = wlf_vector3_to_str(&v1_normalized);
	char *v2_norm_str = wlf_vector3_to_str(&v2_normalized);
	char *zero_norm_str = wlf_vector3_to_str(&zero_normalized);

	wlf_log(WLF_INFO, "v1 normalized: %s", v1_norm_str);
	wlf_log(WLF_INFO, "v2 normalized: %s", v2_norm_str);
	wlf_log(WLF_INFO, "zero normalized: %s", zero_norm_str);

	// Check normalized vector magnitudes
	double v1_norm_mag = wlf_vector3_magnitude(&v1_normalized);
	double v2_norm_mag = wlf_vector3_magnitude(&v2_normalized);
	wlf_log(WLF_INFO, "v1_normalized magnitude: %.6f (should be 1.000000)", v1_norm_mag);
	wlf_log(WLF_INFO, "v2_normalized magnitude: %.6f (should be 1.000000)", v2_norm_mag);

	free(v1_norm_str); free(v2_norm_str); free(zero_norm_str);

	// Test Mathematical Properties
	wlf_log(WLF_INFO, "\n--- Testing Mathematical Properties ---");

	// Test orthogonal vectors (90° angle)
	struct wlf_vector3 ortho1 = wlf_vector3_make(1.0, 0.0, 0.0);
	struct wlf_vector3 ortho2 = wlf_vector3_make(0.0, 1.0, 0.0);
	double ortho_dot = wlf_vector3_dot(&ortho1, &ortho2);
	wlf_log(WLF_INFO, "Orthogonal vectors dot product: %.3f (should be 0.000)", ortho_dot);

	// Test parallel vectors
	struct wlf_vector3 parallel1 = wlf_vector3_make(2.0, 3.0, 4.0);
	struct wlf_vector3 parallel2 = wlf_vector3_make(4.0, 6.0, 8.0);
	double parallel_dot = wlf_vector3_dot(&parallel1, &parallel2);
	double parallel1_mag = wlf_vector3_magnitude(&parallel1);
	double parallel2_mag = wlf_vector3_magnitude(&parallel2);
	double expected_parallel_dot = parallel1_mag * parallel2_mag;
	wlf_log(WLF_INFO, "Parallel vectors dot product: %.3f", parallel_dot);
	wlf_log(WLF_INFO, "Expected (|v1| * |v2|): %.3f", expected_parallel_dot);

	// Test anti-parallel vectors (180° angle)
	struct wlf_vector3 anti1 = wlf_vector3_make(1.0, 2.0, 3.0);
	struct wlf_vector3 anti2 = wlf_vector3_make(-2.0, -4.0, -6.0);
	double anti_dot = wlf_vector3_dot(&anti1, &anti2);
	double anti1_mag = wlf_vector3_magnitude(&anti1);
	double anti2_mag = wlf_vector3_magnitude(&anti2);
	double expected_anti_dot = -anti1_mag * anti2_mag;
	wlf_log(WLF_INFO, "Anti-parallel vectors dot product: %.3f", anti_dot);
	wlf_log(WLF_INFO, "Expected (-|v1| * |v2|): %.3f", expected_anti_dot);

	// Test Edge Cases
	wlf_log(WLF_INFO, "\n--- Testing Edge Cases ---");

	// Division by zero
	struct wlf_vector3 div_by_zero = wlf_vector3_divide(&v1, 0.0);
	char *div_zero_str = wlf_vector3_to_str(&div_by_zero);
	wlf_log(WLF_INFO, "v1 / 0.0 = %s (should equal v1)", div_zero_str);
	free(div_zero_str);

	// Very small numbers
	struct wlf_vector3 tiny = wlf_vector3_make(1e-10, 1e-10, 1e-10);
	double tiny_mag = wlf_vector3_magnitude(&tiny);
	struct wlf_vector3 tiny_norm = wlf_vector3_normalize(&tiny);
	char *tiny_str = wlf_vector3_to_str(&tiny);
	char *tiny_norm_str = wlf_vector3_to_str(&tiny_norm);
	wlf_log(WLF_INFO, "Tiny vector: %s", tiny_str);
	wlf_log(WLF_INFO, "Tiny magnitude: %.12e", tiny_mag);
	wlf_log(WLF_INFO, "Tiny normalized: %s", tiny_norm_str);
	free(tiny_str); free(tiny_norm_str);

	// Very large numbers
	struct wlf_vector3 large = wlf_vector3_make(1e6, 1e6, 1e6);
	double large_mag = wlf_vector3_magnitude(&large);
	struct wlf_vector3 large_norm = wlf_vector3_normalize(&large);
	char *large_str = wlf_vector3_to_str(&large);
	char *large_norm_str = wlf_vector3_to_str(&large_norm);
	wlf_log(WLF_INFO, "Large vector: %s", large_str);
	wlf_log(WLF_INFO, "Large magnitude: %.3e", large_mag);
	wlf_log(WLF_INFO, "Large normalized: %s", large_norm_str);
	free(large_str); free(large_norm_str);

	// Test Epsilon Comparison
	wlf_log(WLF_INFO, "\n--- Testing Epsilon Comparison ---");

	struct wlf_vector3 base = wlf_vector3_make(1.0, 2.0, 3.0);
	struct wlf_vector3 tiny_diff = wlf_vector3_make(1.0000001, 2.0000001, 3.0000001);
	struct wlf_vector3 big_diff = wlf_vector3_make(1.1, 2.1, 3.1);

	double epsilons[] = {1e-10, 1e-6, 1e-3, 0.01, 0.1, 1.0};

	for (int i = 0; i < 6; i++) {
		double eps = epsilons[i];
		bool tiny_equal = wlf_vector3_nearly_equal(&base, &tiny_diff, eps);
		bool big_equal = wlf_vector3_nearly_equal(&base, &big_diff, eps);
		wlf_log(WLF_INFO, "ε=%.0e: tiny_diff=%s, big_diff=%s", eps,
				tiny_equal ? "true" : "false", big_equal ? "true" : "false");
	}

	// Test Vector Algebra Properties
	wlf_log(WLF_INFO, "\n--- Testing Vector Algebra Properties ---");

	// Commutativity: a + b = b + a
	struct wlf_vector3 comm1 = wlf_vector3_add(&v1, &v2);
	struct wlf_vector3 comm2 = wlf_vector3_add(&v2, &v1);
	bool commutative = wlf_vector3_equal(&comm1, &comm2);
	wlf_log(WLF_INFO, "Addition commutative: %s", commutative ? "true" : "false");

	// Associativity: (a + b) + c = a + (b + c)
	struct wlf_vector3 v3 = wlf_vector3_make(1.0, -1.0, 2.0);
	struct wlf_vector3 assoc1_temp = wlf_vector3_add(&v1, &v2);
	struct wlf_vector3 assoc1 = wlf_vector3_add(&assoc1_temp, &v3);
	struct wlf_vector3 assoc2_temp = wlf_vector3_add(&v2, &v3);
	struct wlf_vector3 assoc2 = wlf_vector3_add(&v1, &assoc2_temp);
	bool associative = wlf_vector3_nearly_equal(&assoc1, &assoc2, 1e-10);
	wlf_log(WLF_INFO, "Addition associative: %s", associative ? "true" : "false");

	// Identity: v + 0 = v
	struct wlf_vector3 identity = wlf_vector3_add(&v1, &WLF_VECTOR3_ZERO);
	bool identity_prop = wlf_vector3_equal(&v1, &identity);
	wlf_log(WLF_INFO, "Zero identity: %s", identity_prop ? "true" : "false");

	// Scalar multiplication distributivity: k(a + b) = ka + kb
	double k = 3.5;
	struct wlf_vector3 dist1_temp = wlf_vector3_add(&v1, &v2);
	struct wlf_vector3 dist1 = wlf_vector3_multiply(&dist1_temp, k);
	struct wlf_vector3 dist2_temp1 = wlf_vector3_multiply(&v1, k);
	struct wlf_vector3 dist2_temp2 = wlf_vector3_multiply(&v2, k);
	struct wlf_vector3 dist2 = wlf_vector3_add(&dist2_temp1, &dist2_temp2);
	bool distributive = wlf_vector3_nearly_equal(&dist1, &dist2, 1e-10);
	wlf_log(WLF_INFO, "Scalar multiplication distributive: %s", distributive ? "true" : "false");

	// Test Known Vector Calculations
	wlf_log(WLF_INFO, "\n--- Testing Known Vector Calculations ---");

	// 3-4-5-? Pythagorean-like calculation
	struct wlf_vector3 vec_345 = wlf_vector3_make(3.0, 4.0, 0.0);
	double mag_345 = wlf_vector3_magnitude(&vec_345);
	wlf_log(WLF_INFO, "Vector(3,4,0) magnitude: %.3f (expected: 5.000)", mag_345);

	// Unit cube diagonal
	struct wlf_vector3 unit_cube_diag = wlf_vector3_make(1.0, 1.0, 1.0);
	double unit_cube_mag = wlf_vector3_magnitude(&unit_cube_diag);
	wlf_log(WLF_INFO, "Unit cube diagonal magnitude: %.6f (expected: %.6f)", unit_cube_mag, sqrt(3.0));

	// Right-hand rule test: i × j = k
	bool right_hand_rule = wlf_vector3_equal(&cross_unit_x_unit_y, &WLF_VECTOR3_UNIT_Z);
	wlf_log(WLF_INFO, "Right-hand rule (i × j = k): %s", right_hand_rule ? "true" : "false");

	// Test scalar triple product: a · (b × c)
	struct wlf_vector3 a = wlf_vector3_make(1.0, 2.0, 3.0);
	struct wlf_vector3 b = wlf_vector3_make(4.0, 5.0, 6.0);
	struct wlf_vector3 c = wlf_vector3_make(7.0, 8.0, 9.0);
	struct wlf_vector3 b_cross_c = wlf_vector3_cross(&b, &c);
	double scalar_triple = wlf_vector3_dot(&a, &b_cross_c);
	wlf_log(WLF_INFO, "Scalar triple product a·(b×c): %.3f", scalar_triple);

	// For vectors (1,2,3), (4,5,6), (7,8,9), the scalar triple product should be 0
	// because these vectors are coplanar (linearly dependent)
	wlf_log(WLF_INFO, "Coplanar vectors scalar triple product should be 0.000");

	wlf_log(WLF_INFO, "\n=== Vector3 Test Suite Complete ===");

	return 0;
}
