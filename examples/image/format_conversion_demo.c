#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_jpeg_image.h"
#include "wlf/image/wlf_png_image.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct wlf_image *create_test_image(uint32_t width, uint32_t height) {
	struct wlf_jpeg_image *jpeg_image = wlf_jpeg_image_create();
	if (!jpeg_image) {
		return NULL;
	}

	struct wlf_image *base = &jpeg_image->base;
	base->width = width;
	base->height = height;
	base->format = WLF_COLOR_TYPE_RGB;
	base->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	base->stride = width * 3;
	base->image_type = WLF_IMAGE_TYPE_JPEG;
	base->has_alpha_channel = false;
	base->is_opaque = true;

	base->data = malloc(height * base->stride);
	if (!base->data) {
		free(jpeg_image);
		return NULL;
	}

	// Create colorful pattern
	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			unsigned char *pixel = base->data + y * base->stride + x * 3;
			pixel[0] = (unsigned char)(255 * x / width);           // Red
			pixel[1] = (unsigned char)(255 * y / height);          // Green
			pixel[2] = (unsigned char)(128 + 127 * (x + y) / (width + height)); // Blue
		}
	}

	return base;
}

static void print_file_size(const char *filename) {
	FILE *fp = fopen(filename, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		fclose(fp);
		printf("  File size: %ld bytes\n", size);
	} else {
		printf("  File size: N/A\n");
	}
}

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_INFO, NULL);

	printf("wlframe Image Format Conversion Demo\n");
	printf("====================================\n");

	// Create a test image
	struct wlf_image *image = create_test_image(150, 100);
	if (!image) {
		printf("Failed to create test image\n");
		return EXIT_FAILURE;
	}

	printf("Created test image: %dx%d RGB\n\n", image->width, image->height);

	// Save as JPEG with different qualities
	printf("=== JPEG Compression Test ===\n");
	struct wlf_jpeg_image *jpeg_img = wlf_jpeg_image_from_image(image);
	
	int qualities[] = {50, 75, 90, 95};
	for (int i = 0; i < 4; i++) {
		char filename[64];
		snprintf(filename, sizeof(filename), "demo_q%d.jpg", qualities[i]);
		
		wlf_jpeg_image_set_quality(jpeg_img, qualities[i]);
		if (wlf_image_save(image, filename)) {
			printf("✓ Saved JPEG quality %d: %s\n", qualities[i], filename);
			print_file_size(filename);
		}
	}

	// Save as PNG
	printf("\n=== Format Conversion Test ===\n");
	if (wlf_image_save(image, "demo_converted.png")) {
		printf("✓ Saved as PNG: demo_converted.png\n");
		print_file_size("demo_converted.png");
	}

	// Test loading back different formats
	printf("\n=== Load and Verify Test ===\n");
	
	const char *test_files[] = {"demo_q90.jpg", "demo_converted.png"};
	for (int i = 0; i < 2; i++) {
		struct wlf_image *loaded = wlf_image_load(test_files[i]);
		if (loaded) {
			printf("✓ Loaded %s:\n", test_files[i]);
			printf("  Dimensions: %dx%d\n", loaded->width, loaded->height);
			printf("  Format: %d\n", loaded->format);
			printf("  Type: %s\n", wlf_image_get_type_string(loaded));
			
			// Save in opposite format
			char converted_filename[128];
			if (strstr(test_files[i], ".jpg")) {
				snprintf(converted_filename, sizeof(converted_filename), "converted_%s.png", test_files[i]);
			} else {
				snprintf(converted_filename, sizeof(converted_filename), "converted_%s.jpg", test_files[i]);
			}
			
			if (wlf_image_save(loaded, converted_filename)) {
				printf("  ✓ Cross-converted to: %s\n", converted_filename);
				print_file_size(converted_filename);
			}
			
			wlf_image_finish(loaded);
		} else {
			printf("✗ Failed to load %s\n", test_files[i]);
		}
		printf("\n");
	}

	// Cleanup
	wlf_image_finish(image);

	printf("=== Demo Summary ===\n");
	printf("Generated files:\n");
	printf("  - demo_q*.jpg     (JPEG at different quality levels)\n");
	printf("  - demo_converted.png (PNG conversion)\n");
	printf("  - converted_*.jpg/png (Cross-format conversions)\n");
	printf("\nDemo completed successfully!\n");

	return EXIT_SUCCESS;
}
