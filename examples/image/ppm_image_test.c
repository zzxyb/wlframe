#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_ppm_image.h"

/**
 * @brief Create a simple test PPM image with a gradient pattern.
 * @param width Image width.
 * @param height Image height.
 * @return Pointer to the created PPM image, or NULL on failure.
 */
static struct wlf_ppm_image *create_test_image(uint32_t width, uint32_t height) {
	struct wlf_ppm_image *ppm_image = wlf_ppm_image_create();
	if (!ppm_image) {
		return NULL;
	}

	ppm_image->base.width = width;
	ppm_image->base.height = height;
	ppm_image->base.format = WLF_COLOR_TYPE_RGB;
	ppm_image->base.bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	ppm_image->base.stride = width * 3;
	ppm_image->base.has_alpha_channel = false;
	ppm_image->base.is_opaque = true;

	// Allocate pixel data
	size_t data_size = width * height * 3;
	ppm_image->base.data = malloc(data_size);
	if (!ppm_image->base.data) {
		free(ppm_image);
		return NULL;
	}

	// Create a simple gradient pattern
	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			uint32_t offset = (y * width + x) * 3;

			// Red gradient based on x position
			ppm_image->base.data[offset] = (unsigned char)((x * 255) / width);

			// Green gradient based on y position
			ppm_image->base.data[offset + 1] = (unsigned char)((y * 255) / height);

			// Blue is a combination
			ppm_image->base.data[offset + 2] = (unsigned char)(((x + y) * 255) / (width + height));
		}
	}

	return ppm_image;
}

int main(int argc, char *argv[]) {
	printf("PPM Image Test\n");
	printf("==============\n\n");

	// Test 1: Create and save a PPM image
	printf("Test 1: Creating a test PPM image...\n");
	struct wlf_ppm_image *test_image = create_test_image(256, 256);
	if (!test_image) {
		printf("Failed to create test image!\n");
		return 1;
	}
	printf("✓ Test image created successfully\n");

	// Test both ASCII and binary formats
	const char *ascii_filename = "test_ascii.ppm";
	const char *binary_filename = "test_binary.ppm";

	// Save in ASCII format (P3)
	printf("\nTest 2: Saving image in ASCII format (P3)...\n");
	wlf_ppm_image_set_format(test_image, WLF_PPM_FORMAT_P3);
	if (wlf_image_save((struct wlf_image *)test_image, ascii_filename)) {
		printf("✓ ASCII PPM saved as '%s'\n", ascii_filename);
	} else {
		printf("✗ Failed to save ASCII PPM\n");
	}

	// Save in binary format (P6)
	printf("\nTest 3: Saving image in binary format (P6)...\n");
	wlf_ppm_image_set_format(test_image, WLF_PPM_FORMAT_P6);
	if (wlf_image_save((struct wlf_image *)test_image, binary_filename)) {
		printf("✓ Binary PPM saved as '%s'\n", binary_filename);
	} else {
		printf("✗ Failed to save binary PPM\n");
	}

	// Test 4: Load the binary PPM image
	printf("\nTest 4: Loading binary PPM image...\n");
	struct wlf_image *loaded_image = wlf_image_load(binary_filename);
	if (loaded_image) {
		printf("✓ PPM image loaded successfully\n");
		printf("  - Width: %u\n", loaded_image->width);
		printf("  - Height: %u\n", loaded_image->height);
		printf("  - Format: %s\n",
			loaded_image->format == WLF_COLOR_TYPE_RGB ? "RGB" : "Other");
		printf("  - Type: %s\n", wlf_image_get_type_string(loaded_image));

		// Verify it's a PPM image
		if (wlf_image_is_ppm(loaded_image)) {
			printf("✓ Image correctly identified as PPM\n");
			struct wlf_ppm_image *loaded_ppm = wlf_ppm_image_from_image(loaded_image);
			printf("  - PPM Format: %s\n",
				loaded_ppm->format == WLF_PPM_FORMAT_P6 ? "P6 (Binary)" : "P3 (ASCII)");
			printf("  - Max Value: %u\n", loaded_ppm->max_val);
		} else {
			printf("✗ Image not identified as PPM\n");
		}
	} else {
		printf("✗ Failed to load PPM image\n");
	}

	// Test 5: Load the ASCII PPM image
	printf("\nTest 5: Loading ASCII PPM image...\n");
	struct wlf_image *loaded_ascii = wlf_image_load(ascii_filename);
	if (loaded_ascii) {
		printf("✓ ASCII PPM image loaded successfully\n");
		if (wlf_image_is_ppm(loaded_ascii)) {
			struct wlf_ppm_image *loaded_ppm = wlf_ppm_image_from_image(loaded_ascii);
			printf("  - PPM Format: %s\n",
				loaded_ppm->format == WLF_PPM_FORMAT_P3 ? "P3 (ASCII)" : "P6 (Binary)");
		}
	} else {
		printf("✗ Failed to load ASCII PPM image\n");
	}

	// Cleanup
	wlf_image_finish((struct wlf_image *)test_image);
	free(test_image);

	if (loaded_image) {
		wlf_image_finish(loaded_image);
		free(loaded_image);
	}

	if (loaded_ascii) {
		wlf_image_finish(loaded_ascii);
		free(loaded_ascii);
	}

	printf("\nPPM image test completed!\n");
	return 0;
}
