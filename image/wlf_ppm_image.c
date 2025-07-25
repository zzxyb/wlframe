#include "wlf/image/wlf_ppm_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/**
 * @brief Skip whitespace and comments in PPM file.
 * @param fp File pointer.
 */
static void skip_whitespace_and_comments(FILE *fp) {
	int c;
	while ((c = fgetc(fp)) != EOF) {
		if (c == '#') {
			// Skip comment line
			while ((c = fgetc(fp)) != EOF && c != '\n');
		} else if (!isspace(c)) {
			ungetc(c, fp);
			break;
		}
	}
}

/**
 * @brief Read a number from PPM file, skipping whitespace and comments.
 * @param fp File pointer.
 * @return The number read, or -1 on error.
 */
static int read_ppm_number(FILE *fp) {
	int num = 0;
	int c;

	skip_whitespace_and_comments(fp);

	while ((c = fgetc(fp)) != EOF && isdigit(c)) {
		num = num * 10 + (c - '0');
	}

	if (c != EOF) {
		ungetc(c, fp);
	}

	return num;
}

static bool ppm_image_save(struct wlf_image *image, const char *filename) {
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		wlf_log(WLF_ERROR, "Open %s failed!", filename);
		return false;
	}

	struct wlf_ppm_image *ppm_image = wlf_ppm_image_from_image(image);
	assert(ppm_image != NULL);

	if (image->format != WLF_COLOR_TYPE_RGB) {
		wlf_log(WLF_ERROR, "PPM format only supports RGB images!");
		fclose(fp);
		return false;
	}

	if (ppm_image->format == WLF_PPM_FORMAT_P3) {
		fprintf(fp, "P3\n");
	} else {
		fprintf(fp, "P6\n");
	}

	fprintf(fp, "%u %u\n", image->width, image->height);
	fprintf(fp, "%u\n", ppm_image->max_val);

	if (ppm_image->format == WLF_PPM_FORMAT_P3) {
		for (uint32_t y = 0; y < image->height; y++) {
			for (uint32_t x = 0; x < image->width; x++) {
				uint32_t offset = (y * image->width + x) * 3;
				// Scale values from 0-255 range to 0-max_val range
				uint32_t r = (image->data[offset] * ppm_image->max_val) / 255;
				uint32_t g = (image->data[offset + 1] * ppm_image->max_val) / 255;
				uint32_t b = (image->data[offset + 2] * ppm_image->max_val) / 255;

				fprintf(fp, "%u %u %u ",
					r,  // R
					g,  // G
					b   // B
				);
			}
		}
	} else {
		if (ppm_image->max_val == 255) {
			// Direct write if max_val is 255
			size_t pixels = image->width * image->height * 3;
			if (fwrite(image->data, 1, pixels, fp) != pixels) {
				wlf_log(WLF_ERROR, "Failed to write pixel data!");
				fclose(fp);
				return false;
			}
		} else {
			// Scale and write if max_val is not 255
			for (uint32_t y = 0; y < image->height; y++) {
				for (uint32_t x = 0; x < image->width; x++) {
					uint32_t offset = (y * image->width + x) * 3;
					unsigned char r = (image->data[offset] * ppm_image->max_val) / 255;
					unsigned char g = (image->data[offset + 1] * ppm_image->max_val) / 255;
					unsigned char b = (image->data[offset + 2] * ppm_image->max_val) / 255;

					fputc(r, fp);
					fputc(g, fp);
					fputc(b, fp);
				}
			}
		}
	}

	fclose(fp);
	return true;
}

static bool ppm_image_load(struct wlf_image *image, const char *filename, bool enable_16_bit) {
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		wlf_log(WLF_ERROR, "File %s cannot be opened!", filename);
		return false;
	}

	char magic[3];
	if (fread(magic, 1, 2, fp) != 2) {
		wlf_log(WLF_ERROR, "Cannot read PPM magic number!");
		fclose(fp);
		return false;
	}
	magic[2] = '\0';

	enum wlf_ppm_format format;
	if (strcmp(magic, "P3") == 0) {
		format = WLF_PPM_FORMAT_P3;
	} else if (strcmp(magic, "P6") == 0) {
		format = WLF_PPM_FORMAT_P6;
	} else {
		wlf_log(WLF_ERROR, "File %s is not a valid PPM image!", filename);
		fclose(fp);
		return false;
	}

	int width = read_ppm_number(fp);
	int height = read_ppm_number(fp);
	int max_val = read_ppm_number(fp);

	if (width <= 0 || height <= 0 || max_val <= 0) {
		wlf_log(WLF_ERROR, "Invalid PPM dimensions or max value!");
		fclose(fp);
		return false;
	}

	fgetc(fp);
	size_t data_size = width * height * 3; // RGB format
	image->data = malloc(data_size);
	if (!image->data) {
		wlf_log(WLF_ERROR, "Memory allocation failed!");
		fclose(fp);
		return false;
	}

	if (format == WLF_PPM_FORMAT_P3) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int r = read_ppm_number(fp);
				int g = read_ppm_number(fp);
				int b = read_ppm_number(fp);

				if (r < 0 || g < 0 || b < 0) {
					wlf_log(WLF_ERROR, "Error reading pixel data!");
					free(image->data);
					fclose(fp);
					return false;
				}

				// Scale values to 0-255 range if max_val is not 255
				if (max_val != 255) {
					r = (r * 255) / max_val;
					g = (g * 255) / max_val;
					b = (b * 255) / max_val;
				}

				uint32_t offset = (y * width + x) * 3;
				image->data[offset] = (unsigned char)r;
				image->data[offset + 1] = (unsigned char)g;
				image->data[offset + 2] = (unsigned char)b;
			}
		}
	} else {
		if (fread(image->data, 1, data_size, fp) != data_size) {
			wlf_log(WLF_ERROR, "Error reading binary pixel data!");
			free(image->data);
			fclose(fp);
			return false;
		}

		// Scale values to 0-255 range if max_val is not 255
		if (max_val != 255) {
			for (size_t i = 0; i < data_size; i++) {
				image->data[i] = (image->data[i] * 255) / max_val;
			}
		}
	}

	image->width = width;
	image->height = height;
	image->format = WLF_COLOR_TYPE_RGB;
	image->bit_depth = (max_val <= 255) ? WLF_IMAGE_BIT_DEPTH_8 : WLF_IMAGE_BIT_DEPTH_16;
	image->stride = width * 3;
	image->has_alpha_channel = false;
	image->is_opaque = true;
	image->image_type = WLF_IMAGE_TYPE_PPM;

	struct wlf_ppm_image *ppm_image = wlf_ppm_image_from_image(image);
	ppm_image->format = format;
	ppm_image->max_val = max_val; // Preserve original max_val

	fclose(fp);
	return true;
}

static void ppm_image_destroy(struct wlf_image *wlf_image) {
	struct wlf_ppm_image *image = wlf_ppm_image_from_image(wlf_image);
	free(image->base.data);
}

static const struct wlf_image_impl ppm_image_impl = {
	.save = ppm_image_save,
	.load = ppm_image_load,
	.destroy = ppm_image_destroy,
};

struct wlf_ppm_image *wlf_ppm_image_create(void) {
	struct wlf_ppm_image *image = malloc(sizeof(struct wlf_ppm_image));
	if (image == NULL) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_ppm_image failed!");
		return NULL;
	}

	wlf_image_init(&image->base, &ppm_image_impl, 0, 0, 0);
	image->base.image_type = WLF_IMAGE_TYPE_PPM;
	image->format = WLF_PPM_FORMAT_P6; // Default to binary format
	image->max_val = 255; // Default to 8-bit

	return image;
}

bool wlf_image_is_ppm(struct wlf_image *image) {
	if (!image) {
		return false;
	}

	return (image->impl == &ppm_image_impl &&
			image->image_type == WLF_IMAGE_TYPE_PPM);
}

struct wlf_ppm_image *wlf_ppm_image_from_image(struct wlf_image *wlf_image) {
	assert(wlf_image->impl == &ppm_image_impl);
	struct wlf_ppm_image *image = wlf_container_of(wlf_image, image, base);
	return image;
}

void wlf_ppm_image_set_format(struct wlf_ppm_image *image, enum wlf_ppm_format format) {
	if (image) {
		image->format = format;
	}
}

void wlf_ppm_image_set_max_val(struct wlf_ppm_image *image, uint32_t max_val) {
	if (image) {
		image->max_val = max_val;
	}
}
