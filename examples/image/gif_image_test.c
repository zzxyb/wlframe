#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wlf/image/wlf_gif_image.h"
#include "wlf/image/wlf_image.h"
#include "wlf/utils/wlf_cmd_parser.h"
#include "wlf/utils/wlf_log.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static struct wlf_gif_image *create_test_image(uint32_t width, uint32_t height,
	const char *output_path, const char *filename) {
	struct wlf_gif_image *gif_image = wlf_gif_image_create();
	if (gif_image == NULL) {
		wlf_log(WLF_ERROR, "Failed to create wlf_gif_image");
		return NULL;
	}

	gif_image->base.width = width;
	gif_image->base.height = height;
	gif_image->base.format = WLF_COLOR_TYPE_RGBA;
	gif_image->base.bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	gif_image->base.stride = width * 4;
	gif_image->base.image_type = WLF_IMAGE_TYPE_GIF;
	gif_image->base.has_alpha_channel = true;
	gif_image->base.is_opaque = false;
	gif_image->loop_count = 0;
	gif_image->delay_ms = 100;

	size_t data_size = (size_t)width * height * 4;
	gif_image->base.data = malloc(data_size);
	if (gif_image->base.data == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate image data");
		free(gif_image);
		return NULL;
	}

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			size_t offset = ((size_t)y * width + x) * 4;
			unsigned char r = (unsigned char)((x * 255) / (width ? width : 1));
			unsigned char g = (unsigned char)((y * 255) / (height ? height : 1));
			unsigned char b = (unsigned char)(((x + y) * 255) /
				((width + height) ? (width + height) : 1));
			unsigned char a = 255;

			int32_t cx = (int32_t)(width / 2);
			int32_t cy = (int32_t)(height / 2);
			int32_t dx = (int32_t)x - cx;
			int32_t dy = (int32_t)y - cy;
			int32_t radius = (int32_t)((width < height ? width : height) / 4);
			if ((dx * dx + dy * dy) < (radius * radius)) {
				a = 100;
			}

			gif_image->base.data[offset + 0] = r;
			gif_image->base.data[offset + 1] = g;
			gif_image->base.data[offset + 2] = b;
			gif_image->base.data[offset + 3] = a;
		}
	}

	char full_path[PATH_MAX];
	if (output_path) {
		snprintf(full_path, sizeof(full_path), "%s/%s", output_path, filename);
	} else {
		strncpy(full_path, filename, sizeof(full_path) - 1);
		full_path[sizeof(full_path) - 1] = '\0';
	}

	if (wlf_image_save((struct wlf_image *)gif_image, full_path)) {
		wlf_log(WLF_INFO, "GIF test image saved: %s", full_path);
	} else {
		wlf_log(WLF_ERROR, "Failed to save GIF test image: %s", full_path);
	}

	return gif_image;
}

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("   or: %s [INPUT.gif] [OUTPUT.gif|OUTPUT_DIR]\n", program_name);
	printf("wlframe GIF Image Test Program\\n\\n");
	printf("Options:\n");
	printf("  -i, --input <path>      Input GIF file path to load and test\n");
	printf("  -o, --output <path>     Output directory or output gif file path\n");
	printf("  -w, --width <value>     Width for generated image (default: 256)\n");
	printf("  -H, --height <value>    Height for generated image (default: 256)\n");
	printf("  -v, --verbose           Enable verbose logging\n");
	printf("  -h, --help              Show this help message\\n\\n");
	printf("Examples:\n");
	printf("  %s                              # Create a test GIF in current directory\n", program_name);
	printf("  %s -i in.gif                    # Load and inspect GIF metadata\n", program_name);
	printf("  %s -i in.gif -o output/         # Save processed GIF to output directory\n", program_name);
	printf("  %s -w 320 -H 240 -v             # Generate 320x240 test GIF with verbose output\n", program_name);
}

int main(int argc, char *argv[]) {
	char *input_path = NULL;
	char *output_path = NULL;
	int width = 256;
	int height = 256;
	bool verbose = false;
	bool show_help = false;

	struct wlf_cmd_option options[] = {
		{ WLF_OPTION_STRING, "input", 'i', &input_path },
		{ WLF_OPTION_STRING, "output", 'o', &output_path },
		{ WLF_OPTION_INTEGER, "width", 'w', &width },
		{ WLF_OPTION_INTEGER, "height", 'H', &height },
		{ WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose },
		{ WLF_OPTION_BOOLEAN, "help", 'h', &show_help },
	};

	int remaining_args = wlf_cmd_parse_options(options, 6, &argc, argv);
	if (remaining_args < 0) {
		fprintf(stderr, "Error parsing command line options\n");
		return EXIT_FAILURE;
	}

	/* Support positional args:
	 *   gif_image_test input.gif output.gif
	 *   gif_image_test -i input.gif output.gif
	 */
	if (input_path == NULL && argc >= 2) {
		input_path = strdup(argv[1]);
		if (input_path == NULL) {
			fprintf(stderr, "Error: out of memory\n");
			return EXIT_FAILURE;
		}
		if (output_path == NULL && argc >= 3) {
			output_path = strdup(argv[2]);
			if (output_path == NULL) {
				fprintf(stderr, "Error: out of memory\n");
				free(input_path);
				return EXIT_FAILURE;
			}
		}
	} else if (input_path != NULL && output_path == NULL && argc >= 2) {
		output_path = strdup(argv[1]);
		if (output_path == NULL) {
			fprintf(stderr, "Error: out of memory\n");
			free(input_path);
			return EXIT_FAILURE;
		}
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

	printf("wlframe GIF Image Test\n");
	printf("======================\n");

	if (input_path) {
		printf("\nTesting GIF load: %s\n", input_path);
		struct wlf_image *loaded_image = wlf_image_load(input_path);
		if (loaded_image == NULL) {
			wlf_log(WLF_ERROR, "Failed to load GIF image: %s", input_path);
			free(input_path);
			free(output_path);
			return EXIT_FAILURE;
		}

		printf("GIF image loaded successfully\n");
		printf("  - Width: %u\n", loaded_image->width);
		printf("  - Height: %u\n", loaded_image->height);
		printf("  - Type: %s\n", wlf_image_get_type_string(loaded_image));
		printf("  - Format: %s\n", loaded_image->format == WLF_COLOR_TYPE_RGBA ? "RGBA" : "Other");
		printf("  - Bit depth: %d\n", loaded_image->bit_depth);
		printf("  - Has alpha: %s\n", loaded_image->has_alpha_channel ? "Yes" : "No");

		if (wlf_image_is_gif(loaded_image)) {
			struct wlf_gif_image *gif_img = wlf_gif_image_from_image(loaded_image);
			printf("  - Frame count: %u\n", gif_img->frame_count);
			printf("  - Delay (first frame): %u ms\n", gif_img->delay_ms);
			printf("  - Loop count: %u (0 means infinite/unknown)\n", gif_img->loop_count);
		}

		char output_filename[PATH_MAX];
		if (output_path) {
			const char *dot = strrchr(output_path, '.');
			if (dot && strcmp(dot, ".gif") == 0) {
				snprintf(output_filename, sizeof(output_filename), "%s", output_path);
			} else {
				const char *filename = strrchr(input_path, '/');
				filename = filename ? filename + 1 : input_path;
				snprintf(output_filename, sizeof(output_filename), "%s/processed_%s", output_path, filename);
			}
		} else {
			const char *filename = strrchr(input_path, '/');
			filename = filename ? filename + 1 : input_path;
			snprintf(output_filename, sizeof(output_filename), "processed_%s", filename);
		}

		if (wlf_image_save(loaded_image, output_filename)) {
			wlf_log(WLF_INFO, "Saved processed GIF: %s", output_filename);
		} else {
			wlf_log(WLF_ERROR, "Failed to save processed GIF: %s", output_filename);
		}

		wlf_image_finish(loaded_image);
		free(loaded_image);
	} else {
		printf("\nCreating test GIF image...\n");
		struct wlf_gif_image *gif_img = create_test_image((uint32_t)width, (uint32_t)height,
			output_path, "test_image.gif");
		if (gif_img == NULL) {
			free(input_path);
			free(output_path);
			return EXIT_FAILURE;
		}

		wlf_image_finish((struct wlf_image *)gif_img);
		free(gif_img);
	}

	free(input_path);
	free(output_path);
	return EXIT_SUCCESS;
}
