#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_xpm_image.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

static struct wlf_image *create_simple_icon(uint32_t width, uint32_t height) {
	struct wlf_xpm_image *xpm_image = wlf_xpm_image_create();
	if (!xpm_image) {
		wlf_log(WLF_ERROR, "Failed to create XPM image");
		return NULL;
	}

	struct wlf_image *base = &xpm_image->base;
	base->width = width;
	base->height = height;
	base->format = WLF_COLOR_TYPE_RGB;
	base->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	base->stride = width * 3;
	base->image_type = WLF_IMAGE_TYPE_XPM;
	base->has_alpha_channel = false;
	base->is_opaque = true;

	// Set XPM specific properties
	wlf_xpm_image_set_name(xpm_image, "simple_icon");
	wlf_xpm_image_set_colors_per_pixel(xpm_image, 1);

	base->data = malloc(height * base->stride);
	if (!base->data) {
		wlf_log(WLF_ERROR, "Failed to allocate image data");
		free(xpm_image);
		return NULL;
	}

	// Create a simple cross pattern
	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			unsigned char *pixel = base->data + y * base->stride + x * 3;

			// Draw a cross: vertical and horizontal lines
			if ((x == width/2) || (y == height/2)) {
				// Red cross
				pixel[0] = 255; // Red
				pixel[1] = 0;   // Green
				pixel[2] = 0;   // Blue
			} else {
				// White background
				pixel[0] = 255; // Red
				pixel[1] = 255; // Green
				pixel[2] = 255; // Blue
			}
		}
	}

	return base;
}

static bool test_image_properties(struct wlf_image *image, uint32_t expected_width, uint32_t expected_height) {
	if (!image) {
		wlf_log(WLF_ERROR, "Image is NULL");
		return false;
	}

	if (!wlf_image_is_xpm(image)) {
		wlf_log(WLF_ERROR, "Image is not recognized as XPM");
		return false;
	}

	if (image->width != expected_width) {
		wlf_log(WLF_ERROR, "Width mismatch: expected %u, got %u", expected_width, image->width);
		return false;
	}

	if (image->height != expected_height) {
		wlf_log(WLF_ERROR, "Height mismatch: expected %u, got %u", expected_height, image->height);
		return false;
	}

	if (image->image_type != WLF_IMAGE_TYPE_XPM) {
		wlf_log(WLF_ERROR, "Image type mismatch: expected XPM");
		return false;
	}

	return true;
}

int main(void) {
	wlf_log(WLF_INFO, "Starting XPM image test...");

	const uint32_t test_width = 32;
	const uint32_t test_height = 32;
	const char *test_filename = "test_icon.xpm";

	// Test 1: Create XPM image
	wlf_log(WLF_INFO, "Test 1: Creating XPM image...");
	struct wlf_image *image = create_simple_icon(test_width, test_height);
	if (!image) {
		wlf_log(WLF_ERROR, "Failed to create XPM image");
		return 1;
	}

	// Test 2: Verify image properties
	wlf_log(WLF_INFO, "Test 2: Verifying image properties...");
	if (!test_image_properties(image, test_width, test_height)) {
		wlf_log(WLF_ERROR, "Image properties test failed");
		wlf_image_finish(image);
		return 1;
	}

	// Test 3: Save XPM image
	wlf_log(WLF_INFO, "Test 3: Saving XPM image to %s...", test_filename);
	if (!wlf_image_save(image, test_filename)) {
		wlf_log(WLF_ERROR, "Failed to save XPM image");
		wlf_image_finish(image);
		return 1;
	}

	wlf_image_finish(image);

	// Test 4: Load XPM image
	wlf_log(WLF_INFO, "Test 4: Loading XPM image from %s...", test_filename);
	struct wlf_image *loaded_image = wlf_image_load(test_filename);
	if (!loaded_image) {
		wlf_log(WLF_ERROR, "Failed to load XPM image");
		return 1;
	}

	// Test 5: Verify loaded image properties
	wlf_log(WLF_INFO, "Test 5: Verifying loaded image properties...");
	if (!test_image_properties(loaded_image, test_width, test_height)) {
		wlf_log(WLF_ERROR, "Loaded image properties test failed");
		wlf_image_finish(loaded_image);
		return 1;
	}

	wlf_image_finish(loaded_image);

	wlf_log(WLF_INFO, "All XPM image tests passed!");
	return 0;
}
