#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_bmp_image.h"

/**
 * @brief Create a simple test BMP image with a checkerboard pattern.
 * @param width Image width.
 * @param height Image height.
 * @return Pointer to the created BMP image, or NULL on failure.
 */
static struct wlf_bmp_image *create_test_image(uint32_t width, uint32_t height) {
	struct wlf_bmp_image *bmp_image = wlf_bmp_image_create();
	if (!bmp_image) {
		return NULL;
	}

	bmp_image->base.width = width;
	bmp_image->base.height = height;
	bmp_image->base.format = WLF_COLOR_TYPE_RGB;
	bmp_image->base.bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	bmp_image->base.stride = width * 3;
	bmp_image->base.has_alpha_channel = false;
	bmp_image->base.is_opaque = true;

	// Allocate pixel data
	size_t data_size = width * height * 3;
	bmp_image->base.data = malloc(data_size);
	if (!bmp_image->base.data) {
		free(bmp_image);
		return NULL;
	}

	// Create a checkerboard pattern
	uint32_t block_size = 32;
	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			uint32_t offset = (y * width + x) * 3;

			// Determine if this pixel is in a "black" or "colored" block
			bool black_block = ((x / block_size) + (y / block_size)) % 2 == 0;

			if (black_block) {
				// Black block
				bmp_image->base.data[offset] = 0;     // R
				bmp_image->base.data[offset + 1] = 0; // G
				bmp_image->base.data[offset + 2] = 0; // B
			} else {
				// Colored block with gradient
				bmp_image->base.data[offset] = (unsigned char)((x * 255) / width);        // R
				bmp_image->base.data[offset + 1] = (unsigned char)((y * 255) / height);  // G
				bmp_image->base.data[offset + 2] = (unsigned char)(((x + y) * 255) / (width + height)); // B
			}
		}
	}

	return bmp_image;
}

int main(int argc, char *argv[]) {
	printf("BMP Image Test\n");
	printf("==============\n\n");

	// Test 1: Create and save a BMP image
	printf("Test 1: Creating a test BMP image...\n");
	struct wlf_bmp_image *test_image = create_test_image(256, 256);
	if (!test_image) {
		printf("Failed to create test image!\n");
		return 1;
	}
	printf("✓ Test image created successfully\n");

	// Save BMP image
	const char *filename = "test_image.bmp";
	printf("\nTest 2: Saving BMP image...\n");
	if (wlf_image_save((struct wlf_image *)test_image, filename)) {
		printf("✓ BMP image saved as '%s'\n", filename);
	} else {
		printf("✗ Failed to save BMP image\n");
	}

	// Test 3: Load the BMP image
	printf("\nTest 3: Loading BMP image...\n");
	struct wlf_image *loaded_image = wlf_image_load(filename);
	if (loaded_image) {
		printf("✓ BMP image loaded successfully\n");
		printf("  - Width: %u\n", loaded_image->width);
		printf("  - Height: %u\n", loaded_image->height);
		printf("  - Format: %s\n",
			loaded_image->format == WLF_COLOR_TYPE_RGB ? "RGB" : "Other");
		printf("  - Type: %s\n", wlf_image_get_type_string(loaded_image));
		printf("  - Bit depth: %d\n", loaded_image->bit_depth);

		// Verify it's a BMP image
		if (wlf_image_is_bmp(loaded_image)) {
			printf("✓ Image correctly identified as BMP\n");
			struct wlf_bmp_image *loaded_bmp = wlf_bmp_image_from_image(loaded_image);
			printf("  - Compression: %s\n",
				loaded_bmp->compression == WLF_BMP_COMPRESSION_RGB ? "RGB (No compression)" : "Other");
			printf("  - Bits per pixel: %u\n", loaded_bmp->bits_per_pixel);
			printf("  - Top-down: %s\n", loaded_bmp->top_down ? "Yes" : "No");
		} else {
			printf("✗ Image not identified as BMP\n");
		}
	} else {
		printf("✗ Failed to load BMP image\n");
	}

	// Test 4: Compare original and loaded image data
	if (loaded_image && test_image) {
		printf("\nTest 4: Comparing original and loaded images...\n");

		if (loaded_image->width == test_image->base.width &&
		    loaded_image->height == test_image->base.height &&
		    loaded_image->format == test_image->base.format) {
			printf("✓ Image dimensions and format match\n");

			// Sample a few pixels to verify data integrity
			bool data_matches = true;
			uint32_t samples = 10;
			for (uint32_t i = 0; i < samples && data_matches; i++) {
				uint32_t x = (i * loaded_image->width) / samples;
				uint32_t y = (i * loaded_image->height) / samples;
				uint32_t offset = (y * loaded_image->width + x) * 3;

				if (offset + 2 < loaded_image->width * loaded_image->height * 3) {
					if (loaded_image->data[offset] != test_image->base.data[offset] ||
					    loaded_image->data[offset + 1] != test_image->base.data[offset + 1] ||
					    loaded_image->data[offset + 2] != test_image->base.data[offset + 2]) {
						data_matches = false;
					}
				}
			}

			if (data_matches) {
				printf("✓ Sampled pixel data matches\n");
			} else {
				printf("✗ Pixel data doesn't match\n");
			}
		} else {
			printf("✗ Image dimensions or format don't match\n");
		}
	}

	// Test 5: Test different BMP settings
	printf("\nTest 5: Testing BMP settings...\n");

	// Test top-down orientation
	struct wlf_bmp_image *test_image2 = create_test_image(64, 64);
	if (test_image2) {
		test_image2->top_down = true;
		const char *topdown_filename = "test_topdown.bmp";

		if (wlf_image_save((struct wlf_image *)test_image2, topdown_filename)) {
			printf("✓ Top-down BMP image saved\n");

			struct wlf_image *loaded_topdown = wlf_image_load(topdown_filename);
			if (loaded_topdown && wlf_image_is_bmp(loaded_topdown)) {
				struct wlf_bmp_image *loaded_bmp = wlf_bmp_image_from_image(loaded_topdown);
				printf("✓ Top-down BMP loaded, orientation: %s\n",
					loaded_bmp->top_down ? "Top-down" : "Bottom-up");

				wlf_image_finish(loaded_topdown);
				free(loaded_topdown);
			}
		}

		wlf_image_finish((struct wlf_image *)test_image2);
		free(test_image2);
	}

	// Cleanup
	wlf_image_finish((struct wlf_image *)test_image);
	free(test_image);

	if (loaded_image) {
		wlf_image_finish(loaded_image);
		free(loaded_image);
	}

	printf("\nBMP image test completed!\n");
	return 0;
}
