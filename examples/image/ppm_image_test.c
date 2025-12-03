#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_ppm_image.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_cmd_parser.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * @brief Create a simple test PPM image with a gradient pattern.
 * @param width Image width.
 * @param height Image height.
 * @param output_path Optional output directory path.
 * @param filename Output filename.
 * @return Pointer to the created PPM image, or NULL on failure.
 */
static struct wlf_ppm_image *create_test_image(uint32_t width, uint32_t height, const char *output_path, const char *filename) {
	struct wlf_ppm_image *ppm_image = wlf_ppm_image_create();
	if (ppm_image == NULL) {
		wlf_log(WLF_ERROR, "Failed to create wlf_ppm_image");
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
	if (ppm_image->base.data == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate image data");
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

	// Save the image
	char full_path[PATH_MAX];
	if (output_path) {
		snprintf(full_path, sizeof(full_path), "%s/%s", output_path, filename);
	} else {
		strcpy(full_path, filename);
	}

	if (wlf_image_save((struct wlf_image *)ppm_image, full_path)) {
		wlf_log(WLF_INFO, "✓ PPM test image saved: %s", full_path);
	} else {
		wlf_log(WLF_ERROR, "✗ Failed to save PPM test image: %s", full_path);
	}

	return ppm_image;
}

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("wlframe PPM Image Test Program\n\n");
	printf("Options:\n");
	printf("  -i, --input <path>      Input PPM file path to load and test\n");
	printf("  -o, --output <path>     Output path for saved images (default: current directory)\n");
	printf("  -w, --width <value>     Width for test image (default: 256)\n");
	printf("  -H, --height <value>    Height for test image (default: 256)\n");
	printf("  -a, --ascii             Use ASCII format (P3) instead of binary (P6)\n");
	printf("  -v, --verbose           Enable verbose logging\n");
	printf("  -h, --help              Show this help message\n\n");
	printf("Examples:\n");
	printf("  %s                              # Create test images in current directory\n", program_name);
	printf("  %s -i image.ppm                 # Load and test image.ppm\n", program_name);
	printf("  %s -i image.ppm -o output/      # Load image.ppm and save to output directory\n", program_name);
	printf("  %s -w 512 -H 512 -a -v          # Create 512x512 ASCII PPM with verbose output\n", program_name);
}

int main(int argc, char *argv[]) {
	// Command line options
	char *input_path = NULL;
	char *output_path = NULL;
	int width = 256;
	int height = 256;
	bool ascii_format = false;
	bool verbose = false;
	bool show_help = false;

	// Define command line options
	struct wlf_cmd_option options[] = {
		{WLF_OPTION_STRING, "input", 'i', &input_path},
		{WLF_OPTION_STRING, "output", 'o', &output_path},
		{WLF_OPTION_INTEGER, "width", 'w', &width},
		{WLF_OPTION_INTEGER, "height", 'H', &height},
		{WLF_OPTION_BOOLEAN, "ascii", 'a', &ascii_format},
		{WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose},
		{WLF_OPTION_BOOLEAN, "help", 'h', &show_help}
	};

	// Parse command line arguments
	int remaining_args = wlf_cmd_parse_options(options, 7, &argc, argv);
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
	if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
		fprintf(stderr, "Error: Width and height must be between 1 and 4096\n");
		return EXIT_FAILURE;
	}

	// Initialize logging
	int32_t log_level = verbose ? WLF_DEBUG : WLF_INFO;
	wlf_log_init(log_level, NULL);

	printf("wlframe PPM Image Test\n");
	printf("======================\n");
	if (verbose) {
		printf("Verbose mode enabled\n");
		printf("Test image dimensions: %dx%d\n", width, height);
		printf("Format: %s\n", ascii_format ? "ASCII (P3)" : "Binary (P6)");
		if (input_path) printf("Input path: %s\n", input_path);
		if (output_path) printf("Output path: %s\n", output_path);
		printf("\n");
	}

	if (input_path) {
		// Test loading provided PPM file
		printf("\nTesting PPM load: %s\n", input_path);
		struct wlf_image *loaded_image = wlf_image_load(input_path);
		if (loaded_image) {
			wlf_log(WLF_INFO, "✓ PPM image loaded successfully: %s", input_path);
			printf("  - Width: %u\n", loaded_image->width);
			printf("  - Height: %u\n", loaded_image->height);
			printf("  - Format: %d\n", loaded_image->format);
			printf("  - Type: %s\n", wlf_image_get_type_string(loaded_image));
			printf("  - Bit depth: %d\n", loaded_image->bit_depth);

			if (wlf_image_is_ppm(loaded_image)) {
				printf("✓ Image correctly identified as PPM\n");
				struct wlf_ppm_image *ppm_img = wlf_ppm_image_from_image(loaded_image);
				printf("  - Format: %s\n",
					ppm_img->format == WLF_PPM_FORMAT_P3 ? "ASCII (P3)" : "Binary (P6)");
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
			wlf_log(WLF_ERROR, "✗ Failed to load PPM image: %s", input_path);
		}
	} else {
		// Create and test PPM images
		printf("\nTest 1: Creating a test PPM image...\n");

		char test_filename[PATH_MAX];
		if (ascii_format) {
			if (output_path) {
				snprintf(test_filename, sizeof(test_filename), "%s/test_ascii.ppm", output_path);
			} else {
				strcpy(test_filename, "test_ascii.ppm");
			}
		} else {
			if (output_path) {
				snprintf(test_filename, sizeof(test_filename), "%s/test_binary.ppm", output_path);
			} else {
				strcpy(test_filename, "test_binary.ppm");
			}
		}

		struct wlf_ppm_image *test_image = create_test_image(width, height, output_path,
			ascii_format ? "test_ascii.ppm" : "test_binary.ppm");
		if (test_image == NULL) {
			printf("Failed to create test image!\n");
			free(input_path);
			free(output_path);
			return 1;
		}
		printf("✓ Test image created successfully\n");

		// Set the format based on user preference
		printf("\nTest 2: Saving image in %s format...\n",
			ascii_format ? "ASCII (P3)" : "binary (P6)");
		wlf_ppm_image_set_format(test_image,
			ascii_format ? WLF_PPM_FORMAT_P3 : WLF_PPM_FORMAT_P6);

		// Test 3: Load the saved PPM image back
		printf("\nTest 3: Loading saved PPM image back...\n");
		struct wlf_image *loaded_image = wlf_image_load(test_filename);
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

			wlf_image_finish(loaded_image);
			free(loaded_image);
		} else {
			printf("✗ Failed to load PPM image\n");
		}

		// Test 4: Create and save in the other format for comparison
		printf("\nTest 4: Creating image in %s format for comparison...\n",
			ascii_format ? "binary (P6)" : "ASCII (P3)");

		char alt_filename[PATH_MAX];
		if (output_path) {
			snprintf(alt_filename, sizeof(alt_filename), "%s/%s", output_path,
				ascii_format ? "test_binary.ppm" : "test_ascii.ppm");
		} else {
			strcpy(alt_filename, ascii_format ? "test_binary.ppm" : "test_ascii.ppm");
		}

		struct wlf_ppm_image *alt_image = create_test_image(width, height, output_path,
			ascii_format ? "test_binary.ppm" : "test_ascii.ppm");
		if (alt_image) {
			wlf_ppm_image_set_format(alt_image,
				ascii_format ? WLF_PPM_FORMAT_P6 : WLF_PPM_FORMAT_P3);

			printf("✓ Alternative format image created successfully\n");
			wlf_image_finish((struct wlf_image *)alt_image);
			free(alt_image);
		}

		wlf_image_finish((struct wlf_image *)test_image);
		free(test_image);
	}

	// Cleanup allocated strings
	free(input_path);
	free(output_path);

	printf("\nPPM image test completed!\n");
	return 0;
}
