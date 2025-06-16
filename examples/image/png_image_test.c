#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_png_image.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
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

int main(int argc, char *argv[]) {
	int32_t log_level = WLF_DEBUG;
	wlf_log_init(log_level, NULL);

	if (argc > 1) {
		const char *filename = argv[1];
		struct wlf_image *img = wlf_image_load(filename);
		if (img == NULL) {
			wlf_log(WLF_INFO, "Failed to load image: %s\n", filename);
			return EXIT_FAILURE;
		} else {
			wlf_log(WLF_INFO, "Image loaded successfully: %s", filename);
			wlf_log(WLF_INFO, "Image type: %s", wlf_image_get_type_string(img));
			wlf_log(WLF_INFO, "Image size: %ux%u", img->width, img->height);
			wlf_log(WLF_INFO, "Image format: %d", img->format);
			wlf_log(WLF_INFO, "Image bit depth: %d", img->bit_depth);
			wlf_log(WLF_INFO, "Image stride: %d", img->stride);
			wlf_log(WLF_INFO, "Image has alpha channel: %s", img->has_alpha_channel ? "[✓]" : "[✗]");
			wlf_log(WLF_INFO, "Image is opaque: %s", img->is_opaque ? "[✓]" : "[✗]");
			wlf_log(WLF_INFO, "Image channels: %d", wlf_image_get_channels(img));
		}

		const char *save_path = "wlf_image_test_save.png";
		if (wlf_image_save(img, save_path)) {
			wlf_log(WLF_INFO, "Image saved successfully to: %s", save_path);
		} else {
			wlf_log(WLF_ERROR, "Failed to save image to: %s", save_path);
		}

		wlf_image_finish(img);
	} else {
		wlf_log(WLF_INFO, "No input file specified. Creating demo images...");
		wlf_log(WLF_INFO, "Creating chess board pattern...");
		struct wlf_image *chess_img = create_chess_board(400, 400, 50);
		if (chess_img) {
			const char *chess_path = "chess_board.png";
			if (wlf_image_save(chess_img, chess_path)) {
				wlf_log(WLF_INFO, "Chess board saved to: %s", chess_path);
			} else {
				wlf_log(WLF_ERROR, "Failed to save chess board to: %s", chess_path);
			}
			wlf_image_finish(chess_img);
		} else {
			wlf_log(WLF_ERROR, "Failed to create chess board image");
		}
	}

	return EXIT_SUCCESS;
}
