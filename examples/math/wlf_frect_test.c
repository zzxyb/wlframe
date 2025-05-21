#include "wlf/math/wlf_frect.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.7182818284590452354
#endif

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);

	wlf_log(WLF_INFO, "=== WLF Floating-Point Rectangle Test Suite ===");

	// Test Rectangle Creation
	wlf_log(WLF_INFO, "\n--- Testing Floating-Point Rectangle Creation ---");

	struct wlf_frect r1 = wlf_frect_make(10.5, 20.3, 100.7, 80.2);
	struct wlf_frect r2 = wlf_frect_make(-5.1, -10.8, 50.4, 40.9);
	struct wlf_frect zero_rect = WLF_FRECT_ZERO;
	struct wlf_frect unit_rect = WLF_FRECT_UNIT;

	char *str1 = wlf_frect_to_str_prec(&r1, 3);
	char *str2 = wlf_frect_to_str_prec(&r2, 3);
	char *str_zero = wlf_frect_to_str_prec(&zero_rect, 1);
	char *str_unit = wlf_frect_to_str_prec(&unit_rect, 1);

	wlf_log(WLF_INFO, "r1: %s", str1);
	wlf_log(WLF_INFO, "r2: %s", str2);
	wlf_log(WLF_INFO, "zero: %s", str_zero);
	wlf_log(WLF_INFO, "unit: %s", str_unit);

	free(str1); free(str2); free(str_zero); free(str_unit);

	// Test Constants
	wlf_log(WLF_INFO, "\n--- Testing Constants ---");

	char *zero_str = wlf_frect_to_str_prec(&WLF_FRECT_ZERO, 1);
	char *unit_str = wlf_frect_to_str_prec(&WLF_FRECT_UNIT, 1);
	wlf_log(WLF_INFO, "WLF_FRECT_ZERO: %s", zero_str);
	wlf_log(WLF_INFO, "WLF_FRECT_UNIT: %s", unit_str);
	free(zero_str); free(unit_str);

	// Test Precision Control
	wlf_log(WLF_INFO, "\n--- Testing Precision Control ---");

	struct wlf_frect precise = wlf_frect_make(3.141592653589793, 2.718281828459045, 1.414213562373095, 1.732050807568877);

	for (int prec = 0; prec <= 6; prec++) {
		char *prec_str = wlf_frect_to_str_prec(&precise, prec);
		wlf_log(WLF_INFO, "Precision %d: %s", prec, prec_str);
		free(prec_str);
	}

	// Test Equality
	wlf_log(WLF_INFO, "\n--- Testing Equality ---");

	struct wlf_frect r1_copy = wlf_frect_make(10.5, 20.3, 100.7, 80.2);
	struct wlf_frect r1_approx = wlf_frect_make(10.500001, 20.300001, 100.700001, 80.200001);

	wlf_log(WLF_INFO, "r1 == r1_copy (exact): %s", wlf_frect_equal(&r1, &r1_copy) ? "true" : "false");
	wlf_log(WLF_INFO, "r1 == r2 (exact): %s", wlf_frect_equal(&r1, &r2) ? "true" : "false");
	wlf_log(WLF_INFO, "r1 ≈ r1_approx (ε=0.001): %s", wlf_frect_nearly_equal(&r1, &r1_approx, 0.001) ? "true" : "false");
	wlf_log(WLF_INFO, "r1 ≈ r1_approx (ε=0.000001): %s", wlf_frect_nearly_equal(&r1, &r1_approx, 0.000001) ? "true" : "false");

	// Test Conversions
	wlf_log(WLF_INFO, "\n--- Testing Conversions ---");

	// Integer to floating-point
	struct wlf_rect int_rect = wlf_rect_make(15, 25, 80, 60);
	struct wlf_frect from_int = wlf_rect_to_frect(&int_rect);

	char *int_str = wlf_rect_to_str(&int_rect);
	char *from_int_str = wlf_frect_to_str_prec(&from_int, 1);
	wlf_log(WLF_INFO, "Integer rect: %s", int_str);
	wlf_log(WLF_INFO, "To floating-point: %s", from_int_str);
	free(int_str); free(from_int_str);

	// Floating-point to integer (basic conversion)
	struct wlf_frect float_rect = wlf_frect_make(10.7, 20.3, 100.9, 80.1);
	struct wlf_rect to_int = wlf_frect_to_rect(&float_rect);

	char *float_str = wlf_frect_to_str_prec(&float_rect, 1);
	char *to_int_str = wlf_rect_to_str(&to_int);
	wlf_log(WLF_INFO, "Floating-point rect: %s", float_str);
	wlf_log(WLF_INFO, "To integer (truncated): %s", to_int_str);
	free(float_str); free(to_int_str);

	// Test Rounding Operations
	wlf_log(WLF_INFO, "\n--- Testing Rounding Operations ---");

	struct wlf_frect test_round = wlf_frect_make(10.3, 20.7, 100.2, 80.8);

	struct wlf_rect rounded = wlf_frect_round(&test_round);
	struct wlf_rect floored = wlf_frect_floor(&test_round);
	struct wlf_rect ceiled = wlf_frect_ceil(&test_round);

	char *test_str = wlf_frect_to_str_prec(&test_round, 1);
	char *round_str = wlf_rect_to_str(&rounded);
	char *floor_str = wlf_rect_to_str(&floored);
	char *ceil_str = wlf_rect_to_str(&ceiled);

	wlf_log(WLF_INFO, "Original: %s", test_str);
	wlf_log(WLF_INFO, "Rounded: %s", round_str);
	wlf_log(WLF_INFO, "Floored: %s", floor_str);
	wlf_log(WLF_INFO, "Ceiled: %s", ceil_str);

	free(test_str); free(round_str); free(floor_str); free(ceil_str);

	// Test Specific Rounding Cases
	wlf_log(WLF_INFO, "\n--- Testing Specific Rounding Cases ---");

	struct wlf_frect edge_cases[] = {
		wlf_frect_make(0.5, 0.5, 1.5, 1.5),     // exact halves
		wlf_frect_make(-0.5, -0.5, 2.5, 2.5),   // negative halves
		wlf_frect_make(0.1, 0.9, 1.1, 1.9),     // close to integers
		wlf_frect_make(-0.1, -0.9, 2.1, 2.9),   // negative close to integers
	};

	for (int i = 0; i < 4; i++) {
		char *orig = wlf_frect_to_str_prec(&edge_cases[i], 1);

		struct wlf_rect r = wlf_frect_round(&edge_cases[i]);
		struct wlf_rect f = wlf_frect_floor(&edge_cases[i]);
		struct wlf_rect c = wlf_frect_ceil(&edge_cases[i]);

		char *r_str = wlf_rect_to_str(&r);
		char *f_str = wlf_rect_to_str(&f);
		char *c_str = wlf_rect_to_str(&c);

		wlf_log(WLF_INFO, "%s -> Round: %s, Floor: %s, Ceil: %s", orig, r_str, f_str, c_str);

		free(orig); free(r_str); free(f_str); free(c_str);
	}

	// Test Very Small Numbers
	wlf_log(WLF_INFO, "\n--- Testing Very Small Numbers ---");

	struct wlf_frect tiny = wlf_frect_make(0.000001, 0.000002, 0.000003, 0.000004);
	char *tiny_str = wlf_frect_to_str_prec(&tiny, 8);
	wlf_log(WLF_INFO, "Tiny rectangle: %s", tiny_str);
	free(tiny_str);

	struct wlf_rect tiny_rounded = wlf_frect_round(&tiny);
	char *tiny_round_str = wlf_rect_to_str(&tiny_rounded);
	wlf_log(WLF_INFO, "Tiny rounded: %s", tiny_round_str);
	free(tiny_round_str);

	// Test Very Large Numbers
	wlf_log(WLF_INFO, "\n--- Testing Very Large Numbers ---");

	struct wlf_frect large = wlf_frect_make(1000000.5, 2000000.7, 500000.3, 300000.8);
	char *large_str = wlf_frect_to_str_prec(&large, 1);
	wlf_log(WLF_INFO, "Large rectangle: %s", large_str);
	free(large_str);

	struct wlf_rect large_rounded = wlf_frect_round(&large);
	char *large_round_str = wlf_rect_to_str(&large_rounded);
	wlf_log(WLF_INFO, "Large rounded: %s", large_round_str);
	free(large_round_str);

	// Test Negative Coordinates
	wlf_log(WLF_INFO, "\n--- Testing Negative Coordinates ---");

	struct wlf_frect negative = wlf_frect_make(-10.7, -20.3, 30.9, 40.1);
	char *neg_str = wlf_frect_to_str_prec(&negative, 1);
	wlf_log(WLF_INFO, "Negative coordinates: %s", neg_str);
	free(neg_str);

	struct wlf_rect neg_round = wlf_frect_round(&negative);
	struct wlf_rect neg_floor = wlf_frect_floor(&negative);
	struct wlf_rect neg_ceil = wlf_frect_ceil(&negative);

	char *neg_round_str = wlf_rect_to_str(&neg_round);
	char *neg_floor_str = wlf_rect_to_str(&neg_floor);
	char *neg_ceil_str = wlf_rect_to_str(&neg_ceil);

	wlf_log(WLF_INFO, "Negative rounded: %s", neg_round_str);
	wlf_log(WLF_INFO, "Negative floored: %s", neg_floor_str);
	wlf_log(WLF_INFO, "Negative ceiled: %s", neg_ceil_str);

	free(neg_round_str); free(neg_floor_str); free(neg_ceil_str);

	// Test Round-trip Conversion
	wlf_log(WLF_INFO, "\n--- Testing Round-trip Conversion ---");

	struct wlf_rect original_int = wlf_rect_make(42, 84, 100, 200);
	struct wlf_frect converted_float = wlf_rect_to_frect(&original_int);
	struct wlf_rect back_to_int = wlf_frect_to_rect(&converted_float);

	char *orig_str = wlf_rect_to_str(&original_int);
	char *conv_str = wlf_frect_to_str_prec(&converted_float, 1);
	char *back_str = wlf_rect_to_str(&back_to_int);

	wlf_log(WLF_INFO, "Original int: %s", orig_str);
	wlf_log(WLF_INFO, "To float: %s", conv_str);
	wlf_log(WLF_INFO, "Back to int: %s", back_str);
	wlf_log(WLF_INFO, "Round-trip equal: %s", wlf_rect_equal(&original_int, &back_to_int) ? "true" : "false");

	free(orig_str); free(conv_str); free(back_str);

	// Test Mathematical Constants
	wlf_log(WLF_INFO, "\n--- Testing Mathematical Constants ---");

	struct wlf_frect pi_rect = wlf_frect_make(M_PI, M_E, M_PI/2, M_E/2);
	char *pi_str = wlf_frect_to_str_prec(&pi_rect, 6);
	wlf_log(WLF_INFO, "Pi/e rectangle: %s", pi_str);
	free(pi_str);

	struct wlf_rect pi_rounded = wlf_frect_round(&pi_rect);
	char *pi_round_str = wlf_rect_to_str(&pi_rounded);
	wlf_log(WLF_INFO, "Pi/e rounded: %s", pi_round_str);
	free(pi_round_str);

	// Test Epsilon Comparison Edge Cases
	wlf_log(WLF_INFO, "\n--- Testing Epsilon Comparison Edge Cases ---");

	struct wlf_frect base = wlf_frect_make(1.0, 2.0, 3.0, 4.0);
	struct wlf_frect tiny_diff = wlf_frect_make(1.0000001, 2.0000001, 3.0000001, 4.0000001);
	struct wlf_frect big_diff = wlf_frect_make(1.1, 2.1, 3.1, 4.1);

	double epsilons[] = {1e-10, 1e-6, 1e-3, 0.01, 0.1, 1.0};

	for (int i = 0; i < 6; i++) {
		double eps = epsilons[i];
		bool tiny_equal = wlf_frect_nearly_equal(&base, &tiny_diff, eps);
		bool big_equal = wlf_frect_nearly_equal(&base, &big_diff, eps);
		wlf_log(WLF_INFO, "ε=%.0e: tiny_diff=%s, big_diff=%s", eps,
				tiny_equal ? "true" : "false", big_equal ? "true" : "false");
	}

	// Test Zero and Unit Rectangle Properties
	wlf_log(WLF_INFO, "\n--- Testing Zero and Unit Rectangle Properties ---");

	struct wlf_rect zero_converted = wlf_frect_to_rect(&WLF_FRECT_ZERO);
	struct wlf_rect unit_converted = wlf_frect_to_rect(&WLF_FRECT_UNIT);

	char *zero_conv_str = wlf_rect_to_str(&zero_converted);
	char *unit_conv_str = wlf_rect_to_str(&unit_converted);

	wlf_log(WLF_INFO, "Zero frect to rect: %s", zero_conv_str);
	wlf_log(WLF_INFO, "Unit frect to rect: %s", unit_conv_str);
	wlf_log(WLF_INFO, "Zero conversion matches WLF_RECT_ZERO: %s",
			wlf_rect_equal(&zero_converted, &WLF_RECT_ZERO) ? "true" : "false");
	wlf_log(WLF_INFO, "Unit conversion matches WLF_RECT_UNIT: %s",
			wlf_rect_equal(&unit_converted, &WLF_RECT_UNIT) ? "true" : "false");

	free(zero_conv_str); free(unit_conv_str);

	// Test Validity Checks
	wlf_log(WLF_INFO, "\n--- Testing Validity Checks ---");

	struct wlf_frect valid_rect = wlf_frect_make(10.0, 20.0, 30.5, 40.8);
	struct wlf_frect invalid_width = wlf_frect_make(10.0, 20.0, -30.5, 40.8);
	struct wlf_frect invalid_height = wlf_frect_make(10.0, 20.0, 30.5, -40.8);
	struct wlf_frect zero_width = wlf_frect_make(10.0, 20.0, 0.0, 40.8);
	struct wlf_frect zero_height = wlf_frect_make(10.0, 20.0, 30.5, 0.0);

	wlf_log(WLF_INFO, "Valid rect (10.0,20.0,30.5,40.8) is valid: %s",
			wlf_frect_is_valid(&valid_rect) ? "true" : "false");
	wlf_log(WLF_INFO, "Invalid width rect is valid: %s",
			wlf_frect_is_valid(&invalid_width) ? "true" : "false");
	wlf_log(WLF_INFO, "Invalid height rect is valid: %s",
			wlf_frect_is_valid(&invalid_height) ? "true" : "false");
	wlf_log(WLF_INFO, "Zero width rect is valid: %s",
			wlf_frect_is_valid(&zero_width) ? "true" : "false");
	wlf_log(WLF_INFO, "Zero height rect is valid: %s",
			wlf_frect_is_valid(&zero_height) ? "true" : "false");
	wlf_log(WLF_INFO, "NULL pointer is valid: %s",
			wlf_frect_is_valid(NULL) ? "true" : "false");

	// Test String Parsing
	wlf_log(WLF_INFO, "\n--- Testing String Parsing ---");

	struct wlf_frect parsed_rect;

	// Test basic format with parentheses (should succeed)
	if (wlf_frect_from_str("(10.5,20.3,100.7,80.2)", &parsed_rect)) {
		char *parsed_str = wlf_frect_to_str_prec(&parsed_rect, 3);
		wlf_log(WLF_INFO, "Parsed '(10.5,20.3,100.7,80.2)': %s", parsed_str);
		wlf_log(WLF_INFO, "Is valid: %s", wlf_frect_is_valid(&parsed_rect) ? "true" : "false");
		free(parsed_str);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse '(10.5,20.3,100.7,80.2)'");
	}

	// Test format with spaces and parentheses (should succeed)
	if (wlf_frect_from_str("(5.1, 15.8, 30.2, 25.9)", &parsed_rect)) {
		char *parsed_str = wlf_frect_to_str_prec(&parsed_rect, 3);
		wlf_log(WLF_INFO, "Parsed '(5.1, 15.8, 30.2, 25.9)': %s", parsed_str);
		free(parsed_str);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse '(5.1, 15.8, 30.2, 25.9)'");
	}

	// Test scientific notation (should succeed)
	if (wlf_frect_from_str("(1.5e2, 2.3e1, 1.0e3, 8.7e1)", &parsed_rect)) {
		char *parsed_str = wlf_frect_to_str_prec(&parsed_rect, 3);
		wlf_log(WLF_INFO, "Parsed scientific notation: %s", parsed_str);
		free(parsed_str);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse scientific notation");
	}

	// Test negative values with parentheses (should succeed)
	if (wlf_frect_from_str("(-10.5,-20.8,100.3,80.7)", &parsed_rect)) {
		char *parsed_str = wlf_frect_to_str_prec(&parsed_rect, 3);
		wlf_log(WLF_INFO, "Parsed '(-10.5,-20.8,100.3,80.7)': %s", parsed_str);
		wlf_log(WLF_INFO, "Is valid: %s", wlf_frect_is_valid(&parsed_rect) ? "true" : "false");
		free(parsed_str);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse '(-10.5,-20.8,100.3,80.7)'");
	}

	// Test integer values (should succeed)
	if (wlf_frect_from_str("(10,20,30,40)", &parsed_rect)) {
		char *parsed_str = wlf_frect_to_str_prec(&parsed_rect, 1);
		wlf_log(WLF_INFO, "Parsed integer format: %s", parsed_str);
		free(parsed_str);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse integer format");
	}

	// Test invalid formats (should all fail)
	wlf_log(WLF_INFO, "Testing invalid formats (should all fail):");
	wlf_log(WLF_INFO, "Empty string: %s", wlf_frect_from_str("", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "NULL string: %s", wlf_frect_from_str(NULL, &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "NULL rect: %s", wlf_frect_from_str("(1.0,2.0,3.0,4.0)", NULL) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "No parentheses: %s", wlf_frect_from_str("10.5,20.3,100.7,80.2", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "Only opening paren: %s", wlf_frect_from_str("(10.5,20.3,100.7,80.2", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "Only closing paren: %s", wlf_frect_from_str("10.5,20.3,100.7,80.2)", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "Invalid format: %s", wlf_frect_from_str("(abc,def,ghi,jkl)", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "Too few values: %s", wlf_frect_from_str("(10.5,20.3,30.1)", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "Extra text after: %s", wlf_frect_from_str("(10.5,20.3,30.1,40.2)extra", &parsed_rect) ? "parsed" : "failed");

	wlf_log(WLF_INFO, "\n=== Floating-Point Rectangle Test Suite Complete ===");

	return 0;
}
