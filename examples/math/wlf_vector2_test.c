#include "wlf/math/wlf_vector2.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);

	wlf_log(WLF_INFO, "=== WLF Vector2 Test Suite ===");

	// Test Vector2 Creation
	wlf_log(WLF_INFO, "\n--- Testing Vector2 Creation ---");

	struct wlf_vector2 v1 = (struct wlf_vector2) {.u = 3.0, .v = 4.0};
	struct wlf_vector2 v2 = (struct wlf_vector2) {.u = -2.5, .v = 1.5};
	struct wlf_vector2 zero_vec = WLF_VECTOR2_ZERO;
	struct wlf_vector2 unit_u = WLF_VECTOR2_UNIT_U;
	struct wlf_vector2 unit_v = WLF_VECTOR2_UNIT_V;

	char *str1 = wlf_vector2_to_str(&v1);
	char *str2 = wlf_vector2_to_str(&v2);
	char *str_zero = wlf_vector2_to_str(&zero_vec);
	char *str_unit_u = wlf_vector2_to_str(&unit_u);
	char *str_unit_v = wlf_vector2_to_str(&unit_v);

	wlf_log(WLF_INFO, "v1: %s", str1);
	wlf_log(WLF_INFO, "v2: %s", str2);
	wlf_log(WLF_INFO, "zero: %s", str_zero);
	wlf_log(WLF_INFO, "unit_u: %s", str_unit_u);
	wlf_log(WLF_INFO, "unit_v: %s", str_unit_v);

	free(str1); free(str2); free(str_zero); free(str_unit_u); free(str_unit_v);

	// Test Constants
	wlf_log(WLF_INFO, "\n--- Testing Constants ---");

	char *zero_str = wlf_vector2_to_str(&WLF_VECTOR2_ZERO);
	char *unit_u_str = wlf_vector2_to_str(&WLF_VECTOR2_UNIT_U);
	char *unit_v_str = wlf_vector2_to_str(&WLF_VECTOR2_UNIT_V);

	wlf_log(WLF_INFO, "WLF_VECTOR2_ZERO: %s", zero_str);
	wlf_log(WLF_INFO, "WLF_VECTOR2_UNIT_U: %s", unit_u_str);
	wlf_log(WLF_INFO, "WLF_VECTOR2_UNIT_V: %s", unit_v_str);

	free(zero_str); free(unit_u_str); free(unit_v_str);

	// Test Equality
	wlf_log(WLF_INFO, "\n--- Testing Equality ---");

	struct wlf_vector2 v1_copy = (struct wlf_vector2) {.u = 3.0, .v = 4.0};
	struct wlf_vector2 v1_approx = (struct wlf_vector2) {.u = 3.00001, .v = 4.00001};

	wlf_log(WLF_INFO, "v1 == v1_copy (exact): %s", wlf_vector2_equal(&v1, &v1_copy) ? "true" : "false");
	wlf_log(WLF_INFO, "v1 == v2 (exact): %s", wlf_vector2_equal(&v1, &v2) ? "true" : "false");
	wlf_log(WLF_INFO, "v1 ≈ v1_approx (ε=0.001): %s", wlf_vector2_nearly_equal(&v1, &v1_approx, 0.001) ? "true" : "false");
	wlf_log(WLF_INFO, "v1 ≈ v1_approx (ε=0.000001): %s", wlf_vector2_nearly_equal(&v1, &v1_approx, 0.000001) ? "true" : "false");

	// Test Arithmetic Operations
	wlf_log(WLF_INFO, "\n--- Testing Arithmetic Operations ---");

	struct wlf_vector2 add_result = wlf_vector2_add(&v1, &v2);
	struct wlf_vector2 sub_result = wlf_vector2_subtract(&v1, &v2);
	struct wlf_vector2 mul_result = wlf_vector2_multiply(&v1, 2.5);
	struct wlf_vector2 div_result = wlf_vector2_divide(&v1, 2.0);

	char *add_str = wlf_vector2_to_str(&add_result);
	char *sub_str = wlf_vector2_to_str(&sub_result);
	char *mul_str = wlf_vector2_to_str(&mul_result);
	char *div_str = wlf_vector2_to_str(&div_result);

	wlf_log(WLF_INFO, "v1 + v2 = %s", add_str);
	wlf_log(WLF_INFO, "v1 - v2 = %s", sub_str);
	wlf_log(WLF_INFO, "v1 * 2.5 = %s", mul_str);
	wlf_log(WLF_INFO, "v1 / 2.0 = %s", div_str);

	free(add_str); free(sub_str); free(mul_str); free(div_str);

	// Test Vector Properties
	wlf_log(WLF_INFO, "\n--- Testing Vector Properties ---");

	double v1_magnitude = wlf_vector2_magnitude(&v1);
	double v2_magnitude = wlf_vector2_magnitude(&v2);
	double zero_magnitude = wlf_vector2_magnitude(&WLF_VECTOR2_ZERO);
	double unit_u_magnitude = wlf_vector2_magnitude(&WLF_VECTOR2_UNIT_U);

	wlf_log(WLF_INFO, "v1 magnitude: %.3f (expected: 5.000)", v1_magnitude);
	wlf_log(WLF_INFO, "v2 magnitude: %.3f", v2_magnitude);
	wlf_log(WLF_INFO, "zero magnitude: %.3f", zero_magnitude);
	wlf_log(WLF_INFO, "unit_u magnitude: %.3f (expected: 1.000)", unit_u_magnitude);

	// Test Dot Product
	wlf_log(WLF_INFO, "\n--- Testing Dot Product ---");

	double dot_v1_v2 = wlf_vector2_dot(&v1, &v2);
	double dot_v1_unit_u = wlf_vector2_dot(&v1, &WLF_VECTOR2_UNIT_U);
	double dot_v1_unit_v = wlf_vector2_dot(&v1, &WLF_VECTOR2_UNIT_V);
	double dot_v1_self = wlf_vector2_dot(&v1, &v1);

	wlf_log(WLF_INFO, "v1 · v2 = %.3f", dot_v1_v2);
	wlf_log(WLF_INFO, "v1 · unit_u = %.3f (should equal v1.u = 3.000)", dot_v1_unit_u);
	wlf_log(WLF_INFO, "v1 · unit_v = %.3f (should equal v1.v = 4.000)", dot_v1_unit_v);
	wlf_log(WLF_INFO, "v1 · v1 = %.3f (should equal |v1|² = 25.000)", dot_v1_self);

	// Test Normalization
	wlf_log(WLF_INFO, "\n--- Testing Normalization ---");

	struct wlf_vector2 v1_normalized = wlf_vector2_normalize(&v1);
	struct wlf_vector2 v2_normalized = wlf_vector2_normalize(&v2);
	struct wlf_vector2 zero_normalized = wlf_vector2_normalize(&WLF_VECTOR2_ZERO);

	char *v1_norm_str = wlf_vector2_to_str(&v1_normalized);
	char *v2_norm_str = wlf_vector2_to_str(&v2_normalized);
	char *zero_norm_str = wlf_vector2_to_str(&zero_normalized);

	wlf_log(WLF_INFO, "v1 normalized: %s", v1_norm_str);
	wlf_log(WLF_INFO, "v2 normalized: %s", v2_norm_str);
	wlf_log(WLF_INFO, "zero normalized: %s", zero_norm_str);

	// Check normalized vector magnitudes
	double v1_norm_mag = wlf_vector2_magnitude(&v1_normalized);
	double v2_norm_mag = wlf_vector2_magnitude(&v2_normalized);
	wlf_log(WLF_INFO, "v1_normalized magnitude: %.6f (should be 1.000000)", v1_norm_mag);
	wlf_log(WLF_INFO, "v2_normalized magnitude: %.6f (should be 1.000000)", v2_norm_mag);

	free(v1_norm_str); free(v2_norm_str); free(zero_norm_str);

	// Test Mathematical Properties
	wlf_log(WLF_INFO, "\n--- Testing Mathematical Properties ---");

	// Test orthogonal vectors (90° angle)
	struct wlf_vector2 ortho1 = (struct wlf_vector2) {.u = 1.0, .v = 0.0};
	struct wlf_vector2 ortho2 = (struct wlf_vector2) {.u = 0.0, .v = 1.0};
	double ortho_dot = wlf_vector2_dot(&ortho1, &ortho2);
	wlf_log(WLF_INFO, "Orthogonal vectors dot product: %.3f (should be 0.000)", ortho_dot);

	// Test parallel vectors
	struct wlf_vector2 parallel1 = (struct wlf_vector2) {.u = 2.0, .v = 3.0};
	struct wlf_vector2 parallel2 = (struct wlf_vector2) {.u = 4.0, .v = 6.0};
	double parallel_dot = wlf_vector2_dot(&parallel1, &parallel2);
	double parallel1_mag = wlf_vector2_magnitude(&parallel1);
	double parallel2_mag = wlf_vector2_magnitude(&parallel2);
	double expected_parallel_dot = parallel1_mag * parallel2_mag;
	wlf_log(WLF_INFO, "Parallel vectors dot product: %.3f", parallel_dot);
	wlf_log(WLF_INFO, "Expected (|v1| * |v2|): %.3f", expected_parallel_dot);

	// Test anti-parallel vectors (180° angle)
	struct wlf_vector2 anti1 = (struct wlf_vector2) {.u = 1.0, .v = 2.0};
	struct wlf_vector2 anti2 = (struct wlf_vector2) {.u = -2.0, .v = -4.0};
	double anti_dot = wlf_vector2_dot(&anti1, &anti2);
	double anti1_mag = wlf_vector2_magnitude(&anti1);
	double anti2_mag = wlf_vector2_magnitude(&anti2);
	double expected_anti_dot = -anti1_mag * anti2_mag;
	wlf_log(WLF_INFO, "Anti-parallel vectors dot product: %.3f", anti_dot);
	wlf_log(WLF_INFO, "Expected (-|v1| * |v2|): %.3f", expected_anti_dot);

	// Test Edge Cases
	wlf_log(WLF_INFO, "\n--- Testing Edge Cases ---");

	// Very small numbers
	struct wlf_vector2 tiny = (struct wlf_vector2) {.u = 1e-10, .v = 1e-10};
	double tiny_mag = wlf_vector2_magnitude(&tiny);
	struct wlf_vector2 tiny_norm = wlf_vector2_normalize(&tiny);
	char *tiny_str = wlf_vector2_to_str(&tiny);
	char *tiny_norm_str = wlf_vector2_to_str(&tiny_norm);
	wlf_log(WLF_INFO, "Tiny vector: %s", tiny_str);
	wlf_log(WLF_INFO, "Tiny magnitude: %.12e", tiny_mag);
	wlf_log(WLF_INFO, "Tiny normalized: %s", tiny_norm_str);
	free(tiny_str); free(tiny_norm_str);

	// Very large numbers
	struct wlf_vector2 large = (struct wlf_vector2) {.u = 1e6, .v = 1e6};
	double large_mag = wlf_vector2_magnitude(&large);
	struct wlf_vector2 large_norm = wlf_vector2_normalize(&large);
	char *large_str = wlf_vector2_to_str(&large);
	char *large_norm_str = wlf_vector2_to_str(&large_norm);
	wlf_log(WLF_INFO, "Large vector: %s", large_str);
	wlf_log(WLF_INFO, "Large magnitude: %.3e", large_mag);
	wlf_log(WLF_INFO, "Large normalized: %s", large_norm_str);
	free(large_str); free(large_norm_str);

	// Test Epsilon Comparison
	wlf_log(WLF_INFO, "\n--- Testing Epsilon Comparison ---");

	struct wlf_vector2 base = (struct wlf_vector2) {.u = 1.0, .v = 2.0};
	struct wlf_vector2 tiny_diff = (struct wlf_vector2) {.u = 1.0000001, .v = 2.0000001};
	struct wlf_vector2 big_diff = (struct wlf_vector2) {.u = 1.1, .v = 2.1};

	double epsilons[] = {1e-10, 1e-6, 1e-3, 0.01, 0.1, 1.0};

	for (int i = 0; i < 6; i++) {
		double eps = epsilons[i];
		bool tiny_equal = wlf_vector2_nearly_equal(&base, &tiny_diff, eps);
		bool big_equal = wlf_vector2_nearly_equal(&base, &big_diff, eps);
		wlf_log(WLF_INFO, "ε=%.0e: tiny_diff=%s, big_diff=%s", eps,
				tiny_equal ? "true" : "false", big_equal ? "true" : "false");
	}

	// Test Vector Algebra Properties
	wlf_log(WLF_INFO, "\n--- Testing Vector Algebra Properties ---");

	// Commutativity: a + b = b + a
	struct wlf_vector2 comm1 = wlf_vector2_add(&v1, &v2);
	struct wlf_vector2 comm2 = wlf_vector2_add(&v2, &v1);
	bool commutative = wlf_vector2_equal(&comm1, &comm2);
	wlf_log(WLF_INFO, "Addition commutative: %s", commutative ? "true" : "false");

	// Associativity: (a + b) + c = a + (b + c)
	struct wlf_vector2 v3 = (struct wlf_vector2) {.u = 1.0, .v = -1.0};
	struct wlf_vector2 assoc1_temp = wlf_vector2_add(&v1, &v2);
	struct wlf_vector2 assoc1 = wlf_vector2_add(&assoc1_temp, &v3);
	struct wlf_vector2 assoc2_temp = wlf_vector2_add(&v2, &v3);
	struct wlf_vector2 assoc2 = wlf_vector2_add(&v1, &assoc2_temp);
	bool associative = wlf_vector2_nearly_equal(&assoc1, &assoc2, 1e-10);
	wlf_log(WLF_INFO, "Addition associative: %s", associative ? "true" : "false");

	// Identity: v + 0 = v
	struct wlf_vector2 identity = wlf_vector2_add(&v1, &WLF_VECTOR2_ZERO);
	bool identity_prop = wlf_vector2_equal(&v1, &identity);
	wlf_log(WLF_INFO, "Zero identity: %s", identity_prop ? "true" : "false");

	// Scalar multiplication distributivity: k(a + b) = ka + kb
	double k = 3.5;
	struct wlf_vector2 dist1_temp = wlf_vector2_add(&v1, &v2);
	struct wlf_vector2 dist1 = wlf_vector2_multiply(&dist1_temp, k);
	struct wlf_vector2 dist2_temp1 = wlf_vector2_multiply(&v1, k);
	struct wlf_vector2 dist2_temp2 = wlf_vector2_multiply(&v2, k);
	struct wlf_vector2 dist2 = wlf_vector2_add(&dist2_temp1, &dist2_temp2);
	bool distributive = wlf_vector2_nearly_equal(&dist1, &dist2, 1e-10);
	wlf_log(WLF_INFO, "Scalar multiplication distributive: %s", distributive ? "true" : "false");

	// Test Known Vector Calculations
	wlf_log(WLF_INFO, "\n--- Testing Known Vector Calculations ---");

	// 3-4-5 right triangle
	struct wlf_vector2 vec_3_4 = (struct wlf_vector2) {.u =3.0, .v = 4.0};
	double mag_3_4 = wlf_vector2_magnitude(&vec_3_4);
	wlf_log(WLF_INFO, "Vector(3,4) magnitude: %.3f (expected: 5.000)", mag_3_4);

	// 45-degree angle vectors
	struct wlf_vector2 vec_45_1 = (struct wlf_vector2) {.u = 1.0, .v = 1.0};
	struct wlf_vector2 vec_45_2 = (struct wlf_vector2) { .u = 1.0, .v = 0.0};
	double dot_45 = wlf_vector2_dot(&vec_45_1, &vec_45_2);
	double mag_45_1 = wlf_vector2_magnitude(&vec_45_1);
	double mag_45_2 = wlf_vector2_magnitude(&vec_45_2);
	double cos_45 = dot_45 / (mag_45_1 * mag_45_2);
	wlf_log(WLF_INFO, "45° angle cosine: %.6f (expected: 0.707107)", cos_45);

	wlf_log(WLF_INFO, "\n=== Vector2 Test Suite Complete ===");

	return 0;
}
