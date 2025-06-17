#include "wlf/math/wlf_point.h"
#include "wlf/utils/wlf_log.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);

	wlf_log(WLF_INFO, "=== WLF Point Test Suite ===");

	// Test point creation
	wlf_log(WLF_INFO, "\n--- Testing Point Creation ---");
	struct wlf_point p1 = (struct wlf_point){.x = 3, .y = 4};
	struct wlf_point p2 = (struct wlf_point){.x = -2, .y = 7};
	struct wlf_point zero_point = (struct wlf_point){.x = 0, .y = 0};

	char *p1_str = wlf_point_to_str(&p1);
	char *p2_str = wlf_point_to_str(&p2);
	char *zero_str = wlf_point_to_str(&zero_point);

	wlf_log(WLF_INFO, "p1: %s", p1_str);
	wlf_log(WLF_INFO, "p2: %s", p2_str);
	wlf_log(WLF_INFO, "zero: %s", zero_str);

	free(p1_str);
	free(p2_str);
	free(zero_str);

	// Test constants
	wlf_log(WLF_INFO, "\n--- Testing Constants ---");
	char *zero_const_str = wlf_point_to_str(&WLF_POINT_ZERO);
	char *unit_str = wlf_point_to_str(&WLF_POINT_UNIT);
	char *unit_x_str = wlf_point_to_str(&WLF_POINT_UNIT_X);
	char *unit_y_str = wlf_point_to_str(&WLF_POINT_UNIT_Y);

	wlf_log(WLF_INFO, "WLF_POINT_ZERO: %s", zero_const_str);
	wlf_log(WLF_INFO, "WLF_POINT_UNIT: %s", unit_str);
	wlf_log(WLF_INFO, "WLF_POINT_UNIT_X: %s", unit_x_str);
	wlf_log(WLF_INFO, "WLF_POINT_UNIT_Y: %s", unit_y_str);

	free(zero_const_str);
	free(unit_str);
	free(unit_x_str);
	free(unit_y_str);

	// Test equality
	wlf_log(WLF_INFO, "\n--- Testing Equality ---");
	struct wlf_point p1_copy = (struct wlf_point){.x = 3, .y = 4};
	bool equal = wlf_point_equal(&p1, &p1_copy);
	bool not_equal = wlf_point_equal(&p1, &p2);
	wlf_log(WLF_INFO, "p1 == p1_copy: %s", equal ? "true" : "false");
	wlf_log(WLF_INFO, "p1 == p2: %s", not_equal ? "true" : "false");

	// Test zero check
	wlf_log(WLF_INFO, "\n--- Testing Zero Check ---");
	bool is_zero = wlf_point_is_zero(&zero_point);
	bool is_not_zero = wlf_point_is_zero(&p1);
	wlf_log(WLF_INFO, "zero_point is zero: %s", is_zero ? "true" : "false");
	wlf_log(WLF_INFO, "p1 is zero: %s", is_not_zero ? "true" : "false");

	// Test arithmetic operations
	wlf_log(WLF_INFO, "\n--- Testing Arithmetic Operations ---");
	struct wlf_point sum = wlf_point_add(&p1, &p2);
	struct wlf_point diff = wlf_point_subtract(&p1, &p2);
	struct wlf_point scaled = wlf_point_multiply(&p1, 3);

	char *sum_str = wlf_point_to_str(&sum);
	char *diff_str = wlf_point_to_str(&diff);
	char *scaled_str = wlf_point_to_str(&scaled);

	wlf_log(WLF_INFO, "p1 + p2 = %s", sum_str);
	wlf_log(WLF_INFO, "p1 - p2 = %s", diff_str);
	wlf_log(WLF_INFO, "p1 * 3 = %s", scaled_str);

	free(sum_str);
	free(diff_str);
	free(scaled_str);

	// Test distance calculations
	wlf_log(WLF_INFO, "\n--- Testing Distance Calculations ---");
	int manhattan_dist = wlf_point_manhattan_distance(&p1, &p2);
	double euclidean_dist = wlf_point_euclidean_distance(&p1, &p2);

	wlf_log(WLF_INFO, "Manhattan distance between p1 and p2: %d", manhattan_dist);
	wlf_log(WLF_INFO, "Euclidean distance between p1 and p2: %.3f", euclidean_dist);

	// Test with specific known values
	wlf_log(WLF_INFO, "\n--- Testing Known Distance Values ---");
	struct wlf_point origin = (struct wlf_point){.x = 0, .y = 0};
	struct wlf_point point_3_4 = (struct wlf_point){.x = 3, .y = 4};

	int manhattan_3_4 = wlf_point_manhattan_distance(&origin, &point_3_4);
	double euclidean_3_4 = wlf_point_euclidean_distance(&origin, &point_3_4);

	char *origin_str = wlf_point_to_str(&origin);
	char *point_3_4_str = wlf_point_to_str(&point_3_4);

	wlf_log(WLF_INFO, "From %s to %s:", origin_str, point_3_4_str);
	wlf_log(WLF_INFO, "  Manhattan distance: %d (expected: 7)", manhattan_3_4);
	wlf_log(WLF_INFO, "  Euclidean distance: %.3f (expected: 5.000)", euclidean_3_4);

	free(origin_str);
	free(point_3_4_str);

	// Test edge cases
	wlf_log(WLF_INFO, "\n--- Testing Edge Cases ---");

	// Distance from point to itself
	int self_manhattan = wlf_point_manhattan_distance(&p1, &p1);
	double self_euclidean = wlf_point_euclidean_distance(&p1, &p1);
	wlf_log(WLF_INFO, "Distance from point to itself: Manhattan=%d, Euclidean=%.3f",
			self_manhattan, self_euclidean);

	// Operations with negative numbers
	struct wlf_point neg1 = (struct wlf_point){.x = -5, .y = -3};
	struct wlf_point neg2 = (struct wlf_point){.x = -2, .y = -7};
	struct wlf_point neg_sum = wlf_point_add(&neg1, &neg2);

	char *neg1_str = wlf_point_to_str(&neg1);
	char *neg2_str = wlf_point_to_str(&neg2);
	char *neg_sum_str = wlf_point_to_str(&neg_sum);

	wlf_log(WLF_INFO, "Negative addition: %s + %s = %s", neg1_str, neg2_str, neg_sum_str);

	free(neg1_str);
	free(neg2_str);
	free(neg_sum_str);

	// Test scalar multiplication with negative numbers
	struct wlf_point neg_scaled = wlf_point_multiply(&p1, -2);
	char *neg_scaled_str = wlf_point_to_str(&neg_scaled);
	wlf_log(WLF_INFO, "p1 * -2 = %s", neg_scaled_str);
	free(neg_scaled_str);

	// Test string parsing
	wlf_log(WLF_INFO, "\n--- Testing String Parsing ---");

	// Test valid string parsing
	struct wlf_point parsed_point;
	bool parse_success;

	// Basic positive numbers
	parse_success = wlf_point_from_str("(10, 20)", &parsed_point);
	char *parsed_str = wlf_point_to_str(&parsed_point);
	wlf_log(WLF_INFO, "Parse \"(10, 20)\": %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", parsed_str);
	free(parsed_str);

	// Negative numbers
	parse_success = wlf_point_from_str("(-5, 15)", &parsed_point);
	parsed_str = wlf_point_to_str(&parsed_point);
	wlf_log(WLF_INFO, "Parse \"(-5, 15)\": %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", parsed_str);
	free(parsed_str);

	// With spaces
	parse_success = wlf_point_from_str("( 100 , -200 )", &parsed_point);
	parsed_str = wlf_point_to_str(&parsed_point);
	wlf_log(WLF_INFO, "Parse \"( 100 , -200 )\": %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", parsed_str);
	free(parsed_str);

	// Zero values
	parse_success = wlf_point_from_str("(0, 0)", &parsed_point);
	parsed_str = wlf_point_to_str(&parsed_point);
	wlf_log(WLF_INFO, "Parse \"(0, 0)\": %s -> %s",
			parse_success ? "SUCCESS" : "FAILED", parsed_str);
	free(parsed_str);

	// Test invalid string parsing
	wlf_log(WLF_INFO, "\n--- Testing Invalid String Parsing ---");

	// Missing opening bracket
	parse_success = wlf_point_from_str("10, 20)", &parsed_point);
	wlf_log(WLF_INFO, "Parse \"10, 20)\": %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// Missing closing bracket
	parse_success = wlf_point_from_str("(10, 20", &parsed_point);
	wlf_log(WLF_INFO, "Parse \"(10, 20\": %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// Missing comma
	parse_success = wlf_point_from_str("(10 20)", &parsed_point);
	wlf_log(WLF_INFO, "Parse \"(10 20)\": %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// Invalid characters
	parse_success = wlf_point_from_str("(abc, def)", &parsed_point);
	wlf_log(WLF_INFO, "Parse \"(abc, def)\": %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// NULL string
	parse_success = wlf_point_from_str(NULL, &parsed_point);
	wlf_log(WLF_INFO, "Parse NULL string: %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// NULL point
	parse_success = wlf_point_from_str("(10, 20)", NULL);
	wlf_log(WLF_INFO, "Parse to NULL point: %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// Empty string
	parse_success = wlf_point_from_str("", &parsed_point);
	wlf_log(WLF_INFO, "Parse empty string: %s (expected: FAILED)",
			parse_success ? "SUCCESS" : "FAILED");

	// Test round-trip conversion
	wlf_log(WLF_INFO, "\n--- Testing Round-trip Conversion ---");
	struct wlf_point original = (struct wlf_point){.x = 42, .y = -73};
	char *original_str = wlf_point_to_str(&original);
	struct wlf_point round_trip;
	bool round_trip_success = wlf_point_from_str(original_str, &round_trip);
	bool points_equal = wlf_point_equal(&original, &round_trip);

	char *round_trip_str = wlf_point_to_str(&round_trip);
	wlf_log(WLF_INFO, "Original: %s", original_str);
	wlf_log(WLF_INFO, "Round-trip: %s", round_trip_str);
	wlf_log(WLF_INFO, "Parse success: %s", round_trip_success ? "true" : "false");
	wlf_log(WLF_INFO, "Points equal: %s", points_equal ? "true" : "false");

	free(original_str);
	free(round_trip_str);

	wlf_log(WLF_INFO, "\n=== Point Test Suite Complete ===");

	return 0;
}
