#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_xbm_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_cmd_parser.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * @brief Create a simple test XBM image with a cross pattern.
 * @param width Image width.
 * @param height Image height.
 * @param output_path Optional output directory path.
 * @param filename Output filename.
 * @return Pointer to the created XBM image, or NULL on failure.
 */
static struct wlf_xbm_image *create_test_image(uint32_t width, uint32_t height, const char *output_path, const char *filename) {
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

	// Save the image
	char full_path[PATH_MAX];
	if (output_path) {
		snprintf(full_path, sizeof(full_path), "%s/%s", output_path, filename);
	} else {
		strcpy(full_path, filename);
	}

	if (wlf_image_save((struct wlf_image *)xbm_image, full_path)) {
		wlf_log(WLF_INFO, "✓ XBM test image saved: %s", full_path);
	} else {
		wlf_log(WLF_ERROR, "✗ Failed to save XBM test image: %s", full_path);
	}

	return xbm_image;
}

/**
 * @brief Create a cursor-like XBM image with hotspot.
 * @param size Size of the square cursor.
 * @param output_path Optional output directory path.
 * @param filename Output filename.
 * @return Pointer to the created XBM image, or NULL on failure.
 */
static struct wlf_xbm_image *create_cursor_image(uint32_t size, const char *output_path, const char *filename) {
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

	// Save the image
	char full_path[PATH_MAX];
	if (output_path) {
		snprintf(full_path, sizeof(full_path), "%s/%s", output_path, filename);
	} else {
		strcpy(full_path, filename);
	}

	if (wlf_image_save((struct wlf_image *)xbm_image, full_path)) {
		wlf_log(WLF_INFO, "✓ XBM cursor image saved: %s", full_path);
	} else {
		wlf_log(WLF_ERROR, "✗ Failed to save XBM cursor image: %s", full_path);
	}

	return xbm_image;
}

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("wlframe XBM Image Test Program\n\n");
	printf("Options:\n");
	printf("  -i, --input <path>      Input XBM file path to load and test\n");
	printf("  -o, --output <path>     Output path for saved images (default: current directory)\n");
	printf("  -w, --width <value>     Width for test image (default: 32)\n");
	printf("  -H, --height <value>    Height for test image (default: 32)\n");
	printf("  -c, --cursor            Create cursor pattern instead of cross\n");
	printf("  -v, --verbose           Enable verbose logging\n");
	printf("  -h, --help              Show this help message\n\n");
	printf("Examples:\n");
	printf("  %s                              # Create test cross pattern in current directory\n", program_name);
	printf("  %s -i icon.xbm                  # Load and test icon.xbm\n", program_name);
	printf("  %s -i icon.xbm -o output/       # Load icon.xbm and save to output directory\n", program_name);
	printf("  %s -w 64 -H 64 -c -v            # Create 64x64 cursor with verbose output\n", program_name);
}

int main(int argc, char *argv[]) {
	// Command line options
	char *input_path = NULL;
	char *output_path = NULL;
	int width = 32;
	int height = 32;
	bool cursor_pattern = false;
	bool verbose = false;
	bool show_help = false;

	// Define command line options
	struct wlf_cmd_option options[] = {
		{WLF_OPTION_STRING, "input", 'i', &input_path},
		{WLF_OPTION_STRING, "output", 'o', &output_path},
		{WLF_OPTION_INTEGER, "width", 'w', &width},
		{WLF_OPTION_INTEGER, "height", 'H', &height},
		{WLF_OPTION_BOOLEAN, "cursor", 'c', &cursor_pattern},
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
	if (width <= 0 || height <= 0 || width > 256 || height > 256) {
		fprintf(stderr, "Error: Width and height must be between 1 and 256\n");
		return EXIT_FAILURE;
	}

	// Initialize logging
	int32_t log_level = verbose ? WLF_DEBUG : WLF_INFO;
	wlf_log_init(log_level, NULL);

	printf("wlframe XBM Image Test\n");
	printf("======================\n");
	if (verbose) {
		printf("Verbose mode enabled\n");
		printf("Test image dimensions: %dx%d\n", width, height);
		printf("Pattern: %s\n", cursor_pattern ? "Cursor" : "Cross");
		if (input_path) printf("Input path: %s\n", input_path);
		if (output_path) printf("Output path: %s\n", output_path);
		printf("\n");
	}

	if (input_path) {
		// Test loading provided XBM file
		printf("\nTesting XBM load: %s\n", input_path);
		struct wlf_image *loaded_image = wlf_image_load(input_path);
		if (loaded_image) {
			wlf_log(WLF_INFO, "✓ XBM image loaded successfully: %s", input_path);
			printf("  - Width: %u\n", loaded_image->width);
			printf("  - Height: %u\n", loaded_image->height);
			printf("  - Format: %d\n", loaded_image->format);
			printf("  - Type: %s\n", wlf_image_get_type_string(loaded_image));

			if (wlf_image_is_xbm(loaded_image)) {
				printf("✓ Image correctly identified as XBM\n");
				struct wlf_xbm_image *xbm_img = wlf_xbm_image_from_image(loaded_image);
				printf("  - Name: %s\n", xbm_img->name ? xbm_img->name : "No name");
				printf("  - Hotspot: (%d, %d)\n", xbm_img->hotspot.x, xbm_img->hotspot.y);
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
			wlf_log(WLF_ERROR, "✗ Failed to load XBM image: %s", input_path);
		}
	} else {
		// Create and test XBM images
		printf("\nTest 1: Creating XBM image...\n");
		struct wlf_xbm_image *test_image;

		if (cursor_pattern) {
			test_image = create_cursor_image(width, output_path, "cursor.xbm");
		} else {
			test_image = create_test_image(width, height, output_path, "test_pattern.xbm");
		}

		if (!test_image) {
			printf("Failed to create test image!\n");
			free(input_path);
			free(output_path);
			return 1;
		}

		wlf_xbm_image_set_name(test_image, cursor_pattern ? "cursor" : "test_pattern");
		printf("✓ Test image created successfully\n");

		wlf_image_finish((struct wlf_image *)test_image);
		free(test_image);
	}

	// Cleanup allocated strings
	free(input_path);
	free(output_path);

	printf("\nXBM image test completed!\n");
	return 0;
}
