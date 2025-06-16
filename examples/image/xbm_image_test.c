#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_xbm_image.h"

/**
 * @brief Create a simple test XBM image with a cross pattern.
 * @param width Image width.
 * @param height Image height.
 * @return Pointer to the created XBM image, or NULL on failure.
 */
static struct wlf_xbm_image *create_test_image(uint32_t width, uint32_t height) {
	struct wlf_xbm_image *xbm_image = wlf_xbm_image_create();
	if (!xbm_image) {
		return NULL;
	}

	xbm_image->base.width = width;
	xbm_image->base.height = height;
	xbm_image->base.format = WLF_COLOR_TYPE_GRAY;
	xbm_image->base.bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	xbm_image->base.stride = width;
	xbm_image->base.has_alpha_channel = false;
	xbm_image->base.is_opaque = true;

	// Allocate pixel data (grayscale)
	size_t data_size = width * height;
	xbm_image->base.data = malloc(data_size);
	if (!xbm_image->base.data) {
		free(xbm_image);
		return NULL;
	}

	// Create a cross pattern (black on white)
	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			uint32_t offset = y * width + x;

			// Create a cross pattern
			bool is_cross = (x == width / 2) || (y == height / 2) ||
			               (x == y) || (x == width - 1 - y);

			// Create border
			bool is_border = (x == 0 || x == width - 1 || y == 0 || y == height - 1);

			if (is_cross || is_border) {
				xbm_image->base.data[offset] = 0;   // Black (foreground)
			} else {
				xbm_image->base.data[offset] = 255; // White (background)
			}
		}
	}

	return xbm_image;
}

/**
 * @brief Create a cursor-like XBM image with hotspot.
 * @param size Size of the square cursor.
 * @return Pointer to the created XBM image, or NULL on failure.
 */
static struct wlf_xbm_image *create_cursor_image(uint32_t size) {
	struct wlf_xbm_image *xbm_image = wlf_xbm_image_create();
	if (!xbm_image) {
		return NULL;
	}

	xbm_image->base.width = size;
	xbm_image->base.height = size;
	xbm_image->base.format = WLF_COLOR_TYPE_GRAY;
	xbm_image->base.bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	xbm_image->base.stride = size;
	xbm_image->base.has_alpha_channel = false;
	xbm_image->base.is_opaque = true;

	// Set hotspot at center
	wlf_xbm_image_set_hotspot(xbm_image, size / 2, size / 2);
	wlf_xbm_image_set_name(xbm_image, "cursor");

	// Allocate pixel data
	size_t data_size = size * size;
	xbm_image->base.data = malloc(data_size);
	if (!xbm_image->base.data) {
		free(xbm_image);
		return NULL;
	}

	// Create arrow cursor pattern
	for (uint32_t y = 0; y < size; y++) {
		for (uint32_t x = 0; x < size; x++) {
			uint32_t offset = y * size + x;

			// Simple arrow pattern
			bool is_arrow = false;

			// Vertical line
			if (x == size / 4 && y < size * 3 / 4) {
				is_arrow = true;
			}
			// Horizontal line at top
			else if (y == size / 4 && x < size / 2) {
				is_arrow = true;
			}
			// Arrow head
			else if (y < size / 2 && x < size / 2 && (x + y) > (size / 2 - 2)) {
				is_arrow = true;
			}

			xbm_image->base.data[offset] = is_arrow ? 0 : 255;
		}
	}

	return xbm_image;
}

int main(int argc, char *argv[]) {
	printf("XBM Image Test\n");
	printf("==============\n\n");

	// Test 1: Create and save a simple XBM image
	printf("Test 1: Creating a test XBM image...\n");
	struct wlf_xbm_image *test_image = create_test_image(32, 32);
	if (!test_image) {
		printf("Failed to create test image!\n");
		return 1;
	}

	// Set a custom name
	wlf_xbm_image_set_name(test_image, "test_pattern");
	printf("✓ Test image created successfully\n");

	// Save XBM image
	const char *filename = "test_pattern.xbm";
	printf("\nTest 2: Saving XBM image...\n");
	if (wlf_image_save((struct wlf_image *)test_image, filename)) {
		printf("✓ XBM image saved as '%s'\n", filename);
	} else {
		printf("✗ Failed to save XBM image\n");
	}

	// Test 3: Load the XBM image
	printf("\nTest 3: Loading XBM image...\n");
	struct wlf_image *loaded_image = wlf_image_load(filename);
	if (loaded_image) {
		printf("✓ XBM image loaded successfully\n");
		printf("  - Width: %u\n", loaded_image->width);
		printf("  - Height: %u\n", loaded_image->height);
		printf("  - Format: %s\n",
			loaded_image->format == WLF_COLOR_TYPE_GRAY ? "Grayscale" : "Other");
		printf("  - Type: %s\n", wlf_image_get_type_string(loaded_image));

		// Verify it's an XBM image
		if (wlf_image_is_xbm(loaded_image)) {
			printf("✓ Image correctly identified as XBM\n");
			struct wlf_xbm_image *loaded_xbm = wlf_xbm_image_from_image(loaded_image);
			printf("  - Name: %s\n", loaded_xbm->name ? loaded_xbm->name : "(none)");
			printf("  - Has hotspot: %s\n", loaded_xbm->has_hotspot ? "Yes" : "No");
			if (loaded_xbm->has_hotspot) {
				printf("  - Hotspot: (%d, %d)\n", loaded_xbm->hotspot.x, loaded_xbm->hotspot.y);
			}
		} else {
			printf("✗ Image not identified as XBM\n");
		}
	} else {
		printf("✗ Failed to load XBM image\n");
	}

	// Test 4: Create and test cursor image with hotspot
	printf("\nTest 4: Creating cursor XBM image with hotspot...\n");
	struct wlf_xbm_image *cursor_image = create_cursor_image(16);
	if (cursor_image) {
		const char *cursor_filename = "cursor.xbm";
		if (wlf_image_save((struct wlf_image *)cursor_image, cursor_filename)) {
			printf("✓ Cursor XBM saved as '%s'\n", cursor_filename);

			// Load and verify cursor
			struct wlf_image *loaded_cursor = wlf_image_load(cursor_filename);
			if (loaded_cursor && wlf_image_is_xbm(loaded_cursor)) {
				struct wlf_xbm_image *loaded_cursor_xbm = wlf_xbm_image_from_image(loaded_cursor);
				printf("✓ Cursor loaded, name: %s\n",
					loaded_cursor_xbm->name ? loaded_cursor_xbm->name : "(none)");
				if (loaded_cursor_xbm->has_hotspot) {
					printf("✓ Hotspot preserved: (%d, %d)\n",
						loaded_cursor_xbm->hotspot.x, loaded_cursor_xbm->hotspot.y);
				} else {
					printf("✗ Hotspot not preserved\n");
				}

				wlf_image_finish(loaded_cursor);
				free(loaded_cursor);
			}
		}

		wlf_image_finish((struct wlf_image *)cursor_image);
		free(cursor_image);
	}

	// Test 5: Compare original and loaded image data
	if (loaded_image && test_image) {
		printf("\nTest 5: Comparing original and loaded images...\n");

		if (loaded_image->width == test_image->base.width &&
		    loaded_image->height == test_image->base.height &&
		    loaded_image->format == test_image->base.format) {
			printf("✓ Image dimensions and format match\n");

			// Sample a few pixels to verify data integrity
			bool data_matches = true;
			uint32_t samples = 10;
			for (uint32_t i = 0; i < samples && data_matches; i++) {
				uint32_t offset = (i * loaded_image->width * loaded_image->height) / samples;

				if (offset < loaded_image->width * loaded_image->height) {
					// For XBM, we convert to monochrome and back, so exact match might not occur
					// Check if both are on the same side of the threshold
					bool orig_black = test_image->base.data[offset] < 128;
					bool loaded_black = loaded_image->data[offset] < 128;

					if (orig_black != loaded_black) {
						data_matches = false;
					}
				}
			}

			if (data_matches) {
				printf("✓ Sampled pixel data matches (considering monochrome conversion)\n");
			} else {
				printf("✗ Pixel data doesn't match\n");
			}
		} else {
			printf("✗ Image dimensions or format don't match\n");
		}
	}

	// Test 6: Show saved XBM file content (first few lines)
	printf("\nTest 6: XBM file content preview:\n");
	FILE *fp = fopen(filename, "r");
	if (fp) {
		char line[256];
		int line_count = 0;
		printf("--- %s ---\n", filename);
		while (fgets(line, sizeof(line), fp) && line_count < 6) {
			printf("%s", line);
			line_count++;
		}
		printf("...\n--- End Preview ---\n");
		fclose(fp);
	}

	// Cleanup
	wlf_image_finish((struct wlf_image *)test_image);
	free(test_image);

	if (loaded_image) {
		wlf_image_finish(loaded_image);
		free(loaded_image);
	}

	printf("\nXBM image test completed!\n");
	return 0;
}
