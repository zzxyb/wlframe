#include "wlf/math/wlf_fpoint.h"
#include "wlf/math/wlf_point.h"
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

	wlf_log(WLF_INFO, "=== WLF Floating Point Test Suite ===");

	// Test point creation
	wlf_log(WLF_INFO, "\n--- Testing Floating Point Creation ---");
	struct wlf_fpoint fp1 = (struct wlf_fpoint){.x = 3.5, .y = 4.7};
	struct wlf_fpoint fp2 = (struct wlf_fpoint){.x = -2.3, .y = 7.1};
	struct wlf_fpoint zero_point = (struct wlf_fpoint){.x = 0.0, .y = 0.0};

	char *fp1_str = wlf_fpoint_to_str(&fp1);
	char *fp2_str = wlf_fpoint_to_str(&fp2);
	char *zero_str = wlf_fpoint_to_str(&zero_point);

	wlf_log(WLF_INFO, "fp1: %s", fp1_str);
	wlf_log(WLF_INFO, "fp2: %s", fp2_str);
	wlf_log(WLF_INFO, "zero: %s", zero_str);

	free(fp1_str);
	free(fp2_str);
	free(zero_str);

	// Test precision formatting
	wlf_log(WLF_INFO, "\n--- Testing Precision Formatting ---");
	char *prec1_str = wlf_fpoint_to_str_prec(&fp1, 1);
	char *prec5_str = wlf_fpoint_to_str_prec(&fp1, 5);

	wlf_log(WLF_INFO, "fp1 with 1 decimal: %s", prec1_str);
	wlf_log(WLF_INFO, "fp1 with 5 decimals: %s", prec5_str);

	free(prec1_str);
	free(prec5_str);

	// Test constants
	wlf_log(WLF_INFO, "\n--- Testing Constants ---");
	char *zero_const_str = wlf_fpoint_to_str(&WLF_FPOINT_ZERO);
	char *unit_str = wlf_fpoint_to_str(&WLF_FPOINT_UNIT);
	char *unit_x_str = wlf_fpoint_to_str(&WLF_FPOINT_UNIT_X);
	char *unit_y_str = wlf_fpoint_to_str(&WLF_FPOINT_UNIT_Y);

	wlf_log(WLF_INFO, "WLF_FPOINT_ZERO: %s", zero_const_str);
	wlf_log(WLF_INFO, "WLF_FPOINT_UNIT: %s", unit_str);
	wlf_log(WLF_INFO, "WLF_FPOINT_UNIT_X: %s", unit_x_str);
	wlf_log(WLF_INFO, "WLF_FPOINT_UNIT_Y: %s", unit_y_str);

	free(zero_const_str);
	free(unit_str);
	free(unit_x_str);
	free(unit_y_str);

	// Test equality
	wlf_log(WLF_INFO, "\n--- Testing Equality ---");
	struct wlf_fpoint fp1_copy = (struct wlf_fpoint){.x = 3.5, .y = 4.7};
	bool exact_equal = wlf_fpoint_equal(&fp1, &fp1_copy);
	bool not_equal = wlf_fpoint_equal(&fp1, &fp2);

	// Test nearly equal
	struct wlf_fpoint fp1_nearly = (struct wlf_fpoint){.x = 3.500001, .y = 4.700001};
	bool nearly_equal = wlf_fpoint_nearly_equal(&fp1, &fp1_nearly, 0.001);
	bool not_nearly_equal = wlf_fpoint_nearly_equal(&fp1, &fp1_nearly, 0.0000001);

	wlf_log(WLF_INFO, "fp1 == fp1_copy: %s", exact_equal ? "true" : "false");
	wlf_log(WLF_INFO, "fp1 == fp2: %s", not_equal ? "true" : "false");
	wlf_log(WLF_INFO, "fp1 nearly equals fp1_nearly (epsilon=0.001): %s", nearly_equal ? "true" : "false");
	wlf_log(WLF_INFO, "fp1 nearly equals fp1_nearly (epsilon=0.0000001): %s", not_nearly_equal ? "true" : "false");

	// Test zero check
	wlf_log(WLF_INFO, "\n--- Testing Zero Check ---");
	bool is_zero = wlf_fpoint_is_zero(&zero_point);
	bool is_not_zero = wlf_fpoint_is_zero(&fp1);
	wlf_log(WLF_INFO, "zero_point is zero: %s", is_zero ? "true" : "false");
	wlf_log(WLF_INFO, "fp1 is zero: %s", is_not_zero ? "true" : "false");

	// Test arithmetic operations
	wlf_log(WLF_INFO, "\n--- Testing Arithmetic Operations ---");
	struct wlf_fpoint sum = wlf_fpoint_add(&fp1, &fp2);
	struct wlf_fpoint diff = wlf_fpoint_subtract(&fp1, &fp2);
	struct wlf_fpoint scaled = wlf_fpoint_multiply(&fp1, 2.5);
	struct wlf_fpoint divided = wlf_fpoint_divide(&fp1, 2.0);
	struct wlf_fpoint negated = wlf_fpoint_negate(&fp1);

	char *sum_str = wlf_fpoint_to_str(&sum);
	char *diff_str = wlf_fpoint_to_str(&diff);
	char *scaled_str = wlf_fpoint_to_str(&scaled);
	char *divided_str = wlf_fpoint_to_str(&divided);
	char *negated_str = wlf_fpoint_to_str(&negated);

	wlf_log(WLF_INFO, "fp1 + fp2 = %s", sum_str);
	wlf_log(WLF_INFO, "fp1 - fp2 = %s", diff_str);
	wlf_log(WLF_INFO, "fp1 * 2.5 = %s", scaled_str);
	wlf_log(WLF_INFO, "fp1 / 2.0 = %s", divided_str);
	wlf_log(WLF_INFO, "-fp1 = %s", negated_str);

	free(sum_str);
	free(diff_str);
	free(scaled_str);
	free(divided_str);
	free(negated_str);

	// Test distance calculations
	wlf_log(WLF_INFO, "\n--- Testing Distance Calculations ---");
	double manhattan_dist = wlf_fpoint_manhattan_distance(&fp1, &fp2);
	double euclidean_dist = wlf_fpoint_euclidean_distance(&fp1, &fp2);

	wlf_log(WLF_INFO, "Manhattan distance between fp1 and fp2: %.3f", manhattan_dist);
	wlf_log(WLF_INFO, "Euclidean distance between fp1 and fp2: %.3f", euclidean_dist);

	// Test dot product and angle
	wlf_log(WLF_INFO, "\n--- Testing Dot Product and Angles ---");
	double dot_product = wlf_fpoint_dot_product(&fp1, &fp2);
	double fp1_angle = wlf_fpoint_angle(&fp1);
	double fp2_angle = wlf_fpoint_angle(&fp2);
	double angle_between = wlf_fpoint_angle_between(&fp1, &fp2);

	wlf_log(WLF_INFO, "Dot product fp1 · fp2: %.3f", dot_product);
	wlf_log(WLF_INFO, "Angle of fp1: %.3f radians (%.1f degrees)", fp1_angle, fp1_angle * 180.0 / M_PI);
	wlf_log(WLF_INFO, "Angle of fp2: %.3f radians (%.1f degrees)", fp2_angle, fp2_angle * 180.0 / M_PI);
	wlf_log(WLF_INFO, "Angle between fp1 and fp2: %.3f radians (%.1f degrees)",
			angle_between, angle_between * 180.0 / M_PI);

	// Test rotation
	wlf_log(WLF_INFO, "\n--- Testing Rotation ---");
	double rotation_angle = M_PI / 4; // 45 degrees
	struct wlf_fpoint rotated = wlf_fpoint_rotate(&fp1, rotation_angle);
	char *rotated_str = wlf_fpoint_to_str(&rotated);

	wlf_log(WLF_INFO, "fp1 rotated by 45 degrees: %s", rotated_str);
	free(rotated_str);

	// Test length calculations
	wlf_log(WLF_INFO, "\n--- Testing Length Calculations ---");
	double fp1_length = wlf_fpoint_length(&fp1);
	double fp1_length_squared = wlf_fpoint_length_squared(&fp1);

	wlf_log(WLF_INFO, "Length of fp1: %.3f", fp1_length);
	wlf_log(WLF_INFO, "Length squared of fp1: %.3f", fp1_length_squared);

	// Test normalization
	wlf_log(WLF_INFO, "\n--- Testing Normalization ---");
	struct wlf_fpoint normalized = wlf_fpoint_normalize(&fp1);
	double normalized_length = wlf_fpoint_length(&normalized);
	char *normalized_str = wlf_fpoint_to_str(&normalized);

	wlf_log(WLF_INFO, "Normalized fp1: %s (length: %.6f)", normalized_str, normalized_length);
	free(normalized_str);

	// Test circle containment
	wlf_log(WLF_INFO, "\n--- Testing Circle Containment ---");
	struct wlf_fpoint circle_center = (struct wlf_fpoint){.x = 0.0, .y = 0.0};
	double radius = 5.0;
	struct wlf_fpoint inside_point = (struct wlf_fpoint){.x = 3.0, .y = 3.0};
	struct wlf_fpoint outside_point = (struct wlf_fpoint){.x = 10.0, .y = 10.0};

	bool inside = wlf_fpoint_in_circle(&inside_point, &circle_center, radius);
	bool outside = wlf_fpoint_in_circle(&outside_point, &circle_center, radius);

	char *inside_str = wlf_fpoint_to_str(&inside_point);
	char *outside_str = wlf_fpoint_to_str(&outside_point);
	char *center_str = wlf_fpoint_to_str(&circle_center);

	wlf_log(WLF_INFO, "Point %s in circle (center=%s, radius=%.1f): %s",
			inside_str, center_str, radius, inside ? "true" : "false");
	wlf_log(WLF_INFO, "Point %s in circle (center=%s, radius=%.1f): %s",
			outside_str, center_str, radius, outside ? "true" : "false");

	free(inside_str);
	free(outside_str);
	free(center_str);

	// Test rounding operations
	wlf_log(WLF_INFO, "\n--- Testing Rounding Operations ---");
	struct wlf_fpoint test_round = (struct wlf_fpoint){.x = 3.7, .y = -2.3};
	struct wlf_point rounded = wlf_fpoint_round(&test_round);
	struct wlf_point floored = wlf_fpoint_floor(&test_round);
	struct wlf_point ceiled = wlf_fpoint_ceil(&test_round);

	char *test_round_str = wlf_fpoint_to_str(&test_round);
	char *rounded_str = wlf_point_to_str(&rounded);
	char *floored_str = wlf_point_to_str(&floored);
	char *ceiled_str = wlf_point_to_str(&ceiled);

	wlf_log(WLF_INFO, "Original: %s", test_round_str);
	wlf_log(WLF_INFO, "Rounded: %s", rounded_str);
	wlf_log(WLF_INFO, "Floored: %s", floored_str);
	wlf_log(WLF_INFO, "Ceiled: %s", ceiled_str);

	free(test_round_str);
	free(rounded_str);
	free(floored_str);
	free(ceiled_str);

	// Test interpolation
	wlf_log(WLF_INFO, "\n--- Testing Interpolation ---");
	struct wlf_fpoint start = (struct wlf_fpoint){.x = 0.0, .y = 0.0};
	struct wlf_fpoint end = (struct wlf_fpoint){.x = 10.0, .y = 10.0};
	struct wlf_fpoint lerp_half = wlf_fpoint_lerp(&start, &end, 0.5);
	struct wlf_fpoint lerp_quarter = wlf_fpoint_lerp(&start, &end, 0.25);

	char *start_str = wlf_fpoint_to_str(&start);
	char *end_str = wlf_fpoint_to_str(&end);
	char *lerp_half_str = wlf_fpoint_to_str(&lerp_half);
	char *lerp_quarter_str = wlf_fpoint_to_str(&lerp_quarter);

	wlf_log(WLF_INFO, "Linear interpolation from %s to %s:", start_str, end_str);
	wlf_log(WLF_INFO, "  At t=0.5: %s", lerp_half_str);
	wlf_log(WLF_INFO, "  At t=0.25: %s", lerp_quarter_str);

	free(start_str);
	free(end_str);
	free(lerp_half_str);
	free(lerp_quarter_str);

	// Test Bezier curve
	wlf_log(WLF_INFO, "\n--- Testing Bezier Curve ---");
	struct wlf_fpoint p0 = (struct wlf_fpoint){.x = 0.0, .y = 0.0};
	struct wlf_fpoint p1 = (struct wlf_fpoint){.x = 5.0, .y = 10.0};
	struct wlf_fpoint p2 = (struct wlf_fpoint){.x = 10.0, .y = 0.0};
	struct wlf_fpoint bezier_half = wlf_fpoint_bezier(&p0, &p1, &p2, 0.5);

	char *p0_str = wlf_fpoint_to_str(&p0);
	char *p1_str = wlf_fpoint_to_str(&p1);
	char *p2_str = wlf_fpoint_to_str(&p2);
	char *bezier_str = wlf_fpoint_to_str(&bezier_half);

	wlf_log(WLF_INFO, "Quadratic Bezier curve:");
	wlf_log(WLF_INFO, "  P0: %s, P1: %s, P2: %s", p0_str, p1_str, p2_str);
	wlf_log(WLF_INFO, "  At t=0.5: %s", bezier_str);

	free(p0_str);
	free(p1_str);
	free(p2_str);
	free(bezier_str);

	// Test conversions
	wlf_log(WLF_INFO, "\n--- Testing Conversions ---");
	struct wlf_point int_point = (struct wlf_point){.x = 5, .y = 7};
	struct wlf_fpoint converted_to_float = wlf_point_to_fpoint(&int_point);
	struct wlf_point converted_back = wlf_fpoint_to_point(&converted_to_float);

	char *int_point_str = wlf_point_to_str(&int_point);
	char *converted_float_str = wlf_fpoint_to_str(&converted_to_float);
	char *converted_back_str = wlf_point_to_str(&converted_back);

	wlf_log(WLF_INFO, "Integer point: %s", int_point_str);
	wlf_log(WLF_INFO, "Converted to float: %s", converted_float_str);
	wlf_log(WLF_INFO, "Converted back to int: %s", converted_back_str);

	free(int_point_str);
	free(converted_float_str);
	free(converted_back_str);

	// Test string parsing
	wlf_log(WLF_INFO, "\n--- Testing String Parsing ---");

	// Test valid string parsing
	struct wlf_fpoint parsed_fpoint;
	bool parse_success;

	// Basic positive floating point numbers
	parse_success = wlf_fpoint_from_str("(10.5, 20.75)", &parsed_fpoint);
	char *parsed_str = wlf_fpoint_to_str(&parsed_fpoint);
	wlf_log(WLF_INFO, "Parse \"(10.5, 20.75)\": %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", parsed_str);
	free(parsed_str);

	// Negative floating point numbers
	parse_success = wlf_fpoint_from_str("(-5.25, 15.0)", &parsed_fpoint);
	parsed_str = wlf_fpoint_to_str(&parsed_fpoint);
	wlf_log(WLF_INFO, "Parse \"(-5.25, 15.0)\": %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", parsed_str);
	free(parsed_str);

	// With spaces
	parse_success = wlf_fpoint_from_str("( 100.125 , -200.875 )", &parsed_fpoint);
	parsed_str = wlf_fpoint_to_str(&parsed_fpoint);
	wlf_log(WLF_INFO, "Parse \"( 100.125 , -200.875 )\": %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", parsed_str);
	free(parsed_str);

	// Zero values
	parse_success = wlf_fpoint_from_str("(0.0, 0.0)", &parsed_fpoint);
	parsed_str = wlf_fpoint_to_str(&parsed_fpoint);
	wlf_log(WLF_INFO, "Parse \"(0.0, 0.0)\": %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", parsed_str);
	free(parsed_str);

	// Scientific notation
	parse_success = wlf_fpoint_from_str("(1.5e2, -3.14e-1)", &parsed_fpoint);
	parsed_str = wlf_fpoint_to_str(&parsed_fpoint);
	wlf_log(WLF_INFO, "Parse \"(1.5e2, -3.14e-1)\": %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", parsed_str);
	free(parsed_str);

	// Integer values (should work for floating point)
	parse_success = wlf_fpoint_from_str("(42, -17)", &parsed_fpoint);
	parsed_str = wlf_fpoint_to_str(&parsed_fpoint);
	wlf_log(WLF_INFO, "Parse \"(42, -17)\": %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", parsed_str);
	free(parsed_str);

	// Test invalid string parsing
	wlf_log(WLF_INFO, "\n--- Testing Invalid Floating Point String Parsing ---");

	// Missing opening bracket
	parse_success = wlf_fpoint_from_str("10.5, 20.75)", &parsed_fpoint);
	wlf_log(WLF_INFO, "Parse \"10.5, 20.75)\": %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// Missing closing bracket
	parse_success = wlf_fpoint_from_str("(10.5, 20.75", &parsed_fpoint);
	wlf_log(WLF_INFO, "Parse \"(10.5, 20.75\": %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// Missing comma
	parse_success = wlf_fpoint_from_str("(10.5 20.75)", &parsed_fpoint);
	wlf_log(WLF_INFO, "Parse \"(10.5 20.75)\": %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// Invalid characters
	parse_success = wlf_fpoint_from_str("(abc.def, xyz)", &parsed_fpoint);
	wlf_log(WLF_INFO, "Parse \"(abc.def, xyz)\": %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// NULL string
	parse_success = wlf_fpoint_from_str(NULL, &parsed_fpoint);
	wlf_log(WLF_INFO, "Parse NULL string: %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// NULL point
	parse_success = wlf_fpoint_from_str("(10.5, 20.75)", NULL);
	wlf_log(WLF_INFO, "Parse to NULL point: %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// Empty string
	parse_success = wlf_fpoint_from_str("", &parsed_fpoint);
	wlf_log(WLF_INFO, "Parse empty string: %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// Test round-trip conversion
	wlf_log(WLF_INFO, "\n--- Testing Floating Point Round-trip Conversion ---");
	struct wlf_fpoint original_fp = (struct wlf_fpoint){.x = 3.14159, .y = -2.71828};
	char *original_fp_str = wlf_fpoint_to_str(&original_fp);
	struct wlf_fpoint round_trip_fp;
	bool round_trip_fp_success = wlf_fpoint_from_str(original_fp_str, &round_trip_fp);
	bool fpoints_equal = wlf_fpoint_nearly_equal(&original_fp, &round_trip_fp, 0.00001);

	char *round_trip_fp_str = wlf_fpoint_to_str(&round_trip_fp);
	wlf_log(WLF_INFO, "Original: %s", original_fp_str);
	wlf_log(WLF_INFO, "Round-trip: %s", round_trip_fp_str);
	wlf_log(WLF_INFO, "Parse success: %s", round_trip_fp_success ? "true" : "false");
	wlf_log(WLF_INFO, "Points nearly equal: %s", fpoints_equal ? "true" : "false");

	free(original_fp_str);
	free(round_trip_fp_str);	// Test high precision parsing
	wlf_log(WLF_INFO, "\n--- Testing High Precision Parsing ---");
	parse_success = wlf_fpoint_from_str("(3.141592653589793, 2.718281828459045)", &parsed_fpoint);
	char *high_prec_str = wlf_fpoint_to_str_prec(&parsed_fpoint, 10);
	wlf_log(WLF_INFO, "High precision parse: %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", high_prec_str);
	free(high_prec_str);

	// Normalization of zero vector
	struct wlf_fpoint zero_normalize = wlf_fpoint_normalize(&zero_point);
	char *zero_normalize_str = wlf_fpoint_to_str(&zero_normalize);
	wlf_log(WLF_INFO, "Normalized zero vector: %s", zero_normalize_str);
	free(zero_normalize_str);

	wlf_log(WLF_INFO, "\n=== Floating Point Test Suite Complete ===");

	return 0;
}
