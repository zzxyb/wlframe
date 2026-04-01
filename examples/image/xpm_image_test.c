#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_xpm_image.h"
#include "wlf/utils/wlf_cmd_parser.h"
#include "wlf/utils/wlf_log.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static const char *get_filename_from_path(const char *path) {
	if (path == NULL) {
		return NULL;
	}

	const char *filename = strrchr(path, '/');
	return filename != NULL ? filename + 1 : path;
}

static bool is_xpm_file_path(const char *path) {
	if (path == NULL) {
		return false;
	}

	const char *dot = strrchr(path, '.');
	return dot != NULL && strcmp(dot, ".xpm") == 0;
}

static struct wlf_xpm_image *create_test_image(uint32_t width, uint32_t height,
	const char *output_path, const char *filename) {
	struct wlf_xpm_image *xpm_image = wlf_xpm_image_create();
	if (xpm_image == NULL) {
		return NULL;
	}

	xpm_image->base.width = width;
	xpm_image->base.height = height;
	xpm_image->base.format = WLF_COLOR_TYPE_RGBA;
	xpm_image->base.bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	xpm_image->base.stride = width * 4;
	xpm_image->base.image_type = WLF_IMAGE_TYPE_XPM;
	xpm_image->base.has_alpha_channel = true;
	xpm_image->base.is_opaque = false;

	size_t data_size = (size_t)width * height * 4;
	xpm_image->base.data = malloc(data_size);
	if (xpm_image->base.data == NULL) {
		free(xpm_image);
		return NULL;
	}

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			uint32_t offset = (y * width + x) * 4;
			unsigned char r = (unsigned char)((x * 255) / width);
			unsigned char g = (unsigned char)((y * 255) / height);
			unsigned char b = (unsigned char)(((x + y) * 255) / (width + height));
			unsigned char a = ((x / 32 + y / 32) % 2 == 0) ? 255 : 0;

			xpm_image->base.data[offset + 0] = r;
			xpm_image->base.data[offset + 1] = g;
			xpm_image->base.data[offset + 2] = b;
			xpm_image->base.data[offset + 3] = a;
		}
	}

	char full_path[PATH_MAX];
	if (output_path) {
		snprintf(full_path, sizeof(full_path), "%s/%s", output_path, filename);
	} else {
		strncpy(full_path, filename, sizeof(full_path) - 1);
		full_path[sizeof(full_path) - 1] = '\0';
	}

	if (xpm_image->base.impl->save(&xpm_image->base, full_path)) {
		wlf_log(WLF_INFO, "✓ XPM test image saved: %s", full_path);
	} else {
		wlf_log(WLF_ERROR, "✗ Failed to save XPM test image: %s", full_path);
	}

	return xpm_image;
}

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("wlframe XPM Image Test Program\n\n");
	printf("Options:\n");
	printf("  -i, --input <path>      Input XPM file path to load and test\n");
	printf("  -o, --output <path>     Output directory path (default: current directory)\n");
	printf("  -w, --width <value>     Width for test image (default: 256)\n");
	printf("  -H, --height <value>    Height for test image (default: 256)\n");
	printf("  -v, --verbose           Enable verbose logging\n");
	printf("  -h, --help              Show this help message\n\n");
	printf("Examples:\n");
	printf("  %s                               # Create test images in current directory\n", program_name);
	printf("  %s -i image.xpm                  # Load and test image.xpm\n", program_name);
	printf("  %s -i image.xpm -o output/       # Load image.xpm and save to output directory\n", program_name);
	printf("  %s -w 512 -H 512 -v              # Create 512x512 test image with verbose output\n", program_name);
}

static void print_image_info(const struct wlf_image *image) {
	printf("  - Width: %u\n", image->width);
	printf("  - Height: %u\n", image->height);
	printf("  - Format: %s\n", image->format == WLF_COLOR_TYPE_RGBA ? "RGBA" : "RGB");
	printf("  - Type: %s\n", wlf_image_get_type_string(image));
	printf("  - Bit depth: %d\n", image->bit_depth);
	printf("  - Has alpha: %s\n", image->has_alpha_channel ? "Yes" : "No");
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

	if (output_path == NULL && argc > 1) {
		output_path = strdup(argv[1]);
		if (output_path == NULL) {
			fprintf(stderr, "Error: out of memory\n");
			free(input_path);
			return EXIT_FAILURE;
		}
	}

	if (argc > 2) {
		fprintf(stderr, "Error: too many positional arguments\n");
		free(input_path);
		free(output_path);
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

	printf("wlframe XPM Image Test\n");
	printf("======================\n");
	if (verbose) {
		printf("Verbose mode enabled\n");
		printf("Test image dimensions: %dx%d\n", width, height);
		if (input_path) printf("Input path: %s\n", input_path);
		if (output_path) printf("Output path: %s\n", output_path);
		printf("\n");
	}

	if (input_path) {
		printf("\nTesting XPM load: %s\n", input_path);

		struct wlf_xpm_image *xpm_image = wlf_xpm_image_create();
		if (xpm_image == NULL) {
			free(input_path);
			free(output_path);
			return EXIT_FAILURE;
		}

		if (!xpm_image->base.impl->load(&xpm_image->base, input_path, false)) {
			wlf_log(WLF_ERROR, "✗ Failed to load XPM image: %s", input_path);
			wlf_image_finish(&xpm_image->base);
			free(xpm_image);
			free(input_path);
			free(output_path);
			return EXIT_FAILURE;
		}

		xpm_image->base.image_type = WLF_IMAGE_TYPE_XPM;
		print_image_info(&xpm_image->base);
		if (wlf_image_is_xpm(&xpm_image->base)) {
			printf("✓ Image correctly identified as XPM\n");
		}

		char output_filename[PATH_MAX];
		const char *filename = get_filename_from_path(input_path);
		if (output_path) {
			if (is_xpm_file_path(output_path)) {
				snprintf(output_filename, sizeof(output_filename), "%s", output_path);
			} else {
				snprintf(output_filename, sizeof(output_filename), "%s/processed_%s", output_path, filename);
			}
		} else {
			snprintf(output_filename, sizeof(output_filename), "processed_%s", filename);
		}

		if (xpm_image->base.impl->save(&xpm_image->base, output_filename)) {
			wlf_log(WLF_INFO, "✓ Saved processed image: %s", output_filename);
		} else {
			wlf_log(WLF_ERROR, "✗ Failed to save processed image: %s", output_filename);
		}

		wlf_image_finish(&xpm_image->base);
		free(xpm_image);
	} else {
		printf("\nTest 1: Creating a test XPM image...\n");
		struct wlf_xpm_image *test_image = create_test_image(
			(uint32_t)width, (uint32_t)height, output_path, "test_pattern.xpm");
		if (test_image == NULL) {
			wlf_log(WLF_ERROR, "Failed to create test XPM image");
			free(input_path);
			free(output_path);
			return EXIT_FAILURE;
		}

		wlf_image_finish(&test_image->base);
		free(test_image);

		char test_filename[PATH_MAX];
		if (output_path) {
			snprintf(test_filename, sizeof(test_filename), "%s/test_pattern.xpm", output_path);
		} else {
			strncpy(test_filename, "test_pattern.xpm", sizeof(test_filename) - 1);
			test_filename[sizeof(test_filename) - 1] = '\0';
		}

		printf("\nTest 2: Loading saved XPM image back...\n");
		struct wlf_xpm_image *loaded_image = wlf_xpm_image_create();
		if (loaded_image == NULL) {
			free(input_path);
			free(output_path);
			return EXIT_FAILURE;
		}

		if (loaded_image->base.impl->load(&loaded_image->base, test_filename, false)) {
			loaded_image->base.image_type = WLF_IMAGE_TYPE_XPM;
			printf("✓ XPM image loaded successfully\n");
			print_image_info(&loaded_image->base);
		} else {
			printf("✗ Failed to load XPM image\n");
		}

		wlf_image_finish(&loaded_image->base);
		free(loaded_image);
	}

	free(input_path);
	free(output_path);
	return EXIT_SUCCESS;
}
