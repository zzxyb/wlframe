#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_bmp_image.h"
#include "wlf/utils/wlf_cmd_parser.h"
#include "wlf/utils/wlf_log.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * @brief Create a simple test BMP image with a checkerboard pattern.
 * @param width Image width.
 * @param height Image height.
 * @param output_path Optional output directory path.
 * @param filename Output filename.
 * @return Pointer to the created BMP image, or NULL on failure.
 */
static struct wlf_bmp_image *create_test_image(uint32_t width, uint32_t height, const char *output_path, const char *filename) {
	struct wlf_bmp_image *bmp_image = wlf_bmp_image_create();
	if (!bmp_image) {
		return NULL;
	}

	bmp_image->base.width = width;
	bmp_image->base.height = height;
	bmp_image->base.format = WLF_COLOR_TYPE_RGB;
	bmp_image->base.bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	bmp_image->base.stride = width * 3;
	bmp_image->base.has_alpha_channel = false;
	bmp_image->base.is_opaque = true;

	// Allocate pixel data
	size_t data_size = width * height * 3;
	bmp_image->base.data = malloc(data_size);
	if (!bmp_image->base.data) {
		free(bmp_image);
		return NULL;
	}

	// Create a checkerboard pattern
	uint32_t block_size = 32;
	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			uint32_t offset = (y * width + x) * 3;

			// Determine if this pixel is in a "black" or "colored" block
			bool black_block = ((x / block_size) + (y / block_size)) % 2 == 0;

			if (black_block) {
				// Black block
				bmp_image->base.data[offset] = 0;     // R
				bmp_image->base.data[offset + 1] = 0; // G
				bmp_image->base.data[offset + 2] = 0; // B
			} else {
				// Colored block with gradient
				bmp_image->base.data[offset] = (unsigned char)((x * 255) / width);        // R
				bmp_image->base.data[offset + 1] = (unsigned char)((y * 255) / height);  // G
				bmp_image->base.data[offset + 2] = (unsigned char)(((x + y) * 255) / (width + height)); // B
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

	if (wlf_image_save((struct wlf_image *)bmp_image, full_path)) {
		wlf_log(WLF_INFO, "✓ BMP test image saved: %s", full_path);
	} else {
		wlf_log(WLF_ERROR, "✗ Failed to save BMP test image: %s", full_path);
	}

	return bmp_image;
}

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("wlframe BMP Image Test Program\n\n");
	printf("Options:\n");
	printf("  -i, --input <path>      Input BMP file path to load and test\n");
	printf("  -o, --output <path>     Output path for saved images (default: current directory)\n");
	printf("  -w, --width <value>     Width for test image (default: 256)\n");
	printf("  -H, --height <value>    Height for test image (default: 256)\n");
	printf("  -v, --verbose           Enable verbose logging\n");
	printf("  -h, --help              Show this help message\n\n");
	printf("Examples:\n");
	printf("  %s                              # Create test images in current directory\n", program_name);
	printf("  %s -i image.bmp                 # Load and test image.bmp\n", program_name);
	printf("  %s -i image.bmp -o output/      # Load image.bmp and save to output directory\n", program_name);
	printf("  %s -w 512 -H 512 -v             # Create 512x512 test image with verbose output\n", program_name);
}

int main(int argc, char *argv[]) {
	// Command line options
	char *input_path = NULL;
	char *output_path = NULL;
	int width = 256;
	int height = 256;
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
	if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
		fprintf(stderr, "Error: Width and height must be between 1 and 4096\n");
		return EXIT_FAILURE;
	}

	// Initialize logging
	int32_t log_level = verbose ? WLF_DEBUG : WLF_INFO;
	wlf_log_init(log_level, NULL);

	printf("wlframe BMP Image Test\n");
	printf("======================\n");
	if (verbose) {
		printf("Verbose mode enabled\n");
		printf("Test image dimensions: %dx%d\n", width, height);
		if (input_path) printf("Input path: %s\n", input_path);
		if (output_path) printf("Output path: %s\n", output_path);
		printf("\n");
	}

	if (input_path) {
		// Test loading provided BMP file
		printf("\nTesting BMP load: %s\n", input_path);
		struct wlf_image *loaded_image = wlf_image_load(input_path);
		if (loaded_image) {
			printf("✓ BMP image loaded successfully\n");
			printf("  - Width: %u\n", loaded_image->width);
			printf("  - Height: %u\n", loaded_image->height);
			printf("  - Format: %s\n",
				loaded_image->format == WLF_COLOR_TYPE_RGB ? "RGB" : "Other");
			printf("  - Type: %s\n", wlf_image_get_type_string(loaded_image));
			printf("  - Bit depth: %d\n", loaded_image->bit_depth);

			// Verify it's a BMP image
			if (wlf_image_is_bmp(loaded_image)) {
				printf("✓ Image correctly identified as BMP\n");
				struct wlf_bmp_image *loaded_bmp = wlf_bmp_image_from_image(loaded_image);
				printf("  - Compression: %s\n",
					loaded_bmp->compression == WLF_BMP_COMPRESSION_RGB ? "RGB (No compression)" : "Other");
				printf("  - Bits per pixel: %u\n", loaded_bmp->bits_per_pixel);
				printf("  - Top-down: %s\n", loaded_bmp->top_down ? "Yes" : "No");
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
			wlf_log(WLF_ERROR, "✗ Failed to load BMP image: %s", input_path);
		}
	} else {
		// Create and test BMP images
		printf("\nTest 1: Creating a test BMP image...\n");
		struct wlf_bmp_image *test_image = create_test_image(width, height, output_path, "test_image.bmp");
		if (!test_image) {
			printf("Failed to create test image!\n");
			free(input_path);
			free(output_path);
			return 1;
		}
		printf("✓ Test image created successfully\n");

		// Load it back and verify
		char test_filename[PATH_MAX];
		if (output_path) {
			snprintf(test_filename, sizeof(test_filename), "%s/test_image.bmp", output_path);
		} else {
			strcpy(test_filename, "test_image.bmp");
		}
		printf("\nTest 2: Loading BMP image back...\n");
		struct wlf_image *loaded_image = wlf_image_load(test_filename);
		if (loaded_image) {
			printf("✓ BMP image loaded successfully\n");
			printf("  - Width: %u\n", loaded_image->width);
			printf("  - Height: %u\n", loaded_image->height);
			printf("  - Format: %s\n",
				loaded_image->format == WLF_COLOR_TYPE_RGB ? "RGB" : "Other");
			printf("  - Type: %s\n", wlf_image_get_type_string(loaded_image));
			printf("  - Bit depth: %d\n", loaded_image->bit_depth);

			// Verify it's a BMP image
			if (wlf_image_is_bmp(loaded_image)) {
				printf("✓ Image correctly identified as BMP\n");
				struct wlf_bmp_image *loaded_bmp = wlf_bmp_image_from_image(loaded_image);
				printf("  - Compression: %s\n",
					loaded_bmp->compression == WLF_BMP_COMPRESSION_RGB ? "RGB (No compression)" : "Other");
				printf("  - Bits per pixel: %u\n", loaded_bmp->bits_per_pixel);
				printf("  - Top-down: %s\n", loaded_bmp->top_down ? "Yes" : "No");
			} else {
				printf("✗ Image not identified as BMP\n");
			}

			// Test 3: Compare original and loaded image data
			if (loaded_image && test_image) {
				printf("\nTest 3: Comparing original and loaded images...\n");

				if (loaded_image->width == test_image->base.width &&
				    loaded_image->height == test_image->base.height &&
				    loaded_image->format == test_image->base.format) {
					printf("✓ Image dimensions and format match\n");

					// Sample a few pixels to verify data integrity
					bool data_matches = true;
					uint32_t samples = 10;
					for (uint32_t i = 0; i < samples && data_matches; i++) {
						uint32_t x = (i * loaded_image->width) / samples;
						uint32_t y = (i * loaded_image->height) / samples;
						uint32_t offset = (y * loaded_image->width + x) * 3;

						if (offset + 2 < loaded_image->width * loaded_image->height * 3) {
							if (loaded_image->data[offset] != test_image->base.data[offset] ||
							    loaded_image->data[offset + 1] != test_image->base.data[offset + 1] ||
							    loaded_image->data[offset + 2] != test_image->base.data[offset + 2]) {
								data_matches = false;
							}
						}
					}

					if (data_matches) {
						printf("✓ Sampled pixel data matches\n");
					} else {
						printf("✗ Pixel data doesn't match\n");
					}
				} else {
					printf("✗ Image dimensions or format don't match\n");
				}
			}

			wlf_image_finish(loaded_image);
			free(loaded_image);
		} else {
			printf("✗ Failed to load BMP image\n");
		}

		// Test 4: Test different BMP settings
		printf("\nTest 4: Testing BMP settings...\n");

		// Test top-down orientation
		struct wlf_bmp_image *test_image2 = create_test_image(64, 64, output_path, "test_topdown.bmp");
		if (test_image2) {
			test_image2->top_down = true;
			char topdown_filename[PATH_MAX];
			if (output_path) {
				snprintf(topdown_filename, sizeof(topdown_filename), "%s/test_topdown.bmp", output_path);
			} else {
				strcpy(topdown_filename, "test_topdown.bmp");
			}

			if (wlf_image_save((struct wlf_image *)test_image2, topdown_filename)) {
				printf("✓ Top-down BMP image saved\n");

				struct wlf_image *loaded_topdown = wlf_image_load(topdown_filename);
				if (loaded_topdown && wlf_image_is_bmp(loaded_topdown)) {
					struct wlf_bmp_image *loaded_bmp = wlf_bmp_image_from_image(loaded_topdown);
					printf("✓ Top-down BMP loaded, orientation: %s\n",
						loaded_bmp->top_down ? "Top-down" : "Bottom-up");

					wlf_image_finish(loaded_topdown);
					free(loaded_topdown);
				}
			}

			wlf_image_finish((struct wlf_image *)test_image2);
			free(test_image2);
		}

		wlf_image_finish((struct wlf_image *)test_image);
		free(test_image);
	}

	// Cleanup allocated strings
	free(input_path);
	free(output_path);

	printf("\nBMP image test completed!\n");
	return 0;
}
