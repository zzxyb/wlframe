#include "wlf/image/wlf_png_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <strings.h>

#include <png.h>
#include <time.h>

#define PNG_BYTES_TO_CHECK 4

static bool png_image_save(struct wlf_image *image, const char *filename) {
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL) {
		wlf_log_errno(WLF_ERROR, "Open %s failed!", filename);
		return false;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		wlf_log(WLF_ERROR, "png_create_write_struct failed!");
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		wlf_log(WLF_ERROR, "png_create_info_struct failed!");
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		wlf_log(WLF_ERROR, "png_jmpbuf failed!");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);
	struct wlf_png_image *png_image = wlf_png_image_from_image(image);
	assert(png_image != NULL);
	int color_type = wlf_color_type_to_png(image);
	assert(color_type > -1);
	png_set_IHDR(png_ptr, info_ptr,
				image->width,
				image->height,
				image->bit_depth,
				color_type,
				png_image->interlace_type,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);

	png_write_info(png_ptr, info_ptr);

	png_bytep *row_pointers = malloc(sizeof(png_bytep) * image->height);
	for (uint32_t y = 0; y < image->height; ++y) {
		row_pointers[y] = image->data + y * image->width * wlf_image_get_channels(image);
	}

	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, NULL);

	free(row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	return true;
};

static bool png_image_load(struct wlf_image *image, const char *filename, bool enable_16_bit) {
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		wlf_log_errno(WLF_ERROR, "File %s is not a valid PNG image!", filename);
		return false;
	}

	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	int sig_read = 0;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
			&interlace_type, NULL, NULL);

	if (bit_depth == 16 && !enable_16_bit) {
		png_set_strip_16(png_ptr);
	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
		image->is_opaque = true;
		image->has_alpha_channel = true;
	}
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
		image->format = WLF_COLOR_TYPE_RGB;
		image->is_opaque = true;
	} else if (color_type == PNG_COLOR_TYPE_GRAY) {
		if (bit_depth < 8) {
			png_set_expand_gray_1_2_4_to_8(png_ptr);
		}
		image->format = WLF_COLOR_TYPE_GRAY;
	} else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
		image->format = WLF_COLOR_TYPE_GRAY_ALPHA;
		image->has_alpha_channel = true;
	} else if (color_type == PNG_COLOR_TYPE_RGB) {
		image->format = WLF_COLOR_TYPE_RGB;
	} else if (color_type == PNG_COLOR_TYPE_RGBA) {
		image->format = WLF_COLOR_TYPE_RGBA;
		image->has_alpha_channel = true;
	} else {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return false;
	}

	png_read_update_info(png_ptr, info_ptr);

	png_size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	image->data = (unsigned char *)malloc(rowbytes * height);
	png_bytep *row_pointers = malloc(sizeof(png_bytep) * height);
	for (png_uint_32 y = 0; y < height; y++) {
		row_pointers[y] = image->data + y * rowbytes;
	}

	png_read_image(png_ptr, row_pointers);
	free(row_pointers);
	image->width = width;
	image->height = height;
	image->bit_depth = bit_depth;
	image->stride = rowbytes;

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	wlf_log(WLF_DEBUG, "PNG Info: %dx%d, Format: %d, Bit Depth: %d\n, Stride: %d",
		image->width, image->height, image->format, image->bit_depth, image->stride);
	return true;
};

static void png_image_destroy(struct wlf_image *wlf_image) {
	struct wlf_png_image *image = wlf_png_image_from_image(wlf_image);
	free(image->base.data);
};

static const struct wlf_image_impl png_image_impl = {
	.save = png_image_save,
	.load = png_image_load,
	.destroy = png_image_destroy,
};

struct wlf_png_image *wlf_png_image_create(void) {
	struct wlf_png_image *image = malloc(sizeof(struct wlf_png_image));
	if (image == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate wlf_png_image");
		return NULL;
	}

	wlf_image_init(&image->base, &png_image_impl, 0, 0, 0);

	return image;
};

bool wlf_image_is_png(const struct wlf_image *image) {
	if (image == NULL) {
		return false;
	}

	return (image->impl == &png_image_impl &&
			image->image_type == WLF_IMAGE_TYPE_PNG);
}

struct wlf_png_image *wlf_png_image_from_image(struct wlf_image *wlf_image) {
	assert(wlf_image->impl == &png_image_impl);
	struct wlf_png_image *image = wlf_container_of(wlf_image, image, base);
	return image;
}

void wlf_png_image_print_data(const struct wlf_image *image) {
	if (image == NULL || !image->data) {
		wlf_log(WLF_ERROR, "PNG image data is NULL");
		return;
	}

	int channels = wlf_image_get_channels(image);
	size_t pixel_count = image->width * image->height;
	size_t stride = channels * image->bit_depth;

	for (size_t i = 0; i < pixel_count; ++i) {
		char line[128];
		int offset = 0;
		offset += snprintf(line + offset, sizeof(line) - offset, "Pixel %zu: ", i);

		if (image->bit_depth == 16) {
			for (int c = 0; c < channels; ++c) {
				const uint8_t *p = image->data + i * stride + c * 2;
				uint16_t value = (p[0] << 8) | p[1];  // Big-endian
				offset += snprintf(line + offset, sizeof(line) - offset, "%04X ", value);
			}
		} else {
			for (int c = 0; c < channels; ++c) {
				offset += snprintf(line + offset, sizeof(line) - offset, "%02X ", image->data[i * channels + c]);
			}
		}
		wlf_log(WLF_DEBUG, "%s", line);
	}
}

void wlf_png_image_print_data_gimp_style(const struct wlf_image *image) {
	if (image == NULL || !image->data) {
		wlf_log(WLF_ERROR, "PNG image data is NULL");
		return;
	}

	int channels = 0;
	const char *format_name = NULL;
	switch (image->format) {
		case WLF_COLOR_TYPE_RGB:
			channels = 3; format_name = "RGB"; break;
		case WLF_COLOR_TYPE_RGBA:
			channels = 4; format_name = "RGBA"; break;
		case WLF_COLOR_TYPE_GRAY:
			channels = 1; format_name = "GRAY"; break;
		case WLF_COLOR_TYPE_GRAY_ALPHA:
			channels = 2; format_name = "GRAY_ALPHA"; break;
		default:
			wlf_log(WLF_ERROR, "Unknown format: %u", image->format);
			return;
	}

	printf("/* GIMP %s C-Source image dump */\n", format_name);
	printf("static const struct {\n");
	printf("  unsigned int width, height;\n");
	printf("  unsigned int bytes_per_pixel; /* %d: %s */\n", channels, format_name);
	printf("  unsigned char pixel_data[%02u];\n", image->width * image->height * channels + 1);
	printf("} wlf_image_data = {\n");
	printf("  %d, %d, %d,\n", image->width, image->height, channels);
	printf("  \"\n");

	for (uint32_t y = 0; y < image->height; ++y) {
		const unsigned char *row = image->data + y * image->stride;
		char line[1024];
		int offset = 0;
		for (uint32_t x = 0; x < image->width * channels; ++x) {
			offset += snprintf(line + offset, sizeof(line) - offset, "\\x%02u", row[x]);
			if (offset >= (int)sizeof(line) - 6) {
				printf("%s", line);
				offset = 0;
			}
		}
		if (offset > 0) {
			printf("%s", line);
		}
	}
	printf("\"\n};\n");
}

int wlf_color_type_to_png(struct wlf_image *image) {
	assert(wlf_png_image_from_image(image) != NULL);
	if (image == NULL) {
		return -1;
	}

	switch (image->format) {
		case WLF_COLOR_TYPE_RGB:
			return PNG_COLOR_TYPE_RGB;
		case WLF_COLOR_TYPE_RGBA:
			return PNG_COLOR_TYPE_RGBA;
		case WLF_COLOR_TYPE_GRAY:
			return PNG_COLOR_TYPE_GRAY;
		case WLF_COLOR_TYPE_GRAY_ALPHA:
			return PNG_COLOR_TYPE_GRAY_ALPHA;
		default:
			return -1; // Unknown or unsupported
	}
}
