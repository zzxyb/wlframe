#include "wlf/math/wlf_rect.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);

	wlf_log(WLF_INFO, "=== WLF Rectangle Test Suite ===");

	// Test Rectangle Creation
	wlf_log(WLF_INFO, "\n--- Testing Rectangle Creation ---");

	struct wlf_rect r1 = wlf_rect_make(10, 20, 100, 80);
	struct wlf_rect r2 = wlf_rect_make(-5, -10, 50, 40);
	struct wlf_rect zero_rect = WLF_RECT_ZERO;
	struct wlf_rect unit_rect = WLF_RECT_UNIT;

	char *str1 = wlf_rect_to_str(&r1);
	char *str2 = wlf_rect_to_str(&r2);
	char *str_zero = wlf_rect_to_str(&zero_rect);
	char *str_unit = wlf_rect_to_str(&unit_rect);

	wlf_log(WLF_INFO, "r1: %s", str1);
	wlf_log(WLF_INFO, "r2: %s", str2);
	wlf_log(WLF_INFO, "zero: %s", str_zero);
	wlf_log(WLF_INFO, "unit: %s", str_unit);

	free(str1); free(str2); free(str_zero); free(str_unit);

	// Test creation from point and size
	struct wlf_point pos = {15, 25};
	struct wlf_size size = {60, 45};
	struct wlf_rect r3 = wlf_rect_from_point_size(&pos, &size);
	char *str3 = wlf_rect_to_str(&r3);
	wlf_log(WLF_INFO, "From point(15,25) and size(60,45): %s", str3);
	free(str3);

	// Test creation from two points
	struct wlf_point p1 = {10, 10};
	struct wlf_point p2 = {50, 30};
	struct wlf_rect r4 = wlf_rect_from_points(&p1, &p2);
	char *str4 = wlf_rect_to_str(&r4);
	wlf_log(WLF_INFO, "From points(10,10) to (50,30): %s", str4);
	free(str4);

	// Test Constants
	wlf_log(WLF_INFO, "\n--- Testing Constants ---");

	char *zero_str = wlf_rect_to_str(&WLF_RECT_ZERO);
	char *unit_str = wlf_rect_to_str(&WLF_RECT_UNIT);
	wlf_log(WLF_INFO, "WLF_RECT_ZERO: %s", zero_str);
	wlf_log(WLF_INFO, "WLF_RECT_UNIT: %s", unit_str);
	free(zero_str); free(unit_str);

	// Test Equality
	wlf_log(WLF_INFO, "\n--- Testing Equality ---");

	struct wlf_rect r1_copy = wlf_rect_make(10, 20, 100, 80);
	wlf_log(WLF_INFO, "r1 == r1_copy: %s", wlf_rect_equal(&r1, &r1_copy) ? "true" : "false");
	wlf_log(WLF_INFO, "r1 == r2: %s", wlf_rect_equal(&r1, &r2) ? "true" : "false");

	// Test Validity and Empty Checks
	wlf_log(WLF_INFO, "\n--- Testing Validity and Empty Checks ---");

	struct wlf_rect empty_rect = wlf_rect_make(10, 10, 0, 20);
	struct wlf_rect invalid_rect = wlf_rect_make(10, 10, -5, 20);

	wlf_log(WLF_INFO, "r1 is empty: %s", wlf_rect_is_empty(&r1) ? "true" : "false");
	wlf_log(WLF_INFO, "empty_rect is empty: %s", wlf_rect_is_empty(&empty_rect) ? "true" : "false");
	wlf_log(WLF_INFO, "r1 is valid: %s", wlf_rect_is_valid(&r1) ? "true" : "false");
	wlf_log(WLF_INFO, "invalid_rect is valid: %s", wlf_rect_is_valid(&invalid_rect) ? "true" : "false");

	// Test Getters
	wlf_log(WLF_INFO, "\n--- Testing Getters ---");

	struct wlf_point position = wlf_rect_get_position(&r1);
	struct wlf_size rect_size = wlf_rect_get_size(&r1);
	struct wlf_point center = wlf_rect_get_center(&r1);
	struct wlf_point top_left = wlf_rect_get_top_left(&r1);
	struct wlf_point bottom_right = wlf_rect_get_bottom_right(&r1);

	char *pos_str = wlf_point_to_str(&position);
	char *size_str = wlf_size_to_str(&rect_size);
	char *center_str = wlf_point_to_str(&center);
	char *tl_str = wlf_point_to_str(&top_left);
	char *br_str = wlf_point_to_str(&bottom_right);

	wlf_log(WLF_INFO, "r1 position: %s", pos_str);
	wlf_log(WLF_INFO, "r1 size: %s", size_str);
	wlf_log(WLF_INFO, "r1 center: %s", center_str);
	wlf_log(WLF_INFO, "r1 top-left: %s", tl_str);
	wlf_log(WLF_INFO, "r1 bottom-right: %s", br_str);

	free(pos_str); free(size_str); free(center_str); free(tl_str); free(br_str);

	// Test Area and Perimeter
	wlf_log(WLF_INFO, "\n--- Testing Area and Perimeter ---");

	int area = wlf_rect_area(&r1);
	int perimeter = wlf_rect_perimeter(&r1);
	wlf_log(WLF_INFO, "r1 area: %d (expected: 8000)", area);
	wlf_log(WLF_INFO, "r1 perimeter: %d (expected: 360)", perimeter);

	// Test Transformations
	wlf_log(WLF_INFO, "\n--- Testing Transformations ---");

	struct wlf_point offset = {5, -3};
	struct wlf_rect offset_rect = wlf_rect_offset(&r1, &offset);
	char *offset_str = wlf_rect_to_str(&offset_rect);
	wlf_log(WLF_INFO, "r1 offset by (5,-3): %s", offset_str);
	free(offset_str);

	struct wlf_rect inflated = wlf_rect_inflate(&r1, 10, 5);
	char *inflated_str = wlf_rect_to_str(&inflated);
	wlf_log(WLF_INFO, "r1 inflated by (10,5): %s", inflated_str);
	free(inflated_str);

	struct wlf_rect scaled = wlf_rect_scale(&r1, 2.0, 0.5);
	char *scaled_str = wlf_rect_to_str(&scaled);
	wlf_log(WLF_INFO, "r1 scaled by (2.0,0.5): %s", scaled_str);
	free(scaled_str);

	// Test Point Containment
	wlf_log(WLF_INFO, "\n--- Testing Point Containment ---");

	struct wlf_point test_point1 = {50, 50}; // inside r1
	struct wlf_point test_point2 = {5, 5};   // outside r1
	struct wlf_point test_point3 = {10, 20}; // on edge

	wlf_log(WLF_INFO, "r1 contains (50,50): %s", wlf_rect_contains_point(&r1, &test_point1) ? "true" : "false");
	wlf_log(WLF_INFO, "r1 contains (5,5): %s", wlf_rect_contains_point(&r1, &test_point2) ? "true" : "false");
	wlf_log(WLF_INFO, "r1 contains (10,20): %s", wlf_rect_contains_point(&r1, &test_point3) ? "true" : "false");

	// Test with double coordinates
	wlf_log(WLF_INFO, "r1 contains (50.5,50.5): %s", wlf_rect_contains_point_d(&r1, 50.5, 50.5) ? "true" : "false");
	wlf_log(WLF_INFO, "r1 contains (109.9,99.9): %s", wlf_rect_contains_point_d(&r1, 109.9, 99.9) ? "true" : "false");

	// Test Rectangle Containment
	wlf_log(WLF_INFO, "\n--- Testing Rectangle Containment ---");

	struct wlf_rect inner = wlf_rect_make(20, 30, 50, 40); // inside r1
	struct wlf_rect outer = wlf_rect_make(0, 0, 200, 200); // contains r1
	struct wlf_rect partial = wlf_rect_make(50, 50, 100, 100); // partially overlaps

	char *inner_str = wlf_rect_to_str(&inner);
	char *outer_str = wlf_rect_to_str(&outer);
	char *partial_str = wlf_rect_to_str(&partial);

	wlf_log(WLF_INFO, "r1 contains %s: %s", inner_str, wlf_rect_contains_rect(&r1, &inner) ? "true" : "false");
	wlf_log(WLF_INFO, "%s contains r1: %s", outer_str, wlf_rect_contains_rect(&outer, &r1) ? "true" : "false");
	wlf_log(WLF_INFO, "r1 contains %s: %s", partial_str, wlf_rect_contains_rect(&r1, &partial) ? "true" : "false");

	free(inner_str); free(outer_str); free(partial_str);

	// Test Intersection
	wlf_log(WLF_INFO, "\n--- Testing Intersection ---");

	struct wlf_rect rect_a = wlf_rect_make(0, 0, 50, 50);
	struct wlf_rect rect_b = wlf_rect_make(25, 25, 50, 50);
	struct wlf_rect no_overlap = wlf_rect_make(100, 100, 20, 20);

	wlf_log(WLF_INFO, "rect_a intersects rect_b: %s", wlf_rect_intersects(&rect_a, &rect_b) ? "true" : "false");
	wlf_log(WLF_INFO, "rect_a intersects no_overlap: %s", wlf_rect_intersects(&rect_a, &no_overlap) ? "true" : "false");

	struct wlf_rect intersection = wlf_rect_intersection(&rect_a, &rect_b);
	char *intersection_str = wlf_rect_to_str(&intersection);
	wlf_log(WLF_INFO, "rect_a ∩ rect_b: %s", intersection_str);
	free(intersection_str);

	// Test Union
	wlf_log(WLF_INFO, "\n--- Testing Union ---");

	struct wlf_rect union_rect = wlf_rect_union(&rect_a, &rect_b);
	char *union_str = wlf_rect_to_str(&union_rect);
	wlf_log(WLF_INFO, "rect_a ∪ rect_b: %s", union_str);
	free(union_str);

	// Test Edge Cases
	wlf_log(WLF_INFO, "\n--- Testing Edge Cases ---");

	// Zero area rectangle
	struct wlf_rect zero_area = wlf_rect_make(10, 10, 0, 0);
	wlf_log(WLF_INFO, "Zero area rectangle area: %d", wlf_rect_area(&zero_area));
	wlf_log(WLF_INFO, "Zero area rectangle is empty: %s", wlf_rect_is_empty(&zero_area) ? "true" : "false");

	// Negative dimensions
	struct wlf_rect negative = wlf_rect_make(10, 10, -20, -30);
	wlf_log(WLF_INFO, "Negative rectangle is valid: %s", wlf_rect_is_valid(&negative) ? "true" : "false");
	wlf_log(WLF_INFO, "Negative rectangle area: %d", wlf_rect_area(&negative));

	// Test large coordinates
	struct wlf_rect large = wlf_rect_make(1000000, 2000000, 500000, 300000);
	wlf_log(WLF_INFO, "Large rectangle area: %d", wlf_rect_area(&large));

	char *large_str = wlf_rect_to_str(&large);
	wlf_log(WLF_INFO, "Large rectangle: %s", large_str);
	free(large_str);
	// Test String Parsing
	wlf_log(WLF_INFO, "\n--- Testing String Parsing ---");

	struct wlf_rect parsed_rect;

	// Test basic format with parentheses (should succeed)
	if (wlf_rect_from_str("(10,20,100,80)", &parsed_rect)) {
		char *parsed_str = wlf_rect_to_str(&parsed_rect);
		wlf_log(WLF_INFO, "Parsed '(10,20,100,80)': %s", parsed_str);
		free(parsed_str);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse '(10,20,100,80)'");
	}

	// Test format with spaces and parentheses (should succeed)
	if (wlf_rect_from_str("(5, 15, 30, 25)", &parsed_rect)) {
		char *parsed_str = wlf_rect_to_str(&parsed_rect);
		wlf_log(WLF_INFO, "Parsed '(5, 15, 30, 25)': %s", parsed_str);
		free(parsed_str);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse '(5, 15, 30, 25)'");
	}

	// Test format with whitespace around parentheses (should succeed)
	if (wlf_rect_from_str("  (0,0,50,50)  ", &parsed_rect)) {
		char *parsed_str = wlf_rect_to_str(&parsed_rect);
		wlf_log(WLF_INFO, "Parsed '  (0,0,50,50)  ': %s", parsed_str);
		free(parsed_str);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse '  (0,0,50,50)  '");
	}

	// Test negative values with parentheses (should succeed)
	if (wlf_rect_from_str("(-10,-20,100,80)", &parsed_rect)) {
		char *parsed_str = wlf_rect_to_str(&parsed_rect);
		wlf_log(WLF_INFO, "Parsed '(-10,-20,100,80)': %s", parsed_str);
		wlf_log(WLF_INFO, "Is valid: %s", wlf_rect_is_valid(&parsed_rect) ? "true" : "false");
		free(parsed_str);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse '(-10,-20,100,80)'");
	}

	// Test invalid formats (should all fail)
	wlf_log(WLF_INFO, "Testing invalid formats (should all fail):");
	wlf_log(WLF_INFO, "Empty string: %s", wlf_rect_from_str("", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "NULL string: %s", wlf_rect_from_str(NULL, &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "No parentheses: %s", wlf_rect_from_str("10,20,100,80", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "Only opening paren: %s", wlf_rect_from_str("(10,20,100,80", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "Only closing paren: %s", wlf_rect_from_str("10,20,100,80)", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "Invalid format: %s", wlf_rect_from_str("(abc,def,ghi,jkl)", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "Too few values: %s", wlf_rect_from_str("(10,20,30)", &parsed_rect) ? "parsed" : "failed");
	wlf_log(WLF_INFO, "Extra text after: %s", wlf_rect_from_str("(10,20,30,40)extra", &parsed_rect) ? "parsed" : "failed");

	wlf_log(WLF_INFO, "\n=== Rectangle Test Suite Complete ===");

	return 0;
}
