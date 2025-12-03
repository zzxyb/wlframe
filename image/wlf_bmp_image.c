#include "wlf/image/wlf_bmp_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

// BMP file header structure (14 bytes)
#pragma pack(push, 1)
struct bmp_file_header {
	uint16_t signature;      // "BM" (0x4D42)
	uint32_t file_size;      // Size of the BMP file
	uint16_t reserved1;      // Reserved, must be 0
	uint16_t reserved2;      // Reserved, must be 0
	uint32_t data_offset;    // Offset to start of pixel data
};

// BMP info header structure (40 bytes for BITMAPINFOHEADER)
struct bmp_info_header {
	uint32_t header_size;        // Size of info header (40)
	int32_t  width;              // Image width
	int32_t  height;             // Image height (negative = top-down)
	uint16_t planes;             // Number of color planes (must be 1)
	uint16_t bits_per_pixel;     // Bits per pixel
	uint32_t compression;        // Compression type
	uint32_t image_size;         // Size of compressed image (0 for RGB)
	int32_t  x_pixels_per_meter; // Horizontal resolution
	int32_t  y_pixels_per_meter; // Vertical resolution
	uint32_t colors_used;        // Number of colors used
	uint32_t important_colors;   // Number of important colors
};
#pragma pack(pop)

/**
 * @brief Calculate the row size with padding for BMP format.
 * BMP rows are padded to 4-byte boundaries.
 */
static uint32_t calculate_row_size(uint32_t width, uint16_t bits_per_pixel) {
	uint32_t bytes_per_row = (width * bits_per_pixel + 7) / 8;
	return (bytes_per_row + 3) & ~3; // Round up to nearest multiple of 4
}

/**
 * @brief Write a 16-bit value in little-endian format.
 */
static void write_uint16_le(FILE *fp, uint16_t value) {
	fputc(value & 0xFF, fp);
	fputc((value >> 8) & 0xFF, fp);
}

/**
 * @brief Write a 32-bit value in little-endian format.
 */
static void write_uint32_le(FILE *fp, uint32_t value) {
	fputc(value & 0xFF, fp);
	fputc((value >> 8) & 0xFF, fp);
	fputc((value >> 16) & 0xFF, fp);
	fputc((value >> 24) & 0xFF, fp);
}

/**
 * @brief Read a 16-bit value in little-endian format.
 */
static uint16_t read_uint16_le(FILE *fp) {
	uint16_t value = 0;
	value |= fgetc(fp);
	value |= (fgetc(fp) << 8);
	return value;
}

/**
 * @brief Read a 32-bit value in little-endian format.
 */
static uint32_t read_uint32_le(FILE *fp) {
	uint32_t value = 0;
	value |= fgetc(fp);
	value |= (fgetc(fp) << 8);
	value |= (fgetc(fp) << 16);
	value |= (fgetc(fp) << 24);
	return value;
}

static bool bmp_image_save(struct wlf_image *image, const char *filename) {
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL) {
		wlf_log_errno(WLF_ERROR, "Open %s failed!", filename);
		return false;
	}

	struct wlf_bmp_image *bmp_image = wlf_bmp_image_from_image(image);
	assert(bmp_image != NULL);

	// Only support RGB format for now
	if (image->format != WLF_COLOR_TYPE_RGB) {
		wlf_log(WLF_ERROR, "BMP format currently only supports RGB images!");
		fclose(fp);
		return false;
	}

	uint16_t bits_per_pixel = 24; // RGB = 24 bits per pixel
	uint32_t row_size = calculate_row_size(image->width, bits_per_pixel);
	uint32_t image_size = row_size * image->height;
	uint32_t file_size = 14 + 40 + image_size; // file header + info header + image data

	// Write BMP file header
	write_uint16_le(fp, 0x4D42); // "BM" signature
	write_uint32_le(fp, file_size);
	write_uint16_le(fp, 0); // reserved1
	write_uint16_le(fp, 0); // reserved2
	write_uint32_le(fp, 54); // data offset (14 + 40)

	// Write BMP info header
	write_uint32_le(fp, 40); // header size
	write_uint32_le(fp, image->width);
	write_uint32_le(fp, bmp_image->top_down ? -(int32_t)image->height : (int32_t)image->height);
	write_uint16_le(fp, 1); // planes
	write_uint16_le(fp, bits_per_pixel);
	write_uint32_le(fp, bmp_image->compression);
	write_uint32_le(fp, image_size);
	write_uint32_le(fp, 2835); // x pixels per meter (72 DPI)
	write_uint32_le(fp, 2835); // y pixels per meter (72 DPI)
	write_uint32_le(fp, 0); // colors used
	write_uint32_le(fp, 0); // important colors

	// Write pixel data (BMP stores rows bottom-up by default, BGR format)
	uint8_t padding[4] = {0, 0, 0, 0};
	uint32_t padding_size = row_size - (image->width * 3);

	for (int32_t y = image->height - 1; y >= 0; y--) {
		for (uint32_t x = 0; x < image->width; x++) {
			uint32_t offset = (y * image->width + x) * 3;
			// Convert RGB to BGR
			fputc(image->data[offset + 2], fp); // B
			fputc(image->data[offset + 1], fp); // G
			fputc(image->data[offset], fp);     // R
		}
		// Write padding bytes
		if (padding_size > 0) {
			fwrite(padding, 1, padding_size, fp);
		}
	}

	fclose(fp);
	return true;
}

static bool bmp_image_load(struct wlf_image *image, const char *filename, bool enable_16_bit) {
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		wlf_log_errno(WLF_ERROR, "File %s cannot be opened!", filename);
		return false;
	}

	// Read and verify BMP file header
	uint16_t signature = read_uint16_le(fp);
	if (signature != 0x4D42) { // "BM"
		wlf_log(WLF_ERROR, "File %s is not a valid BMP image!", filename);
		fclose(fp);
		return false;
	}

	read_uint32_le(fp); // file_size (not used)
	read_uint16_le(fp); // reserved1 (not used)
	read_uint16_le(fp); // reserved2 (not used)
	uint32_t data_offset = read_uint32_le(fp);

	// Read BMP info header
	uint32_t header_size = read_uint32_le(fp);
	if (header_size != 40) {
		wlf_log(WLF_ERROR, "Unsupported BMP header size: %u", header_size);
		fclose(fp);
		return false;
	}

	int32_t width = (int32_t)read_uint32_le(fp);
	int32_t height = (int32_t)read_uint32_le(fp);
	uint16_t planes = read_uint16_le(fp);
	uint16_t bits_per_pixel = read_uint16_le(fp);
	uint32_t compression = read_uint32_le(fp);
	read_uint32_le(fp); // image_size (not used)
	read_uint32_le(fp); // x_ppm (not used)
	read_uint32_le(fp); // y_ppm (not used)
	uint32_t colors_used = read_uint32_le(fp);
	uint32_t important_colors = read_uint32_le(fp);

	// Validate header values
	if (width <= 0 || abs(height) <= 0 || planes != 1) {
		wlf_log(WLF_ERROR, "Invalid BMP dimensions or plane count!");
		fclose(fp);
		return false;
	}

	// Only support 24-bit RGB for now
	if (bits_per_pixel != 24 || compression != WLF_BMP_COMPRESSION_RGB) {
		wlf_log(WLF_ERROR, "Unsupported BMP format: %u bits, compression %u",
			bits_per_pixel, compression);
		fclose(fp);
		return false;
	}

	bool top_down = (height < 0);
	uint32_t abs_height = abs(height);
	uint32_t row_size = calculate_row_size(width, bits_per_pixel);

	// Allocate memory for pixel data
	size_t data_size = width * abs_height * 3; // RGB format
	image->data = malloc(data_size);
	if (image->data == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate image data");
		fclose(fp);
		return false;
	}

	// Seek to pixel data
	fseek(fp, data_offset, SEEK_SET);

	// Read pixel data
	uint8_t *row_buffer = malloc(row_size);
	if (row_buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate row buffer");
		free(image->data);
		fclose(fp);
		return false;
	}

	for (uint32_t y = 0; y < abs_height; y++) {
		if (fread(row_buffer, 1, row_size, fp) != row_size) {
			wlf_log(WLF_ERROR, "Error reading pixel data!");
			free(row_buffer);
			free(image->data);
			fclose(fp);
			return false;
		}

		// Determine the target row (BMP is bottom-up by default)
		uint32_t target_y = top_down ? y : (abs_height - 1 - y);

		// Convert BGR to RGB
		for (uint32_t x = 0; x < (uint32_t)width; x++) {
			uint32_t src_offset = x * 3;
			uint32_t dst_offset = (target_y * width + x) * 3;

			image->data[dst_offset] = row_buffer[src_offset + 2];     // R
			image->data[dst_offset + 1] = row_buffer[src_offset + 1]; // G
			image->data[dst_offset + 2] = row_buffer[src_offset];     // B
		}
	}

	free(row_buffer);

	// Set image properties
	image->width = width;
	image->height = abs_height;
	image->format = WLF_COLOR_TYPE_RGB;
	image->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	image->stride = width * 3;
	image->has_alpha_channel = false;
	image->is_opaque = true;
	image->image_type = WLF_IMAGE_TYPE_BMP;

	// Set BMP-specific properties
	struct wlf_bmp_image *bmp_image = wlf_bmp_image_from_image(image);
	bmp_image->compression = compression;
	bmp_image->bits_per_pixel = bits_per_pixel;
	bmp_image->colors_used = colors_used;
	bmp_image->important_colors = important_colors;
	bmp_image->top_down = top_down;

	fclose(fp);
	return true;
}

static void bmp_image_destroy(struct wlf_image *wlf_image) {
	struct wlf_bmp_image *image = wlf_bmp_image_from_image(wlf_image);
	free(image->base.data);
}

static const struct wlf_image_impl bmp_image_impl = {
	.save = bmp_image_save,
	.load = bmp_image_load,
	.destroy = bmp_image_destroy,
};

struct wlf_bmp_image *wlf_bmp_image_create(void) {
	struct wlf_bmp_image *image = malloc(sizeof(struct wlf_bmp_image));
	if (image == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate wlf_bmp_image");
		return NULL;
	}

	wlf_image_init(&image->base, &bmp_image_impl, 0, 0, 0);
	image->base.image_type = WLF_IMAGE_TYPE_BMP;
	image->compression = WLF_BMP_COMPRESSION_RGB;
	image->bits_per_pixel = 24; // Default to 24-bit RGB
	image->colors_used = 0;
	image->important_colors = 0;
	image->top_down = false; // Default to bottom-up

	return image;
}

bool wlf_image_is_bmp(struct wlf_image *image) {
	if (image == NULL) {
		return false;
	}

	return (image->impl == &bmp_image_impl &&
			image->image_type == WLF_IMAGE_TYPE_BMP);
}

struct wlf_bmp_image *wlf_bmp_image_from_image(struct wlf_image *wlf_image) {
	assert(wlf_image->impl == &bmp_image_impl);
	struct wlf_bmp_image *image = wlf_container_of(wlf_image, image, base);
	return image;
}

void wlf_bmp_image_set_compression(struct wlf_bmp_image *image, enum wlf_bmp_compression compression) {
	if (image) {
		image->compression = compression;
	}
}

void wlf_bmp_image_set_bits_per_pixel(struct wlf_bmp_image *image, uint32_t bits_per_pixel) {
	if (image) {
		image->bits_per_pixel = bits_per_pixel;
	}
}
