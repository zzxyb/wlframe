#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_webp_image.h"
#include "wlf/utils/wlf_cmd_parser.h"
#include "wlf/utils/wlf_log.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static struct wlf_webp_image *create_test_image(uint32_t width, uint32_t height,
	const char *output_path, const char *filename) {
	struct wlf_webp_image *webp_image = wlf_webp_image_create();
	if (webp_image == NULL) {
		return NULL;
	}

	webp_image->base.width = width;
	webp_image->base.height = height;
	webp_image->base.format = WLF_COLOR_TYPE_RGBA;
	webp_image->base.bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	webp_image->base.stride = width * 4;
	webp_image->base.image_type = WLF_IMAGE_TYPE_WEBP;
	webp_image->base.has_alpha_channel = true;
	webp_image->base.is_opaque = false;

	size_t data_size = (size_t)width * height * 4;
	webp_image->base.data = malloc(data_size);
	if (webp_image->base.data == NULL) {
		free(webp_image);
		return NULL;
	}

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			uint32_t offset = (y * width + x) * 4;
			unsigned char r = (unsigned char)((x * 255) / width);
			unsigned char g = (unsigned char)((y * 255) / height);
			unsigned char b = (unsigned char)(((x + y) * 255) / (width + height));
			unsigned char a = (unsigned char)((x * 255) / width);

			webp_image->base.data[offset + 0] = r;
			webp_image->base.data[offset + 1] = g;
			webp_image->base.data[offset + 2] = b;
			webp_image->base.data[offset + 3] = a;
		}
	}

	char full_path[PATH_MAX];
	if (output_path) {
		snprintf(full_path, sizeof(full_path), "%s/%s", output_path, filename);
	} else {
		strncpy(full_path, filename, sizeof(full_path) - 1);
		full_path[sizeof(full_path) - 1] = '\0';
	}

	if (wlf_image_save((struct wlf_image *)webp_image, full_path)) {
		wlf_log(WLF_INFO, "✓ WebP test image saved: %s", full_path);
	} else {
		wlf_log(WLF_ERROR, "✗ Failed to save WebP test image: %s", full_path);
	}

	return webp_image;
}

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("wlframe WebP Image Test Program\n\n");
	printf("Options:\n");
	printf("  -i, --input <path>      Input WebP file path to load and test\n");
	printf("  -o, --output <path>     Output dir or output file path (default: current directory)\n");
	printf("  -w, --width <value>     Width for test image (default: 256)\n");
	printf("  -H, --height <value>    Height for test image (default: 256)\n");
	printf("  -v, --verbose           Enable verbose logging\n");
	printf("  -h, --help              Show this help message\n\n");
	printf("Examples:\n");
	printf("  %s                              # Create test images in current directory\n", program_name);
	printf("  %s -i image.webp                # Load and test image.webp\n", program_name);
	printf("  %s -i image.webp -o output/     # Load image.webp and save to output directory\n", program_name);
	printf("  %s -i image.webp -o out.webp    # Load image.webp and save to out.webp\n", program_name);
	printf("  %s -w 512 -H 512 -v             # Create 512x512 test image with verbose output\n", program_name);
}

int main(int argc, char *argv[]) {
	char *input_path = NULL;
	char *output_path = NULL;
	int width = 256;
	int height = 256;
	bool verbose = false;
	bool show_help = false;

	struct wlf_cmd_option options[] = {
		{WLF_OPTION_STRING, "input", 'i', &input_path},
		{WLF_OPTION_STRING, "output", 'o', &output_path},
		{WLF_OPTION_INTEGER, "width", 'w', &width},
		{WLF_OPTION_INTEGER, "height", 'H', &height},
		{WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose},
		{WLF_OPTION_BOOLEAN, "help", 'h', &show_help}
	};

	int remaining_args = wlf_cmd_parse_options(options, 6, &argc, argv);
	if (remaining_args < 0) {
		fprintf(stderr, "Error parsing command line options\n");
		return EXIT_FAILURE;
	}

	if (show_help) {
		print_usage(argv[0]);
		return EXIT_SUCCESS;
	}

	if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
		fprintf(stderr, "Error: Width and height must be between 1 and 4096\n");
		return EXIT_FAILURE;
	}

	int32_t log_level = verbose ? WLF_DEBUG : WLF_INFO;
	wlf_log_init(log_level, NULL);

	printf("wlframe WebP Image Test\n");
	printf("=======================\n");
	if (verbose) {
		printf("Verbose mode enabled\n");
		printf("Test image dimensions: %dx%d\n", width, height);
		if (input_path) printf("Input path: %s\n", input_path);
		if (output_path) printf("Output path: %s\n", output_path);
		printf("\n");
	}

	if (input_path) {
		printf("\nTesting WebP load: %s\n", input_path);
		struct wlf_image *loaded_image = wlf_image_load(input_path);
		if (loaded_image) {
			printf("✓ WebP image loaded successfully\n");
			printf("  - Width: %u\n", loaded_image->width);
			printf("  - Height: %u\n", loaded_image->height);
			printf("  - Format: %s\n",
				loaded_image->format == WLF_COLOR_TYPE_RGBA ? "RGBA" : "RGB");
			printf("  - Type: %s\n", wlf_image_get_type_string(loaded_image));
			printf("  - Bit depth: %d\n", loaded_image->bit_depth);
			printf("  - Has alpha: %s\n", loaded_image->has_alpha_channel ? "Yes" : "No");

			if (wlf_image_is_webp(loaded_image)) {
				printf("✓ Image correctly identified as WebP\n");
			}

			char output_filename[PATH_MAX];
			if (output_path) {
				const char *dot = strrchr(output_path, '.');
				if (dot && strcmp(dot, ".webp") == 0) {
					snprintf(output_filename, sizeof(output_filename), "%s", output_path);
				} else {
				const char *filename = strrchr(input_path, '/');
				filename = filename != NULL ? filename + 1 : input_path;
				snprintf(output_filename, sizeof(output_filename), "%s/processed_%s", output_path, filename);
				}
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
			wlf_log(WLF_ERROR, "✗ Failed to load WebP image: %s", input_path);
		}
	} else {
		printf("\nTest 1: Creating a test WebP image...\n");
		struct wlf_webp_image *test_image = create_test_image(
			(uint32_t)width, (uint32_t)height, output_path, "test_gradient.webp");
		if (test_image == NULL) {
			wlf_log(WLF_ERROR, "Failed to create test WebP image");
			return EXIT_FAILURE;
		}

		wlf_image_finish((struct wlf_image *)test_image);
		free(test_image);
	}

	free(input_path);
	free(output_path);
	return EXIT_SUCCESS;
}
