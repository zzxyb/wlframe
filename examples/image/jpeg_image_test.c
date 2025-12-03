#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_jpeg_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_cmd_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void create_test_jpeg(const char *filename, int quality) {
	// Create a simple test JPEG image
	struct wlf_jpeg_image *jpeg_image = wlf_jpeg_image_create();
	if (jpeg_image == NULL) {
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
	if (base->data == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate image data");
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
	wlf_jpeg_image_set_quality(jpeg_image, quality);

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

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("wlframe JPEG Image Test Program\n\n");
	printf("Options:\n");
	printf("  -i, --input <path>     Input JPEG file path to load and test\n");
	printf("  -o, --output <path>    Output path for saved images (default: current directory)\n");
	printf("  -q, --quality <value>  JPEG quality for output (1-100, default: 85)\n");
	printf("  -v, --verbose          Enable verbose logging\n");
	printf("  -h, --help             Show this help message\n\n");
	printf("Examples:\n");
	printf("  %s                           # Create test images in current directory\n", program_name);
	printf("  %s -i photo.jpg              # Load and test photo.jpg\n", program_name);
	printf("  %s -i photo.jpg -o output/   # Load photo.jpg and save to output directory\n", program_name);
	printf("  %s -v -q 95                  # Create test images with high quality and verbose output\n", program_name);
}

int main(int argc, char *argv[]) {
	// Command line options
	char *input_path = NULL;
	char *output_path = NULL;
	int quality = 85;
	bool verbose = false;
	bool show_help = false;

	// Define command line options
	struct wlf_cmd_option options[] = {
		{WLF_OPTION_STRING, "input", 'i', &input_path},
		{WLF_OPTION_STRING, "output", 'o', &output_path},
		{WLF_OPTION_INTEGER, "quality", 'q', &quality},
		{WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose},
		{WLF_OPTION_BOOLEAN, "help", 'h', &show_help}
	};

	// Parse command line arguments
	int remaining_args = wlf_cmd_parse_options(options, 5, &argc, argv);
	if (remaining_args < 0) {
		fprintf(stderr, "Error parsing command line options\n");
		return EXIT_FAILURE;
	}

	// Show help if requested
	if (show_help) {
		print_usage(argv[0]);
		return EXIT_SUCCESS;
	}

	// Validate quality range
	if (quality < 1 || quality > 100) {
		fprintf(stderr, "Error: Quality must be between 1 and 100\n");
		return EXIT_FAILURE;
	}

	// Initialize logging
	int32_t log_level = verbose ? WLF_DEBUG : WLF_INFO;
	wlf_log_init(log_level, NULL);

	printf("wlframe JPEG Image Test\n");
	printf("=======================\n");
	if (verbose) {
		printf("Verbose mode enabled\n");
		printf("Quality setting: %d\n", quality);
		if (input_path) printf("Input path: %s\n", input_path);
		if (output_path) printf("Output path: %s\n", output_path);
		printf("\n");
	}

	if (input_path) {
		// Test loading provided JPEG file
		printf("\nTesting JPEG load: %s\n", input_path);

		struct wlf_image *img = wlf_image_load(input_path);
		if (img == NULL) {
			wlf_log(WLF_ERROR, "Failed to load image: %s", input_path);
			free(input_path);
			free(output_path);
			return EXIT_FAILURE;
		}

		print_image_info(img, input_path);

		// Prepare output filename
		char output_filename[PATH_MAX];
		if (output_path) {
			// Extract filename from input path
			const char *filename = strrchr(input_path, '/');
			if (filename) {
				filename++; // Skip the '/'
			} else {
				filename = input_path;
			}

			// Create output path with original filename
			snprintf(output_filename, sizeof(output_filename), "%s/%s", output_path, filename);

			// Also create a processed version
			char processed_filename[PATH_MAX];
			snprintf(processed_filename, sizeof(processed_filename), "%s/processed_%s", output_path, filename);

			if (wlf_image_save(img, processed_filename)) {
				wlf_log(WLF_INFO, "✓ Saved processed image as: %s", processed_filename);
			} else {
				wlf_log(WLF_ERROR, "✗ Failed to save image as: %s", processed_filename);
			}
		} else {
			snprintf(output_filename, sizeof(output_filename), "loaded_%s", input_path);

			if (wlf_image_save(img, output_filename)) {
				wlf_log(WLF_INFO, "✓ Saved loaded image as: %s", output_filename);
			} else {
				wlf_log(WLF_ERROR, "✗ Failed to save image as: %s", output_filename);
			}
		}

		wlf_image_finish(img);
	} else {
		// Create and test a simple JPEG image
		printf("\nCreating test JPEG images...\n");

		// Prepare output filenames based on output path
		char test_filename[PATH_MAX];
		char hq_filename[PATH_MAX];

		if (output_path) {
			snprintf(test_filename, sizeof(test_filename), "%s/test_gradient.jpg", output_path);
			snprintf(hq_filename, sizeof(hq_filename), "%s/test_gradient_hq.jpg", output_path);
		} else {
			strcpy(test_filename, "test_gradient.jpg");
			strcpy(hq_filename, "test_gradient_hq.jpg");
		}

		create_test_jpeg(test_filename, quality);

		// Load it back and verify
		printf("\nTesting JPEG load/save cycle...\n");
		struct wlf_image *img = wlf_image_load(test_filename);
		if (img) {
			print_image_info(img, test_filename);

			// Save it again with different quality (higher quality)
			if (img->image_type == WLF_IMAGE_TYPE_JPEG) {
				struct wlf_jpeg_image *jpeg_img = wlf_jpeg_image_from_image(img);
				if (jpeg_img) {
					int hq_quality = (quality < 95) ? 95 : quality;
					wlf_jpeg_image_set_quality(jpeg_img, hq_quality);
					if (wlf_image_save(img, hq_filename)) {
						wlf_log(WLF_INFO, "✓ Saved high quality version (q=%d): %s", hq_quality, hq_filename);
					}
				}
			}

			wlf_image_finish(img);
		} else {
			wlf_log(WLF_ERROR, "Failed to load back the test image");
		}
	}

	// Cleanup allocated strings
	free(input_path);
	free(output_path);

	printf("\nJPEG test completed!\n");
	return EXIT_SUCCESS;
}
