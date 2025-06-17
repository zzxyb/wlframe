#include "wlf/math/wlf_size.h"
#include "wlf/utils/wlf_log.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);

	wlf_log(WLF_INFO, "=== WLF Size Test Suite ===");

	// Test size creation
	wlf_log(WLF_INFO, "\n--- Testing Size Creation ---");
	struct wlf_size s1 = (struct wlf_size) {.width = 800, .height = 600};
	struct wlf_size s2 = (struct wlf_size) {.width = 1920, .height = 1080};
	struct wlf_size zero_size = (struct wlf_size) {.width = 0, .height = 0};
	struct wlf_size invalid_size = (struct wlf_size) {.width = -10, .height= 20};

	char *s1_str = wlf_size_to_str(&s1);
	char *s2_str = wlf_size_to_str(&s2);
	char *zero_str = wlf_size_to_str(&zero_size);
	char *invalid_str = wlf_size_to_str(&invalid_size);

	wlf_log(WLF_INFO, "s1: %s", s1_str);
	wlf_log(WLF_INFO, "s2: %s", s2_str);
	wlf_log(WLF_INFO, "zero: %s", zero_str);
	wlf_log(WLF_INFO, "invalid: %s", invalid_str);

	free(s1_str);
	free(s2_str);
	free(zero_str);
	free(invalid_str);

	// Test constants
	wlf_log(WLF_INFO, "\n--- Testing Constants ---");
	char *zero_const_str = wlf_size_to_str(&WLF_SIZE_ZERO);
	char *unit_str = wlf_size_to_str(&WLF_SIZE_UNIT);

	wlf_log(WLF_INFO, "WLF_SIZE_ZERO: %s", zero_const_str);
	wlf_log(WLF_INFO, "WLF_SIZE_UNIT: %s", unit_str);

	free(zero_const_str);
	free(unit_str);

	// Test equality
	wlf_log(WLF_INFO, "\n--- Testing Equality ---");
	struct wlf_size s1_copy = (struct wlf_size) {.width = 800, .height = 600};
	bool equal = wlf_size_equal(&s1, &s1_copy);
	bool not_equal = wlf_size_equal(&s1, &s2);
	wlf_log(WLF_INFO, "s1 == s1_copy: %s", equal ? "true" : "false");
	wlf_log(WLF_INFO, "s1 == s2: %s", not_equal ? "true" : "false");

	// Test validity and emptiness checks
	wlf_log(WLF_INFO, "\n--- Testing Validity and Emptiness ---");
	bool s1_valid = wlf_size_is_valid(&s1);
	bool s1_empty = wlf_size_is_empty(&s1);
	bool zero_valid = wlf_size_is_valid(&zero_size);
	bool zero_empty = wlf_size_is_empty(&zero_size);
	bool invalid_valid = wlf_size_is_valid(&invalid_size);
	bool invalid_empty = wlf_size_is_empty(&invalid_size);

	wlf_log(WLF_INFO, "s1 is valid: %s, is empty: %s",
		s1_valid ? "true" : "false", s1_empty ? "true" : "false");
	wlf_log(WLF_INFO, "zero is valid: %s, is empty: %s",
		zero_valid ? "true" : "false", zero_empty ? "true" : "false");
	wlf_log(WLF_INFO, "invalid is valid: %s, is empty: %s",
		invalid_valid ? "true" : "false", invalid_empty ? "true" : "false");

	// Test arithmetic operations
	wlf_log(WLF_INFO, "\n--- Testing Arithmetic Operations ---");
	struct wlf_size sum = wlf_size_add(&s1, &s2);
	struct wlf_size diff = wlf_size_subtract(&s2, &s1);
	struct wlf_size scaled = wlf_size_multiply(&s1, 2);
	struct wlf_size divided = wlf_size_divide(&s1, 2);

	char *sum_str = wlf_size_to_str(&sum);
	char *diff_str = wlf_size_to_str(&diff);
	char *scaled_str = wlf_size_to_str(&scaled);
	char *divided_str = wlf_size_to_str(&divided);

	wlf_log(WLF_INFO, "s1 + s2 = %s", sum_str);
	wlf_log(WLF_INFO, "s2 - s1 = %s", diff_str);
	wlf_log(WLF_INFO, "s1 * 2 = %s", scaled_str);
	wlf_log(WLF_INFO, "s1 / 2 = %s", divided_str);

	free(sum_str);
	free(diff_str);
	free(scaled_str);
	free(divided_str);

	// Test area calculation
	wlf_log(WLF_INFO, "\n--- Testing Area Calculation ---");
	int s1_area = wlf_size_area(&s1);
	int s2_area = wlf_size_area(&s2);
	int zero_area = wlf_size_area(&zero_size);

	wlf_log(WLF_INFO, "Area of s1 (800x600): %d", s1_area);
	wlf_log(WLF_INFO, "Area of s2 (1920x1080): %d", s2_area);
	wlf_log(WLF_INFO, "Area of zero size: %d", zero_area);

	// Test common screen resolutions
	wlf_log(WLF_INFO, "\n--- Testing Common Screen Resolutions ---");
	struct wlf_size vga = (struct wlf_size) {.width = 640, .height = 480};
	struct wlf_size hd = (struct wlf_size) {.width = 1280, .height = 720};
	struct wlf_size full_hd = (struct wlf_size) {.width = 1920, .height = 1080};
	struct wlf_size quad_hd = (struct wlf_size) {.width = 2560, .height = 1440};
	struct wlf_size ultra_hd = (struct wlf_size) {.width = 3840, .height = 2160};

	char *vga_str = wlf_size_to_str(&vga);
	char *hd_str = wlf_size_to_str(&hd);
	char *full_hd_str = wlf_size_to_str(&full_hd);
	char *quad_hd_str = wlf_size_to_str(&quad_hd);
	char *ultra_hd_str = wlf_size_to_str(&ultra_hd);

	wlf_log(WLF_INFO, "VGA: %s (area: %d)", vga_str, wlf_size_area(&vga));
	wlf_log(WLF_INFO, "HD: %s (area: %d)", hd_str, wlf_size_area(&hd));
	wlf_log(WLF_INFO, "Full HD: %s (area: %d)", full_hd_str, wlf_size_area(&full_hd));
	wlf_log(WLF_INFO, "Quad HD: %s (area: %d)", quad_hd_str, wlf_size_area(&quad_hd));
	wlf_log(WLF_INFO, "Ultra HD: %s (area: %d)", ultra_hd_str, wlf_size_area(&ultra_hd));

	free(vga_str);
	free(hd_str);
	free(full_hd_str);
	free(quad_hd_str);
	free(ultra_hd_str);

	// Test edge cases
	wlf_log(WLF_INFO, "\n--- Testing Edge Cases ---");

	// Division by zero
	struct wlf_size div_by_zero = wlf_size_divide(&s1, 0);
	char *div_by_zero_str = wlf_size_to_str(&div_by_zero);
	wlf_log(WLF_INFO, "Division by zero result: %s", div_by_zero_str);
	free(div_by_zero_str);

	// Negative operations
	struct wlf_size neg_scaled = wlf_size_multiply(&s1, -1);
	char *neg_scaled_str = wlf_size_to_str(&neg_scaled);
	wlf_log(WLF_INFO, "s1 * -1 = %s", neg_scaled_str);
	free(neg_scaled_str);

	// Large numbers
	struct wlf_size large = (struct wlf_size) {.width = 100000, .height = 100000};
	int large_area = wlf_size_area(&large);
	char *large_str = wlf_size_to_str(&large);
	wlf_log(WLF_INFO, "Large size: %s (area: %d)", large_str, large_area);
	free(large_str);

	// Test aspect ratios
	wlf_log(WLF_INFO, "\n--- Testing Aspect Ratios ---");
	struct wlf_size square = (struct wlf_size) {.width = 512, .height = 512};
	struct wlf_size wide = (struct wlf_size) {.width = 1920, .height = 800};
	struct wlf_size tall = (struct wlf_size) {.width = 600, .height = 1200};

	char *square_str = wlf_size_to_str(&square);
	char *wide_str = wlf_size_to_str(&wide);
	char *tall_str = wlf_size_to_str(&tall);

	wlf_log(WLF_INFO, "Square: %s (ratio: %.2f)", square_str, (double)square.width / square.height);
	wlf_log(WLF_INFO, "Wide: %s (ratio: %.2f)", wide_str, (double)wide.width / wide.height);
	wlf_log(WLF_INFO, "Tall: %s (ratio: %.2f)", tall_str, (double)tall.width / tall.height);

	free(square_str);
	free(wide_str);
	free(tall_str);

	// Test scaling operations
	wlf_log(WLF_INFO, "\n--- Testing Scaling Operations ---");
	struct wlf_size original = (struct wlf_size) {.width = 100, .height = 50};
	struct wlf_size double_size = wlf_size_multiply(&original, 2);
	struct wlf_size triple_size = wlf_size_multiply(&original, 3);
	struct wlf_size half_size = wlf_size_divide(&original, 2);

	char *original_str = wlf_size_to_str(&original);
	char *double_str = wlf_size_to_str(&double_size);
	char *triple_str = wlf_size_to_str(&triple_size);
	char *half_str = wlf_size_to_str(&half_size);

	wlf_log(WLF_INFO, "Original: %s", original_str);
	wlf_log(WLF_INFO, "2x scale: %s", double_str);
	wlf_log(WLF_INFO, "3x scale: %s", triple_str);
	wlf_log(WLF_INFO, "0.5x scale: %s", half_str);

	free(original_str);
	free(double_str);
	free(triple_str);
	free(half_str);

	wlf_log(WLF_INFO, "\n=== Size Test Suite Complete ===");

	return 0;
}
