#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_png_image.h"
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

static struct wlf_image *create_chess_board(uint32_t width, uint32_t height, uint32_t square_size) {
	struct wlf_png_image *png_image = wlf_png_image_create();
	if (!png_image) {
		wlf_log(WLF_ERROR, "Failed to create PNG image");
		return NULL;
	}

	struct wlf_image *base = &png_image->base;
	base->width = width;
	base->height = height;
	base->format = WLF_COLOR_TYPE_RGB;
	base->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	base->stride = width * 3;
	base->image_type = WLF_IMAGE_TYPE_PNG;
	base->has_alpha_channel = false;
	base->is_opaque = true;

	base->data = malloc(height * base->stride);
	if (!base->data) {
		wlf_log(WLF_ERROR, "Failed to allocate image data");
		free(png_image);
		return NULL;
	}

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			unsigned char *pixel = base->data + y * base->stride + x * 3;
			uint32_t square_x = x / square_size;
			uint32_t square_y = y / square_size;

			if ((square_x + square_y) % 2 == 0) {
				// White square
				pixel[0] = 255; // Red
				pixel[1] = 255; // Green
				pixel[2] = 255; // Blue
			} else {
				// Black square
				pixel[0] = 0;   // Red
				pixel[1] = 0;   // Green
				pixel[2] = 0;   // Blue
			}
		}
	}

	return base;
}

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("wlframe PNG Image Test Program\n\n");
	printf("Options:\n");
	printf("  -i, --input <path>      Input PNG file path to load and test\n");
	printf("  -o, --output <path>     Output path for saved images (default: current directory)\n");
	printf("  -w, --width <value>     Width for test image (default: 400)\n");
	printf("  -H, --height <value>    Height for test image (default: 400)\n");
	printf("  -s, --square <value>    Square size for chess pattern (default: 50)\n");
	printf("  -v, --verbose           Enable verbose logging\n");
	printf("  -h, --help              Show this help message\n\n");
	printf("Examples:\n");
	printf("  %s                              # Create chess board demo in current directory\n", program_name);
	printf("  %s -i image.png                 # Load and test image.png\n", program_name);
	printf("  %s -i image.png -o output/      # Load image.png and save to output directory\n", program_name);
	printf("  %s -w 800 -H 600 -s 25 -v       # Create 800x600 chess board with 25px squares\n", program_name);
}

int main(int argc, char *argv[]) {
	// Command line options
	char *input_path = NULL;
	char *output_path = NULL;
	int width = 400;
	int height = 400;
	int square_size = 50;
	bool verbose = false;
	bool show_help = false;

	// Define command line options
	struct wlf_cmd_option options[] = {
		{WLF_OPTION_STRING, "input", 'i', &input_path},
		{WLF_OPTION_STRING, "output", 'o', &output_path},
		{WLF_OPTION_INTEGER, "width", 'w', &width},
		{WLF_OPTION_INTEGER, "height", 'H', &height},
		{WLF_OPTION_INTEGER, "square", 's', &square_size},
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

	// Validate parameters
	if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
		fprintf(stderr, "Error: Width and height must be between 1 and 4096\n");
		return EXIT_FAILURE;
	}
	if (square_size <= 0 || square_size > width || square_size > height) {
		fprintf(stderr, "Error: Square size must be positive and smaller than image dimensions\n");
		return EXIT_FAILURE;
	}

	// Initialize logging
	int32_t log_level = verbose ? WLF_DEBUG : WLF_INFO;
	wlf_log_init(log_level, NULL);

	printf("wlframe PNG Image Test\n");
	printf("======================\n");
	if (verbose) {
		printf("Verbose mode enabled\n");
		printf("Test image dimensions: %dx%d\n", width, height);
		printf("Chess square size: %d\n", square_size);
		if (input_path) printf("Input path: %s\n", input_path);
		if (output_path) printf("Output path: %s\n", output_path);
		printf("\n");
	}

	if (input_path) {
		// Test loading provided PNG file
		printf("\nTesting PNG load: %s\n", input_path);
		struct wlf_image *img = wlf_image_load(input_path);
		if (img == NULL) {
			wlf_log(WLF_ERROR, "Failed to load image: %s", input_path);
			free(input_path);
			free(output_path);
			return EXIT_FAILURE;
		} else {
			wlf_log(WLF_INFO, "✓ Image loaded successfully: %s", input_path);
			wlf_log(WLF_INFO, "Image type: %s", wlf_image_get_type_string(img));
			wlf_log(WLF_INFO, "Image size: %ux%u", img->width, img->height);
			wlf_log(WLF_INFO, "Image format: %d", img->format);
			wlf_log(WLF_INFO, "Image bit depth: %d", img->bit_depth);
			wlf_log(WLF_INFO, "Image stride: %d", img->stride);
			wlf_log(WLF_INFO, "Image has alpha channel: %s", img->has_alpha_channel ? "[✓]" : "[✗]");
			wlf_log(WLF_INFO, "Image is opaque: %s", img->is_opaque ? "[✓]" : "[✗]");
			wlf_log(WLF_INFO, "Image channels: %d", wlf_image_get_channels(img));
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

		if (wlf_image_save(img, output_filename)) {
			wlf_log(WLF_INFO, "✓ Image saved successfully to: %s", output_filename);
		} else {
			wlf_log(WLF_ERROR, "✗ Failed to save image to: %s", output_filename);
		}

		wlf_image_finish(img);
	} else {
		// Create demo images
		wlf_log(WLF_INFO, "No input file specified. Creating demo images...");
		wlf_log(WLF_INFO, "Creating chess board pattern...");

		struct wlf_image *chess_img = create_chess_board(width, height, square_size);
		if (chess_img) {
			char chess_path[PATH_MAX];
			if (output_path) {
				snprintf(chess_path, sizeof(chess_path), "%s/chess_board.png", output_path);
			} else {
				strcpy(chess_path, "chess_board.png");
			}

			if (wlf_image_save(chess_img, chess_path)) {
				wlf_log(WLF_INFO, "✓ Chess board saved to: %s", chess_path);
			} else {
				wlf_log(WLF_ERROR, "✗ Failed to save chess board to: %s", chess_path);
			}
			wlf_image_finish(chess_img);
		} else {
			wlf_log(WLF_ERROR, "Failed to create chess board image");
		}
	}

	// Cleanup allocated strings
	free(input_path);
	free(output_path);

	wlf_log(WLF_INFO, "PNG image test completed!");
	return EXIT_SUCCESS;
}
