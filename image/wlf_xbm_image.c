#include "wlf/image/wlf_xbm_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/**
 * @brief Extract the base name from a filename (without path and extension).
 */
static char *extract_base_name(const char *filename) {
	const char *basename = strrchr(filename, '/');
	if (basename) {
		basename++; // Skip the '/'
	} else {
		basename = filename;
	}

	// Find the extension
	const char *ext = strrchr(basename, '.');
	size_t name_len = ext ? (size_t)(ext - basename) : strlen(basename);

	char *name = malloc(name_len + 1);
	if (name) {
		strncpy(name, basename, name_len);
		name[name_len] = '\0';
	}

	return name;
}

/**
 * @brief Convert a grayscale value to monochrome (0 or 1).
 */
static unsigned char gray_to_mono(unsigned char gray) {
	return (gray > 127) ? 0 : 1; // 0 = white/background, 1 = black/foreground
}

static bool xbm_image_save(struct wlf_image *image, const char *filename) {
	FILE *fp = fopen(filename, "w");
	if (!fp) {
		wlf_log(WLF_ERROR, "Open %s failed!", filename);
		return false;
	}

	struct wlf_xbm_image *xbm_image = wlf_xbm_image_from_image(image);
	assert(xbm_image != NULL);

	// Convert image to monochrome if not already
	unsigned char *mono_data = NULL;
	if (image->format == WLF_COLOR_TYPE_GRAY) {
		// Already grayscale, just convert to monochrome
		mono_data = malloc(image->width * image->height);
		if (!mono_data) {
			wlf_log(WLF_ERROR, "Memory allocation failed!");
			fclose(fp);
			return false;
		}

		for (uint32_t i = 0; i < image->width * image->height; i++) {
			mono_data[i] = gray_to_mono(image->data[i]);
		}
	} else if (image->format == WLF_COLOR_TYPE_RGB) {
		// Convert RGB to grayscale then to monochrome
		mono_data = malloc(image->width * image->height);
		if (!mono_data) {
			wlf_log(WLF_ERROR, "Memory allocation failed!");
			fclose(fp);
			return false;
		}

		for (uint32_t i = 0; i < image->width * image->height; i++) {
			uint32_t rgb_offset = i * 3;
			// Convert to grayscale using standard weights
			unsigned char gray = (unsigned char)(
				0.299f * image->data[rgb_offset] +     // R
				0.587f * image->data[rgb_offset + 1] + // G
				0.114f * image->data[rgb_offset + 2]   // B
			);
			mono_data[i] = gray_to_mono(gray);
		}
	} else {
		wlf_log(WLF_ERROR, "Unsupported image format for XBM!");
		fclose(fp);
		return false;
	}

	// Determine the variable name
	char *name = xbm_image->name ? strdup(xbm_image->name) : extract_base_name(filename);
	if (!name) {
		name = strdup("image");
	}

	// Calculate the number of bytes per row (rounded up to nearest byte)
	uint32_t bytes_per_row = (image->width + 7) / 8;

	// Write XBM header
	fprintf(fp, "#define %s_width %u\n", name, image->width);
	fprintf(fp, "#define %s_height %u\n", name, image->height);

	// Write hotspot if present
	if (xbm_image->has_hotspot) {
		fprintf(fp, "#define %s_x_hot %d\n", name, xbm_image->hotspot.x);
		fprintf(fp, "#define %s_y_hot %d\n", name, xbm_image->hotspot.y);
	}

	fprintf(fp, "static unsigned char %s_bits[] = {\n", name);

	// Write bitmap data
	for (uint32_t y = 0; y < image->height; y++) {
		fprintf(fp, "  ");
		for (uint32_t byte_x = 0; byte_x < bytes_per_row; byte_x++) {
			unsigned char byte_val = 0;

			// Pack 8 pixels into one byte (LSB first)
			for (int bit = 0; bit < 8; bit++) {
				uint32_t x = byte_x * 8 + bit;
				if (x < image->width) {
					uint32_t pixel_idx = y * image->width + x;
					if (mono_data[pixel_idx]) {
						byte_val |= (1 << bit);
					}
				}
			}

			fprintf(fp, "0x%02x", byte_val);
			if (y < image->height - 1 || byte_x < bytes_per_row - 1) {
				fprintf(fp, ",");
			}
			if (byte_x < bytes_per_row - 1) {
				fprintf(fp, " ");
			}
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "};\n");

	free(mono_data);
	free(name);
	fclose(fp);
	return true;
}

static bool xbm_image_load(struct wlf_image *image, const char *filename, bool enable_16_bit) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		wlf_log(WLF_ERROR, "File %s cannot be opened!", filename);
		return false;
	}

	char line[1024];
	uint32_t width = 0, height = 0;
	int32_t x_hot = -1, y_hot = -1;
	char *name = NULL;
	bool found_width = false, found_height = false, found_data = false;

	// Parse header
	while (fgets(line, sizeof(line), fp)) {
		// Skip empty lines and comments
		char *trimmed = line;
		while (isspace(*trimmed)) trimmed++;
		if (*trimmed == '\0' || *trimmed == '/' || *trimmed == '*') {
			continue;
		}

		// Look for #define statements
		if (strncmp(trimmed, "#define", 7) == 0) {
			char def_name[256];
			int value;

			if (sscanf(trimmed, "#define %255s %d", def_name, &value) == 2) {
				char *suffix = strrchr(def_name, '_');
				if (suffix) {
					if (strcmp(suffix, "_width") == 0) {
						width = value;
						found_width = true;

						// Extract base name
						if (!name) {
							size_t base_len = suffix - def_name;
							name = malloc(base_len + 1);
							if (name) {
								strncpy(name, def_name, base_len);
								name[base_len] = '\0';
							}
						}
					} else if (strcmp(suffix, "_height") == 0) {
						height = value;
						found_height = true;
					} else if (strcmp(suffix, "_x_hot") == 0) {
						x_hot = value;
					} else if (strcmp(suffix, "_y_hot") == 0) {
						y_hot = value;
					}
				}
			}
		}

		// Look for data array
		if (strstr(trimmed, "_bits[]") && strstr(trimmed, "static")) {
			found_data = true;
			break;
		}
	}

	if (!found_width || !found_height || !found_data || width == 0 || height == 0) {
		wlf_log(WLF_ERROR, "Invalid XBM file format!");
		if (name) free(name);
		fclose(fp);
		return false;
	}

	// Allocate memory for grayscale pixel data
	size_t data_size = width * height;
	image->data = malloc(data_size);
	if (!image->data) {
		wlf_log(WLF_ERROR, "Memory allocation failed!");
		if (name) free(name);
		fclose(fp);
		return false;
	}

	// Read bitmap data
	uint32_t pixel_idx = 0;
	int hex_value;

	while (pixel_idx < width * height && fscanf(fp, " 0x%x", &hex_value) == 1) {
		unsigned char byte_val = (unsigned char)hex_value;

		// Unpack 8 pixels from one byte (LSB first)
		for (int bit = 0; bit < 8 && pixel_idx < width * height; bit++) {
			uint32_t x = pixel_idx % width;
			uint32_t y = pixel_idx / width;

			if (x < width && y < height) {
				// Convert bit to grayscale (0 = white, 1 = black)
				image->data[pixel_idx] = (byte_val & (1 << bit)) ? 0 : 255;
				pixel_idx++;
			}
		}

		// Skip comma or other separators
		int c;
		while ((c = fgetc(fp)) != EOF && c != '0' && !isdigit(c) && c != 'x') {
			if (c == '}') break; // End of array
		}
		if (c != EOF && c != '}') {
			ungetc(c, fp);
		}
	}

	// Set image properties
	image->width = width;
	image->height = height;
	image->format = WLF_COLOR_TYPE_GRAY;
	image->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	image->stride = width;
	image->has_alpha_channel = false;
	image->is_opaque = true;
	image->image_type = WLF_IMAGE_TYPE_XBM;

	// Set XBM-specific properties
	struct wlf_xbm_image *xbm_image = wlf_xbm_image_from_image(image);
	if (name) {
		wlf_xbm_image_set_name(xbm_image, name);
		free(name);
	}

	if (x_hot >= 0 && y_hot >= 0) {
		wlf_xbm_image_set_hotspot(xbm_image, x_hot, y_hot);
	}

	fclose(fp);
	return true;
}

static void xbm_image_destroy(struct wlf_image *wlf_image) {
	struct wlf_xbm_image *image = wlf_xbm_image_from_image(wlf_image);
	free(image->base.data);
	free(image->name);
}

static const struct wlf_image_impl xbm_image_impl = {
	.save = xbm_image_save,
	.load = xbm_image_load,
	.destroy = xbm_image_destroy,
};

struct wlf_xbm_image *wlf_xbm_image_create(void) {
	struct wlf_xbm_image *image = malloc(sizeof(struct wlf_xbm_image));
	if (image == NULL) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_xbm_image failed!");
		return NULL;
	}

	wlf_image_init(&image->base, &xbm_image_impl, 0, 0, 0);
	image->base.image_type = WLF_IMAGE_TYPE_XBM;
	image->name = NULL;
	image->hotspot.x = -1;
	image->hotspot.y = -1;
	image->has_hotspot = false;

	return image;
}

bool wlf_image_is_xbm(struct wlf_image *image) {
	if (!image) {
		return false;
	}

	return (image->impl == &xbm_image_impl &&
			image->image_type == WLF_IMAGE_TYPE_XBM);
}

struct wlf_xbm_image *wlf_xbm_image_from_image(struct wlf_image *wlf_image) {
	assert(wlf_image->impl == &xbm_image_impl);
	struct wlf_xbm_image *image = wlf_container_of(wlf_image, image, base);
	return image;
}

void wlf_xbm_image_set_name(struct wlf_xbm_image *image, const char *name) {
	if (image && name) {
		free(image->name);
		image->name = strdup(name);
	}
}

void wlf_xbm_image_set_hotspot(struct wlf_xbm_image *image, int32_t x, int32_t y) {
	if (image) {
		image->hotspot.x = x;
		image->hotspot.y = y;
		image->has_hotspot = true;
	}
}

void wlf_xbm_image_clear_hotspot(struct wlf_xbm_image *image) {
	if (image) {
		image->hotspot.x = -1;
		image->hotspot.y = -1;
		image->has_hotspot = false;
	}
}
