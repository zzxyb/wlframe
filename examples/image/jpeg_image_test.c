#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_jpeg_image.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void create_test_jpeg(const char *filename) {
	// Create a simple test JPEG image
	struct wlf_jpeg_image *jpeg_image = wlf_jpeg_image_create();
	if (!jpeg_image) {
		wlf_log(WLF_ERROR, "Failed to create JPEG image");
		return;
	}

	struct wlf_image *base = &jpeg_image->base;

	// Set up image properties
	base->width = 100;
	base->height = 100;
	base->format = WLF_COLOR_TYPE_RGB;
	base->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	base->stride = base->width * 3;
	base->image_type = WLF_IMAGE_TYPE_JPEG;
	base->has_alpha_channel = false;
	base->is_opaque = true;

	// Allocate image data
	base->data = malloc(base->height * base->stride);
	if (!base->data) {
		wlf_log(WLF_ERROR, "Failed to allocate image data");
		free(jpeg_image);
		return;
	}

	// Create a simple gradient pattern
	for (uint32_t y = 0; y < base->height; y++) {
		for (uint32_t x = 0; x < base->width; x++) {
			unsigned char *pixel = base->data + y * base->stride + x * 3;
			pixel[0] = (unsigned char)(x * 255 / base->width);        // Red gradient
			pixel[1] = (unsigned char)(y * 255 / base->height);       // Green gradient
			pixel[2] = (unsigned char)((x + y) * 128 / (base->width + base->height)); // Blue
		}
	}

	// Set JPEG quality
	wlf_jpeg_image_set_quality(jpeg_image, 85);

	// Save the image
	if (base->impl->save(base, filename)) {
		wlf_log(WLF_INFO, "✓ Created test JPEG: %s", filename);
	} else {
		wlf_log(WLF_ERROR, "✗ Failed to save test JPEG: %s", filename);
	}

	// Cleanup
	free(base->data);
	free(jpeg_image);
}

static void print_image_info(const struct wlf_image *image, const char *filename) {
	printf("Image information for %s:\n", filename);
	printf("  Dimensions: %dx%d\n", image->width, image->height);
	printf("  Format: %d\n", image->format);
	printf("  Bit depth: %d\n", image->bit_depth);
	printf("  Stride: %d bytes\n", image->stride);
	printf("  Has alpha: %s\n", image->has_alpha_channel ? "Yes" : "No");
	printf("  Is opaque: %s\n", image->is_opaque ? "Yes" : "No");
	printf("  Type: %s\n", wlf_image_get_type_string(image));
	printf("  Channels: %d\n", wlf_image_get_channels(image));
}

int main(int argc, char *argv[]) {
	int32_t log_level = WLF_INFO;
	wlf_log_init(log_level, NULL);

	printf("wlframe JPEG Image Test\n");
	printf("=======================\n");

	if (argc > 1) {
		// Test loading provided JPEG file
		const char *filename = argv[1];
		printf("\nTesting JPEG load: %s\n", filename);

		struct wlf_image *img = wlf_image_load(filename);
		if (img == NULL) {
			wlf_log(WLF_ERROR, "Failed to load image: %s", filename);
			return EXIT_FAILURE;
		}

		print_image_info(img, filename);

		// Save as another JPEG file
		char output_filename[256];
		snprintf(output_filename, sizeof(output_filename), "loaded_%s", filename);

		if (wlf_image_save(img, output_filename)) {
			wlf_log(WLF_INFO, "✓ Saved loaded image as: %s", output_filename);
		} else {
			wlf_log(WLF_ERROR, "✗ Failed to save image as: %s", output_filename);
		}

		wlf_image_finish(img);
	} else {
		// Create and test a simple JPEG image
		printf("\nCreating test JPEG images...\n");

		create_test_jpeg("test_gradient.jpg");

		// Load it back and verify
		printf("\nTesting JPEG load/save cycle...\n");
		struct wlf_image *img = wlf_image_load("test_gradient.jpg");
		if (img) {
			print_image_info(img, "test_gradient.jpg");

			// Save it again with different quality
			if (img->image_type == WLF_IMAGE_TYPE_JPEG) {
				struct wlf_jpeg_image *jpeg_img = wlf_jpeg_image_from_image(img);
				if (jpeg_img) {
					wlf_jpeg_image_set_quality(jpeg_img, 95);
					if (wlf_image_save(img, "test_gradient_hq.jpg")) {
						wlf_log(WLF_INFO, "✓ Saved high quality version: test_gradient_hq.jpg");
					}
				}
			}

			wlf_image_finish(img);
		} else {
			wlf_log(WLF_ERROR, "Failed to load back the test image");
		}
	}

	printf("\nJPEG test completed!\n");
	return EXIT_SUCCESS;
}
