#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_xpm_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_cmd_parser.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
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

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("wlframe XPM Image Test Program\n\n");
	printf("Options:\n");
	printf("  -i, --input <path>      Input XPM file path to load and test\n");
	printf("  -o, --output <path>     Output path for saved images (default: current directory)\n");
	printf("  -w, --width <value>     Width for test image (default: 64)\n");
	printf("  -H, --height <value>    Height for test image (default: 64)\n");
	printf("  -v, --verbose           Enable verbose logging\n");
	printf("  -h, --help              Show this help message\n\n");
	printf("Examples:\n");
	printf("  %s                              # Create test cross icon in current directory\n", program_name);
	printf("  %s -i icon.xpm                  # Load and test icon.xpm\n", program_name);
	printf("  %s -i icon.xpm -o output/       # Load icon.xpm and save to output directory\n", program_name);
	printf("  %s -w 128 -H 128 -v             # Create 128x128 test icon with verbose output\n", program_name);
}

int main(int argc, char *argv[]) {
	// Command line options
	char *input_path = NULL;
	char *output_path = NULL;
	int width = 64;
	int height = 64;
	bool verbose = false;
	bool show_help = false;

	// Define command line options
	struct wlf_cmd_option options[] = {
		{WLF_OPTION_STRING, "input", 'i', &input_path},
		{WLF_OPTION_STRING, "output", 'o', &output_path},
		{WLF_OPTION_INTEGER, "width", 'w', &width},
		{WLF_OPTION_INTEGER, "height", 'H', &height},
		{WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose},
		{WLF_OPTION_BOOLEAN, "help", 'h', &show_help}
	};

	// Parse command line arguments
	int remaining_args = wlf_cmd_parse_options(options, 6, &argc, argv);
	if (remaining_args < 0) {
		fprintf(stderr, "Error parsing command line options\n");
		return EXIT_FAILURE;
	}

	// Show help if requested
	if (show_help) {
		print_usage(argv[0]);
		return EXIT_SUCCESS;
	}

	// Validate dimensions
	if (width <= 0 || height <= 0 || width > 512 || height > 512) {
		fprintf(stderr, "Error: Width and height must be between 1 and 512\n");
		return EXIT_FAILURE;
	}

	// Initialize logging
	int32_t log_level = verbose ? WLF_DEBUG : WLF_INFO;
	wlf_log_init(log_level, NULL);

	printf("wlframe XPM Image Test\n");
	printf("======================\n");
	if (verbose) {
		printf("Verbose mode enabled\n");
		printf("Test image dimensions: %dx%d\n", width, height);
		if (input_path) printf("Input path: %s\n", input_path);
		if (output_path) printf("Output path: %s\n", output_path);
		printf("\n");
	}

	wlf_log(WLF_INFO, "Starting XPM image test...");

	if (input_path) {
		// Test loading provided XPM file
		printf("\nTesting XPM load: %s\n", input_path);
		struct wlf_image *loaded_image = wlf_image_load(input_path);
		if (loaded_image) {
			wlf_log(WLF_INFO, "✓ XPM image loaded successfully: %s", input_path);
			printf("  - Width: %u\n", loaded_image->width);
			printf("  - Height: %u\n", loaded_image->height);
			printf("  - Format: %d\n", loaded_image->format);
			printf("  - Type: %s\n", wlf_image_get_type_string(loaded_image));
			printf("  - Bit depth: %d\n", loaded_image->bit_depth);

			if (wlf_image_is_xpm(loaded_image)) {
				printf("✓ Image correctly identified as XPM\n");
				struct wlf_xpm_image *xpm_img = wlf_xpm_image_from_image(loaded_image);
				printf("  - Colors per pixel: %u\n", xpm_img->colors_per_pixel);
				printf("  - Name: %s\n", xpm_img->name ? xpm_img->name : "No name");
			}

			// Save processed version
			char output_filename[PATH_MAX];
			if (output_path) {
				const char *filename = strrchr(input_path, '/');
				filename = filename ? filename + 1 : input_path;
				snprintf(output_filename, sizeof(output_filename), "%s/processed_%s", output_path, filename);
			} else {
				snprintf(output_filename, sizeof(output_filename), "processed_%s", input_path);
			}

			if (wlf_image_save(loaded_image, output_filename)) {
				wlf_log(WLF_INFO, "✓ Saved processed image: %s", output_filename);
			} else {
				wlf_log(WLF_ERROR, "✗ Failed to save processed image: %s", output_filename);
			}

			wlf_image_finish(loaded_image);
			free(loaded_image);
		} else {
			wlf_log(WLF_ERROR, "✗ Failed to load XPM image: %s", input_path);
		}
	} else {
		// Create and test XPM images
		printf("\nTest 1: Creating XPM image...\n");
		struct wlf_image *image = create_simple_icon(width, height);
		if (!image) {
			wlf_log(WLF_ERROR, "Failed to create XPM image");
			free(input_path);
			free(output_path);
			return 1;
		}

		// Test 2: Verify image properties
		printf("\nTest 2: Verifying image properties...\n");
		if (!test_image_properties(image, width, height)) {
			wlf_log(WLF_ERROR, "Image properties test failed");
			wlf_image_finish(image);
			free(input_path);
			free(output_path);
			return 1;
		}

		// Test 3: Save XPM image
		char test_filename[PATH_MAX];
		if (output_path) {
			snprintf(test_filename, sizeof(test_filename), "%s/test_icon.xpm", output_path);
		} else {
			strcpy(test_filename, "test_icon.xpm");
		}

		printf("\nTest 3: Saving XPM image to %s...\n", test_filename);
		if (!wlf_image_save(image, test_filename)) {
			wlf_log(WLF_ERROR, "Failed to save XPM image");
			wlf_image_finish(image);
			free(input_path);
			free(output_path);
			return 1;
		}

		wlf_image_finish(image);

		// Test 4: Load XPM image
		printf("\nTest 4: Loading XPM image from %s...\n", test_filename);
		struct wlf_image *loaded_image = wlf_image_load(test_filename);
		if (!loaded_image) {
			wlf_log(WLF_ERROR, "Failed to load XPM image");
			free(input_path);
			free(output_path);
			return 1;
		}

		// Test 5: Verify loaded image properties
		printf("\nTest 5: Verifying loaded image properties...\n");
		if (!test_image_properties(loaded_image, width, height)) {
			wlf_log(WLF_ERROR, "Loaded image properties test failed");
			wlf_image_finish(loaded_image);
			free(input_path);
			free(output_path);
			return 1;
		}

		wlf_image_finish(loaded_image);
		free(loaded_image);
	}

	// Cleanup allocated strings
	free(input_path);
	free(output_path);

	wlf_log(WLF_INFO, "All XPM image tests passed!");
	return 0;
}
