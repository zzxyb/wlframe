#include "wlf/math/wlf_fsize.h"
#include "wlf/math/wlf_size.h"
#include "wlf/utils/wlf_log.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);

	wlf_log(WLF_INFO, "=== WLF Floating Point Size Test Suite ===");

	// Test size creation
	wlf_log(WLF_INFO, "\n--- Testing Floating Point Size Creation ---");
	struct wlf_fsize fs1 = (struct wlf_fsize) {.width = 800.5, .height = 600.25};
	struct wlf_fsize fs2 = (struct wlf_fsize) {.width = 1920.0, .height = 1080.0};
	struct wlf_fsize zero_size = (struct wlf_fsize) {.width = 0.0, .height = 0.0};
	struct wlf_fsize negative_size = (struct wlf_fsize) {.width = -10.5, .height = 20.75};

	char *fs1_str = wlf_fsize_to_str(&fs1);
	char *fs2_str = wlf_fsize_to_str(&fs2);
	char *zero_str = wlf_fsize_to_str(&zero_size);
	char *negative_str = wlf_fsize_to_str(&negative_size);

	wlf_log(WLF_INFO, "fs1: %s", fs1_str);
	wlf_log(WLF_INFO, "fs2: %s", fs2_str);
	wlf_log(WLF_INFO, "zero: %s", zero_str);
	wlf_log(WLF_INFO, "negative: %s", negative_str);

	free(fs1_str);
	free(fs2_str);
	free(zero_str);
	free(negative_str);

	// Test precision formatting
	wlf_log(WLF_INFO, "\n--- Testing Precision Formatting ---");
	char *prec1_str = wlf_fsize_to_str_prec(&fs1, 1);
	char *prec5_str = wlf_fsize_to_str_prec(&fs1, 5);
	char *prec0_str = wlf_fsize_to_str_prec(&fs1, 0);

	wlf_log(WLF_INFO, "fs1 with 1 decimal: %s", prec1_str);
	wlf_log(WLF_INFO, "fs1 with 5 decimals: %s", prec5_str);
	wlf_log(WLF_INFO, "fs1 with 0 decimals: %s", prec0_str);

	free(prec1_str);
	free(prec5_str);
	free(prec0_str);

	// Test constants
	wlf_log(WLF_INFO, "\n--- Testing Constants ---");
	char *zero_const_str = wlf_fsize_to_str(&WLF_FSIZE_ZERO);
	char *unit_str = wlf_fsize_to_str(&WLF_FSIZE_UNIT);

	wlf_log(WLF_INFO, "WLF_FSIZE_ZERO: %s", zero_const_str);
	wlf_log(WLF_INFO, "WLF_FSIZE_UNIT: %s", unit_str);

	free(zero_const_str);
	free(unit_str);

	// Test equality
	wlf_log(WLF_INFO, "\n--- Testing Equality ---");
	struct wlf_fsize fs1_copy = (struct wlf_fsize) {.width = 800.5, .height = 600.25};
	bool exact_equal = wlf_fsize_equal(&fs1, &fs1_copy);
	bool not_equal = wlf_fsize_equal(&fs1, &fs2);

	// Test nearly equal
	struct wlf_fsize fs1_nearly = (struct wlf_fsize) {.width = 800.500001, .height = 600.250001};
	bool nearly_equal = wlf_fsize_nearly_equal(&fs1, &fs1_nearly, 0.001);
	bool not_nearly_equal = wlf_fsize_nearly_equal(&fs1, &fs1_nearly, 0.0000001);

	wlf_log(WLF_INFO, "fs1 == fs1_copy: %s", exact_equal ? "true" : "false");
	wlf_log(WLF_INFO, "fs1 == fs2: %s", not_equal ? "true" : "false");
	wlf_log(WLF_INFO, "fs1 nearly equals fs1_nearly (epsilon=0.001): %s",
		nearly_equal ? "true" : "false");
	wlf_log(WLF_INFO, "fs1 nearly equals fs1_nearly (epsilon=0.0000001): %s",
		not_nearly_equal ? "true" : "false");

	// Test arithmetic operations
	wlf_log(WLF_INFO, "\n--- Testing Arithmetic Operations ---");
	struct wlf_fsize sum = wlf_fsize_add(&fs1, &fs2);
	struct wlf_fsize diff = wlf_fsize_subtract(&fs2, &fs1);
	struct wlf_fsize scaled = wlf_fsize_multiply(&fs1, 2.5);
	struct wlf_fsize divided = wlf_fsize_divide(&fs1, 2.0);

	char *sum_str = wlf_fsize_to_str(&sum);
	char *diff_str = wlf_fsize_to_str(&diff);
	char *scaled_str = wlf_fsize_to_str(&scaled);
	char *divided_str = wlf_fsize_to_str(&divided);

	wlf_log(WLF_INFO, "fs1 + fs2 = %s", sum_str);
	wlf_log(WLF_INFO, "fs2 - fs1 = %s", diff_str);
	wlf_log(WLF_INFO, "fs1 * 2.5 = %s", scaled_str);
	wlf_log(WLF_INFO, "fs1 / 2.0 = %s", divided_str);

	free(sum_str);
	free(diff_str);
	free(scaled_str);
	free(divided_str);

	// Test area calculation
	wlf_log(WLF_INFO, "\n--- Testing Area Calculation ---");
	double fs1_area = wlf_fsize_area(&fs1);
	double fs2_area = wlf_fsize_area(&fs2);
	double zero_area = wlf_fsize_area(&zero_size);

	wlf_log(WLF_INFO, "Area of fs1: %.3f", fs1_area);
	wlf_log(WLF_INFO, "Area of fs2: %.3f", fs2_area);
	wlf_log(WLF_INFO, "Area of zero size: %.3f", zero_area);

	// Test conversions between integer and floating point sizes
	wlf_log(WLF_INFO, "\n--- Testing Conversions ---");
	struct wlf_size int_size = (struct wlf_size) {.width = 640, .height = 480};
	struct wlf_fsize converted_to_float = wlf_size_to_fsize(&int_size);
	struct wlf_size converted_back = wlf_fsize_to_size(&converted_to_float);

	char *int_size_str = wlf_size_to_str(&int_size);
	char *converted_float_str = wlf_fsize_to_str(&converted_to_float);
	char *converted_back_str = wlf_size_to_str(&converted_back);

	wlf_log(WLF_INFO, "Integer size: %s", int_size_str);
	wlf_log(WLF_INFO, "Converted to float: %s", converted_float_str);
	wlf_log(WLF_INFO, "Converted back to int: %s", converted_back_str);

	free(int_size_str);
	free(converted_float_str);
	free(converted_back_str);

	// Test rounding operations
	wlf_log(WLF_INFO, "\n--- Testing Rounding Operations ---");
	struct wlf_fsize test_round = (struct wlf_fsize) {.width = 123.7, .height = 456.3};
	struct wlf_size rounded = wlf_fsize_round(&test_round);
	struct wlf_size floored = wlf_fsize_floor(&test_round);
	struct wlf_size ceiled = wlf_fsize_ceil(&test_round);

	char *test_round_str = wlf_fsize_to_str(&test_round);
	char *rounded_str = wlf_size_to_str(&rounded);
	char *floored_str = wlf_size_to_str(&floored);
	char *ceiled_str = wlf_size_to_str(&ceiled);

	wlf_log(WLF_INFO, "Original: %s", test_round_str);
	wlf_log(WLF_INFO, "Rounded: %s", rounded_str);
	wlf_log(WLF_INFO, "Floored: %s", floored_str);
	wlf_log(WLF_INFO, "Ceiled: %s", ceiled_str);

	free(test_round_str);
	free(rounded_str);
	free(floored_str);
	free(ceiled_str);

	// Test with fractional values
	wlf_log(WLF_INFO, "\n--- Testing Fractional Values ---");
	struct wlf_fsize fractional = (struct wlf_fsize) {.width = 99.99, .height = 199.01};
	struct wlf_size frac_rounded = wlf_fsize_round(&fractional);
	struct wlf_size frac_floored = wlf_fsize_floor(&fractional);
	struct wlf_size frac_ceiled = wlf_fsize_ceil(&fractional);

	char *fractional_str = wlf_fsize_to_str(&fractional);
	char *frac_rounded_str = wlf_size_to_str(&frac_rounded);
	char *frac_floored_str = wlf_size_to_str(&frac_floored);
	char *frac_ceiled_str = wlf_size_to_str(&frac_ceiled);

	wlf_log(WLF_INFO, "Fractional: %s", fractional_str);
	wlf_log(WLF_INFO, "Rounded: %s", frac_rounded_str);
	wlf_log(WLF_INFO, "Floored: %s", frac_floored_str);
	wlf_log(WLF_INFO, "Ceiled: %s", frac_ceiled_str);

	free(fractional_str);
	free(frac_rounded_str);
	free(frac_floored_str);
	free(frac_ceiled_str);

	// Test aspect ratio calculations
	wlf_log(WLF_INFO, "\n--- Testing Aspect Ratios ---");
	struct wlf_fsize wide_screen = (struct wlf_fsize) {.width = 1920.0, .height = 1080.0};
	struct wlf_fsize ultra_wide = (struct wlf_fsize) {.width = 3440.0, .height = 1440.0};
	struct wlf_fsize square = (struct wlf_fsize) {.width = 1024.0, .height = 1024.0};
	struct wlf_fsize portrait = (struct wlf_fsize) {.width = 1080.0, .height = 1920.0};

	char *wide_str = wlf_fsize_to_str(&wide_screen);
	char *ultra_str = wlf_fsize_to_str(&ultra_wide);
	char *square_str = wlf_fsize_to_str(&square);
	char *portrait_str = wlf_fsize_to_str(&portrait);

	wlf_log(WLF_INFO, "Wide screen: %s (ratio: %.3f)", wide_str, wide_screen.width / wide_screen.height);
	wlf_log(WLF_INFO, "Ultra wide: %s (ratio: %.3f)", ultra_str, ultra_wide.width / ultra_wide.height);
	wlf_log(WLF_INFO, "Square: %s (ratio: %.3f)", square_str, square.width / square.height);
	wlf_log(WLF_INFO, "Portrait: %s (ratio: %.3f)", portrait_str, portrait.width / portrait.height);

	free(wide_str);
	free(ultra_str);
	free(square_str);
	free(portrait_str);

	// Test scaling with different factors
	wlf_log(WLF_INFO, "\n--- Testing Various Scaling Factors ---");
	struct wlf_fsize base = (struct wlf_fsize) {.width = 100.0, .height = 100.0};
	struct wlf_fsize scaled_pi = wlf_fsize_multiply(&base, M_PI);
	struct wlf_fsize scaled_half = wlf_fsize_multiply(&base, 0.5);
	struct wlf_fsize scaled_tiny = wlf_fsize_multiply(&base, 0.001);

	char *base_str = wlf_fsize_to_str(&base);
	char *scaled_pi_str = wlf_fsize_to_str(&scaled_pi);
	char *scaled_half_str = wlf_fsize_to_str(&scaled_half);
	char *scaled_tiny_str = wlf_fsize_to_str(&scaled_tiny);

	wlf_log(WLF_INFO, "Base: %s", base_str);
	wlf_log(WLF_INFO, "Scaled by π: %s", scaled_pi_str);
	wlf_log(WLF_INFO, "Scaled by 0.5: %s", scaled_half_str);
	wlf_log(WLF_INFO, "Scaled by 0.001: %s", scaled_tiny_str);

	free(base_str);
	free(scaled_pi_str);
	free(scaled_half_str);
	free(scaled_tiny_str);

	// Test edge cases
	wlf_log(WLF_INFO, "\n--- Testing Edge Cases ---");

	// Division by zero
	struct wlf_fsize div_test = (struct wlf_fsize) {.width = 100.0, .height = 200.0};
	struct wlf_fsize div_by_zero = wlf_fsize_divide(&div_test, 0.0);
	char *div_by_zero_str = wlf_fsize_to_str(&div_by_zero);
	wlf_log(WLF_INFO, "Division by zero result: %s", div_by_zero_str);
	free(div_by_zero_str);

	// Very small numbers
	struct wlf_fsize very_small = {.width = 0.00001, .height = 0.00002};
	char *very_small_str = wlf_fsize_to_str_prec(&very_small, 6);
	wlf_log(WLF_INFO, "Very small size: %s", very_small_str);
	free(very_small_str);

	// Very large numbers
	struct wlf_fsize very_large = (struct wlf_fsize) {.width = 999999.999, .height = 888888.888};
	char *very_large_str = wlf_fsize_to_str(&very_large);
	double very_large_area = wlf_fsize_area(&very_large);
	wlf_log(WLF_INFO, "Very large size: %s (area: %.0f)", very_large_str, very_large_area);
	free(very_large_str);

	// Negative scaling
	struct wlf_fsize neg_scaled = wlf_fsize_multiply(&base, -1.5);
	char *neg_scaled_str = wlf_fsize_to_str(&neg_scaled);
	wlf_log(WLF_INFO, "Negative scaling: %s", neg_scaled_str);
	free(neg_scaled_str);

	// Test precision edge cases
	wlf_log(WLF_INFO, "\n--- Testing Precision Edge Cases ---");
	struct wlf_fsize precise = (struct wlf_fsize) {.width = 1.23456789, .height = 9.87654321};
	char *precise_str = wlf_fsize_to_str_prec(&precise, 8);
	char *precise_str_invalid = wlf_fsize_to_str_prec(&precise, -1); // Should default to 3
	char *precise_str_large = wlf_fsize_to_str_prec(&precise, 20);   // Should clamp to 15

	wlf_log(WLF_INFO, "High precision (8 decimals): %s", precise_str);
	wlf_log(WLF_INFO, "Invalid precision (-1): %s", precise_str_invalid);
	wlf_log(WLF_INFO, "Large precision (20): %s", precise_str_large);

	free(precise_str);
	free(precise_str_invalid);
	free(precise_str_large);

	// Test calculations with mixed operations
	wlf_log(WLF_INFO, "\n--- Testing Complex Calculations ---");
	struct wlf_fsize calc1 = (struct wlf_fsize) {.width = 10.0, .height = 20.0};
	struct wlf_fsize calc2 = (struct wlf_fsize) {.width = 5.0, .height = 8.0};

	// Complex calculation: ((calc1 + calc2) * 2.0) / 3.0
	struct wlf_fsize step1 = wlf_fsize_add(&calc1, &calc2);
	struct wlf_fsize step2 = wlf_fsize_multiply(&step1, 2.0);
	struct wlf_fsize result = wlf_fsize_divide(&step2, 3.0);

	char *calc1_str = wlf_fsize_to_str(&calc1);
	char *calc2_str = wlf_fsize_to_str(&calc2);
	char *result_str = wlf_fsize_to_str(&result);

	wlf_log(WLF_INFO, "calc1: %s, calc2: %s", calc1_str, calc2_str);
	wlf_log(WLF_INFO, "((calc1 + calc2) * 2.0) / 3.0 = %s", result_str);

	free(calc1_str);
	free(calc2_str);
	free(result_str);

	wlf_log(WLF_INFO, "\n=== Floating Point Size Test Suite Complete ===");

	return 0;
}
