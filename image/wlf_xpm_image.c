#include "wlf/image/wlf_xpm_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <strings.h>

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
 * @brief Convert RGB values to a character key for small palettes.
 */
static char rgb_to_char(uint8_t r, uint8_t g, uint8_t b, int index) {
	// Use printable ASCII characters, avoiding quotes and backslashes
	const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-.";
	const int charset_len = sizeof(charset) - 1;

	if (index < charset_len) {
		return charset[index];
	}

	// Fallback: use intensity-based character
	int intensity = (r + g + b) / 3;
	return charset[intensity * charset_len / 255];
}

/**
 * @brief Parse a hex color string (e.g., "#FF0000" or "red").
 */
static bool parse_color_string(const char *color_str, uint8_t *r, uint8_t *g, uint8_t *b) {
	if (!color_str || !r || !g || !b) {
		return false;
	}

	// Handle hex colors like #RRGGBB
	if (color_str[0] == '#' && strlen(color_str) == 7) {
		unsigned int rgb;
		if (sscanf(color_str + 1, "%6x", &rgb) == 1) {
			*r = (rgb >> 16) & 0xFF;
			*g = (rgb >> 8) & 0xFF;
			*b = rgb & 0xFF;
			return true;
		}
	}

	// Handle named colors (simplified set)
	struct { const char *name; uint8_t r, g, b; } colors[] = {
		{"black", 0, 0, 0},
		{"white", 255, 255, 255},
		{"red", 255, 0, 0},
		{"green", 0, 255, 0},
		{"blue", 0, 0, 255},
		{"yellow", 255, 255, 0},
		{"cyan", 0, 255, 255},
		{"magenta", 255, 0, 255},
		{"gray", 128, 128, 128},
		{"grey", 128, 128, 128},
		{NULL, 0, 0, 0}
	};

	for (int i = 0; colors[i].name; i++) {
		if (strcasecmp(color_str, colors[i].name) == 0) {
			*r = colors[i].r;
			*g = colors[i].g;
			*b = colors[i].b;
			return true;
		}
	}

	return false;
}

/**
 * @brief Create a simple color palette from RGB image data.
 */
static bool create_palette_from_rgb(struct wlf_xpm_image *xpm_image, const unsigned char *rgb_data,
                                   uint32_t width, uint32_t height) {
	// For simplicity, limit to a maximum of 64 colors
	const uint32_t max_colors = 64;
	uint8_t palette[max_colors * 3]; // RGB values
	uint32_t color_count = 0;

	// Scan image and collect unique colors
	for (uint32_t i = 0; i < width * height * 3; i += 3) {
		uint8_t r = rgb_data[i];
		uint8_t g = rgb_data[i + 1];
		uint8_t b = rgb_data[i + 2];

		// Check if color already exists in palette
		bool found = false;
		for (uint32_t j = 0; j < color_count; j++) {
			if (palette[j * 3] == r && palette[j * 3 + 1] == g && palette[j * 3 + 2] == b) {
				found = true;
				break;
			}
		}

		// Add new color if not found and we have space
		if (!found && color_count < max_colors) {
			palette[color_count * 3] = r;
			palette[color_count * 3 + 1] = g;
			palette[color_count * 3 + 2] = b;
			color_count++;
		}

		// Stop if we've reached the maximum
		if (color_count >= max_colors) {
			break;
		}
	}

	// Create color palette
	xpm_image->colors = malloc(color_count * sizeof(struct wlf_xpm_color));
	if (!xpm_image->colors) {
		return false;
	}

	xpm_image->num_colors = color_count;

	for (uint32_t i = 0; i < color_count; i++) {
		xpm_image->colors[i].key = rgb_to_char(palette[i * 3], palette[i * 3 + 1], palette[i * 3 + 2], i);
		xpm_image->colors[i].r = palette[i * 3];
		xpm_image->colors[i].g = palette[i * 3 + 1];
		xpm_image->colors[i].b = palette[i * 3 + 2];
		xpm_image->colors[i].name = NULL; // No named colors for auto-generated palette
	}

	return true;
}

static bool xpm_image_save(struct wlf_image *image, const char *filename) {
	FILE *fp = fopen(filename, "w");
	if (!fp) {
		wlf_log(WLF_ERROR, "Open %s failed!", filename);
		return false;
	}

	struct wlf_xpm_image *xpm_image = wlf_xpm_image_from_image(image);
	assert(xpm_image != NULL);

	// Only support RGB format for now
	if (image->format != WLF_COLOR_TYPE_RGB) {
		wlf_log(WLF_ERROR, "XPM format currently only supports RGB images!");
		fclose(fp);
		return false;
	}

	// Create palette if not already exists
	if (!xpm_image->colors) {
		if (!create_palette_from_rgb(xpm_image, image->data, image->width, image->height)) {
			wlf_log(WLF_ERROR, "Failed to create color palette!");
			fclose(fp);
			return false;
		}
	}

	// Determine the variable name
	char *name = xpm_image->name ? strdup(xpm_image->name) : extract_base_name(filename);
	if (!name) {
		name = strdup("image");
	}

	// Write XPM header
	fprintf(fp, "/* XPM */\n");
	fprintf(fp, "static char *%s[] = {\n", name);

	// Write values line: "width height num_colors chars_per_pixel [x_hot y_hot]"
	if (xpm_image->has_hotspot) {
		fprintf(fp, "\"%u %u %u %u %d %d\",\n",
			image->width, image->height, xpm_image->num_colors,
			xpm_image->colors_per_pixel, xpm_image->hotspot.x, xpm_image->hotspot.y);
	} else {
		fprintf(fp, "\"%u %u %u %u\",\n",
			image->width, image->height, xpm_image->num_colors, xpm_image->colors_per_pixel);
	}

	// Write color definitions
	for (uint32_t i = 0; i < xpm_image->num_colors; i++) {
		fprintf(fp, "\"%c c #%02X%02X%02X\",\n",
			xpm_image->colors[i].key,
			xpm_image->colors[i].r,
			xpm_image->colors[i].g,
			xpm_image->colors[i].b);
	}

	// Write pixel data
	for (uint32_t y = 0; y < image->height; y++) {
		fprintf(fp, "\"");
		for (uint32_t x = 0; x < image->width; x++) {
			uint32_t rgb_offset = (y * image->width + x) * 3;
			uint8_t r = image->data[rgb_offset];
			uint8_t g = image->data[rgb_offset + 1];
			uint8_t b = image->data[rgb_offset + 2];

			// Find matching color in palette
			char pixel_char = '?'; // Default for unknown colors
			for (uint32_t i = 0; i < xpm_image->num_colors; i++) {
				if (xpm_image->colors[i].r == r &&
				    xpm_image->colors[i].g == g &&
				    xpm_image->colors[i].b == b) {
					pixel_char = xpm_image->colors[i].key;
					break;
				}
			}

			fprintf(fp, "%c", pixel_char);
		}
		fprintf(fp, "\"%s\n", (y < image->height - 1) ? "," : "");
	}

	fprintf(fp, "};\n");

	free(name);
	fclose(fp);
	return true;
}

static bool xpm_image_load(struct wlf_image *image, const char *filename, bool enable_16_bit) {
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		wlf_log(WLF_ERROR, "File %s cannot be opened!", filename);
		return false;
	}

	char line[1024];
	uint32_t width = 0, height = 0, num_colors = 0, chars_per_pixel = 1;
	int32_t x_hot = -1, y_hot = -1;
	char *name = NULL;
	struct wlf_xpm_color *colors = NULL;
	bool found_header = false;

	// Skip to the array start
	while (fgets(line, sizeof(line), fp)) {
		if (strstr(line, "[] = {") || strstr(line, "[]={")) {
			// Extract variable name
			char *bracket = strstr(line, "[]");
			if (bracket) {
				// Find start of variable name
				char *start = bracket - 1;
				while (start > line && (isalnum(*start) || *start == '_')) {
					start--;
				}
				if (start != bracket - 1) {
					start++; // Move to first character of name
					size_t name_len = bracket - start;
					name = malloc(name_len + 1);
					if (name) {
						strncpy(name, start, name_len);
						name[name_len] = '\0';
					}
				}
			}
			break;
		}
	}

	// Read values line
	while (fgets(line, sizeof(line), fp)) {
		char *start = strchr(line, '"');
		char *end = strrchr(line, '"');
		if (start && end && start != end) {
			start++; // Skip opening quote
			*end = '\0'; // Terminate at closing quote

			int values = sscanf(start, "%u %u %u %u %d %d",
				&width, &height, &num_colors, &chars_per_pixel, &x_hot, &y_hot);
			if (values >= 4) {
				found_header = true;
				if (values < 6) {
					x_hot = y_hot = -1; // No hotspot
				}
				break;
			}
		}
	}

	if (!found_header || width == 0 || height == 0 || num_colors == 0) {
		wlf_log(WLF_ERROR, "Invalid XPM header!");
		if (name) free(name);
		fclose(fp);
		return false;
	}

	// Only support single character per pixel for now
	if (chars_per_pixel != 1) {
		wlf_log(WLF_ERROR, "Only single character per pixel XPM files are supported!");
		if (name) free(name);
		fclose(fp);
		return false;
	}

	// Read color definitions
	colors = malloc(num_colors * sizeof(struct wlf_xpm_color));
	if (!colors) {
		wlf_log(WLF_ERROR, "Memory allocation failed!");
		if (name) free(name);
		fclose(fp);
		return false;
	}

	uint32_t color_index = 0;
	while (color_index < num_colors && fgets(line, sizeof(line), fp)) {
		char *start = strchr(line, '"');
		char *end = strrchr(line, '"');
		if (start && end && start != end) {
			start++; // Skip opening quote
			*end = '\0'; // Terminate at closing quote

			// Parse: "char c color_value"
			if (strlen(start) >= 5 && start[1] == ' ' && start[2] == 'c' && start[3] == ' ') {
				colors[color_index].key = start[0];
				colors[color_index].name = NULL;

				// Parse color value
				char *color_str = start + 4;
				uint8_t r, g, b;
				if (parse_color_string(color_str, &r, &g, &b)) {
					colors[color_index].r = r;
					colors[color_index].g = g;
					colors[color_index].b = b;
					color_index++;
				}
			}
		}
	}

	if (color_index != num_colors) {
		wlf_log(WLF_ERROR, "Failed to read all color definitions!");
		free(colors);
		if (name) free(name);
		fclose(fp);
		return false;
	}

	// Allocate memory for RGB pixel data
	size_t data_size = width * height * 3;
	image->data = malloc(data_size);
	if (!image->data) {
		wlf_log(WLF_ERROR, "Memory allocation failed!");
		free(colors);
		if (name) free(name);
		fclose(fp);
		return false;
	}

	// Read pixel data
	uint32_t pixel_row = 0;
	while (pixel_row < height && fgets(line, sizeof(line), fp)) {
		char *start = strchr(line, '"');
		char *end = strrchr(line, '"');
		if (start && end && start != end) {
			start++; // Skip opening quote
			*end = '\0'; // Terminate at closing quote

			if (strlen(start) >= width) {
				for (uint32_t x = 0; x < width; x++) {
					char pixel_char = start[x];

					// Find color for this character
					bool found_color = false;
					for (uint32_t c = 0; c < num_colors; c++) {
						if (colors[c].key == pixel_char) {
							uint32_t rgb_offset = (pixel_row * width + x) * 3;
							image->data[rgb_offset] = colors[c].r;
							image->data[rgb_offset + 1] = colors[c].g;
							image->data[rgb_offset + 2] = colors[c].b;
							found_color = true;
							break;
						}
					}

					if (!found_color) {
						// Default to black for unknown characters
						uint32_t rgb_offset = (pixel_row * width + x) * 3;
						image->data[rgb_offset] = 0;
						image->data[rgb_offset + 1] = 0;
						image->data[rgb_offset + 2] = 0;
					}
				}
				pixel_row++;
			}
		}
	}

	// Set image properties
	image->width = width;
	image->height = height;
	image->format = WLF_COLOR_TYPE_RGB;
	image->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	image->stride = width * 3;
	image->has_alpha_channel = false;
	image->is_opaque = true;
	image->image_type = WLF_IMAGE_TYPE_XPM;

	// Set XPM-specific properties
	struct wlf_xpm_image *xpm_image = wlf_xpm_image_from_image(image);
	if (name) {
		wlf_xpm_image_set_name(xpm_image, name);
		free(name);
	}

	if (x_hot >= 0 && y_hot >= 0) {
		wlf_xpm_image_set_hotspot(xpm_image, x_hot, y_hot);
	}

	xpm_image->colors_per_pixel = chars_per_pixel;
	xpm_image->num_colors = num_colors;
	xpm_image->colors = colors;

	fclose(fp);
	return true;
}

static void xpm_image_destroy(struct wlf_image *wlf_image) {
	struct wlf_xpm_image *image = wlf_xpm_image_from_image(wlf_image);
	free(image->base.data);
	free(image->name);

	if (image->colors) {
		for (uint32_t i = 0; i < image->num_colors; i++) {
			free(image->colors[i].name);
		}
		free(image->colors);
	}
}

static const struct wlf_image_impl xpm_image_impl = {
	.save = xpm_image_save,
	.load = xpm_image_load,
	.destroy = xpm_image_destroy,
};

struct wlf_xpm_image *wlf_xpm_image_create(void) {
	struct wlf_xpm_image *image = malloc(sizeof(struct wlf_xpm_image));
	if (image == NULL) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_xpm_image failed!");
		return NULL;
	}

	wlf_image_init(&image->base, &xpm_image_impl, 0, 0, 0);
	image->base.image_type = WLF_IMAGE_TYPE_XPM;
	image->name = NULL;
	image->hotspot.x = -1;
	image->hotspot.y = -1;
	image->has_hotspot = false;
	image->colors_per_pixel = 1;
	image->num_colors = 0;
	image->colors = NULL;

	return image;
}

bool wlf_image_is_xpm(struct wlf_image *image) {
	if (!image) {
		return false;
	}

	return (image->impl == &xpm_image_impl &&
			image->image_type == WLF_IMAGE_TYPE_XPM);
}

struct wlf_xpm_image *wlf_xpm_image_from_image(struct wlf_image *wlf_image) {
	assert(wlf_image->impl == &xpm_image_impl);
	struct wlf_xpm_image *image = wlf_container_of(wlf_image, image, base);
	return image;
}

void wlf_xpm_image_set_name(struct wlf_xpm_image *image, const char *name) {
	if (image && name) {
		free(image->name);
		image->name = strdup(name);
	}
}

void wlf_xpm_image_set_hotspot(struct wlf_xpm_image *image, int32_t x, int32_t y) {
	if (image) {
		image->hotspot.x = x;
		image->hotspot.y = y;
		image->has_hotspot = true;
	}
}

void wlf_xpm_image_clear_hotspot(struct wlf_xpm_image *image) {
	if (image) {
		image->hotspot.x = -1;
		image->hotspot.y = -1;
		image->has_hotspot = false;
	}
}

void wlf_xpm_image_set_colors_per_pixel(struct wlf_xpm_image *image, uint32_t cpp) {
	if (image) {
		image->colors_per_pixel = cpp;
	}
}
