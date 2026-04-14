#include "wlf/image/wlf_xpm_image.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

struct wlf_xpm_color {
	char *key;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct wlf_x11_named_color {
	char *name;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

static struct wlf_x11_named_color *x11_named_colors;
static size_t x11_named_color_count;
static bool x11_named_colors_loaded;

static void trim_ascii(char *s) {
	if (s == NULL) {
		return;
	}

	char *start = s;
	while (*start != '\0' && isspace((unsigned char)*start)) {
		start++;
	}

	if (start != s) {
		memmove(s, start, strlen(start) + 1);
	}

	size_t len = strlen(s);
	while (len > 0 && isspace((unsigned char)s[len - 1])) {
		s[--len] = '\0';
	}
}

static bool normalize_color_name(const char *name, char *out, size_t out_size) {
	if (name == NULL || out == NULL || out_size == 0) {
		return false;
	}

	size_t j = 0;
	for (size_t i = 0; name[i] != '\0'; i++) {
		unsigned char c = (unsigned char)name[i];
		if (!isalnum(c)) {
			continue;
		}
		if (j + 1 >= out_size) {
			return false;
		}
		out[j++] = (char)tolower(c);
	}

	if (j == 0) {
		return false;
	}

	out[j] = '\0';
	return true;
}

static int compare_named_color(const void *a, const void *b) {
	const struct wlf_x11_named_color *ca = a;
	const struct wlf_x11_named_color *cb = b;
	return strcmp(ca->name, cb->name);
}

static void add_named_color(uint8_t r, uint8_t g, uint8_t b, const char *name) {
	char normalized[128];
	if (!normalize_color_name(name, normalized, sizeof(normalized))) {
		return;
	}

	for (size_t i = 0; i < x11_named_color_count; i++) {
		if (strcmp(x11_named_colors[i].name, normalized) == 0) {
			return;
		}
	}

	struct wlf_x11_named_color *tmp = realloc(
		x11_named_colors,
		(x11_named_color_count + 1) * sizeof(*x11_named_colors)
	);
	if (tmp == NULL) {
		return;
	}

	x11_named_colors = tmp;
	x11_named_colors[x11_named_color_count].name = strdup(normalized);
	if (x11_named_colors[x11_named_color_count].name == NULL) {
		return;
	}

	x11_named_colors[x11_named_color_count].r = r;
	x11_named_colors[x11_named_color_count].g = g;
	x11_named_colors[x11_named_color_count].b = b;
	x11_named_color_count++;
}

static void load_x11_named_colors(void) {
	if (x11_named_colors_loaded) {
		return;
	}
	x11_named_colors_loaded = true;

	static const char *paths[] = {
		"/usr/share/X11/rgb.txt",
		"/usr/X11R6/lib/X11/rgb.txt",
	};

	FILE *fp = NULL;
	for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
		fp = fopen(paths[i], "rb");
		if (fp != NULL) {
			break;
		}
	}

	if (fp == NULL) {
		return;
	}

	char line[512];
	while (fgets(line, sizeof(line), fp) != NULL) {
		trim_ascii(line);
		if (line[0] == '\0' || line[0] == '!' || line[0] == '#') {
			continue;
		}

		int r = -1;
		int g = -1;
		int b = -1;
		char name[256] = {0};
		if (sscanf(line, "%d %d %d %255[^\n]", &r, &g, &b, name) != 4) {
			continue;
		}

		trim_ascii(name);
		if (name[0] == '\0' || r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
			continue;
		}

		add_named_color((uint8_t)r, (uint8_t)g, (uint8_t)b, name);
	}

	fclose(fp);

	if (x11_named_color_count > 1) {
		qsort(x11_named_colors, x11_named_color_count, sizeof(*x11_named_colors), compare_named_color);
	}
}

static bool lookup_x11_named_color(const char *spec, uint8_t *r, uint8_t *g, uint8_t *b) {
	load_x11_named_colors();
	if (x11_named_color_count == 0) {
		return false;
	}

	char normalized[128];
	if (!normalize_color_name(spec, normalized, sizeof(normalized))) {
		return false;
	}

	struct wlf_x11_named_color key = {
		.name = normalized,
	};

	struct wlf_x11_named_color *result = bsearch(
		&key,
		x11_named_colors,
		x11_named_color_count,
		sizeof(*x11_named_colors),
		compare_named_color
	);
	if (result == NULL) {
		return false;
	}

	*r = result->r;
	*g = result->g;
	*b = result->b;
	return true;
}

static bool parse_hex_color(const char *spec, uint8_t *r, uint8_t *g, uint8_t *b) {
	if (spec == NULL || spec[0] != '#') {
		return false;
	}

	size_t len = strlen(spec);
	if (len == 4) {
		char rs[3] = { spec[1], spec[1], '\0' };
		char gs[3] = { spec[2], spec[2], '\0' };
		char bs[3] = { spec[3], spec[3], '\0' };
		*r = (uint8_t)strtoul(rs, NULL, 16);
		*g = (uint8_t)strtoul(gs, NULL, 16);
		*b = (uint8_t)strtoul(bs, NULL, 16);
		return true;
	}

	if (len == 7) {
		char rs[3] = { spec[1], spec[2], '\0' };
		char gs[3] = { spec[3], spec[4], '\0' };
		char bs[3] = { spec[5], spec[6], '\0' };
		*r = (uint8_t)strtoul(rs, NULL, 16);
		*g = (uint8_t)strtoul(gs, NULL, 16);
		*b = (uint8_t)strtoul(bs, NULL, 16);
		return true;
	}

	if (len == 13) {
		char rs[5] = { spec[1], spec[2], spec[3], spec[4], '\0' };
		char gs[5] = { spec[5], spec[6], spec[7], spec[8], '\0' };
		char bs[5] = { spec[9], spec[10], spec[11], spec[12], '\0' };
		*r = (uint8_t)(strtoul(rs, NULL, 16) >> 8);
		*g = (uint8_t)(strtoul(gs, NULL, 16) >> 8);
		*b = (uint8_t)(strtoul(bs, NULL, 16) >> 8);
		return true;
	}

	return false;
}

static bool parse_named_color(const char *spec, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a) {
	if (strcasecmp(spec, "none") == 0 || strcasecmp(spec, "transparent") == 0) {
		*r = 0;
		*g = 0;
		*b = 0;
		*a = 0;
		return true;
	}

	if (lookup_x11_named_color(spec, r, g, b)) {
		*a = 255;
		return true;
	}

	if (strcasecmp(spec, "black") == 0) {
		*r = 0;
		*g = 0;
		*b = 0;
		*a = 255;
		return true;
	}
	if (strcasecmp(spec, "white") == 0) {
		*r = 255;
		*g = 255;
		*b = 255;
		*a = 255;
		return true;
	}
	if (strcasecmp(spec, "red") == 0) {
		*r = 255;
		*g = 0;
		*b = 0;
		*a = 255;
		return true;
	}
	if (strcasecmp(spec, "green") == 0) {
		*r = 0;
		*g = 255;
		*b = 0;
		*a = 255;
		return true;
	}
	if (strcasecmp(spec, "blue") == 0) {
		*r = 0;
		*g = 0;
		*b = 255;
		*a = 255;
		return true;
	}
	if (strcasecmp(spec, "yellow") == 0) {
		*r = 255;
		*g = 255;
		*b = 0;
		*a = 255;
		return true;
	}
	if (strcasecmp(spec, "cyan") == 0) {
		*r = 0;
		*g = 255;
		*b = 255;
		*a = 255;
		return true;
	}
	if (strcasecmp(spec, "magenta") == 0) {
		*r = 255;
		*g = 0;
		*b = 255;
		*a = 255;
		return true;
	}
	if (strcasecmp(spec, "gray") == 0 || strcasecmp(spec, "grey") == 0) {
		*r = 128;
		*g = 128;
		*b = 128;
		*a = 255;
		return true;
	}
	return false;
}

static char *read_file_all(const char *filename, size_t *size_out) {
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		wlf_log_errno(WLF_ERROR, "Open %s failed", filename);
		return NULL;
	}
	if (fseek(fp, 0, SEEK_END) != 0) {
		fclose(fp);
		return NULL;
	}
	long fsize = ftell(fp);
	if (fsize < 0) {
		fclose(fp);
		return NULL;
	}
	rewind(fp);
	char *buf = malloc((size_t)fsize + 1);
	if (buf == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate file buf");
		fclose(fp);
		return NULL;
	}
	size_t n = fread(buf, 1, (size_t)fsize, fp);
	fclose(fp);
	if (n != (size_t)fsize) {
		free(buf);
		return NULL;
	}
	buf[n] = '\0';
	if (size_out) {
		*size_out = n;
	}
	return buf;
}

static char **extract_quoted_strings(const char *buf, size_t *count_out) {
	size_t cap = 64;
	size_t count = 0;
	char **arr = malloc(cap * sizeof(char *));
	if (arr == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate char");
		return NULL;
	}

	for (const char *p = buf; *p != '\0'; p++) {
		if (*p != '"') {
			continue;
		}
		p++;
		const char *start = p;
		size_t len = 0;
		while (*p != '\0') {
			if (*p == '"' && (p == start || p[-1] != '\\')) {
				break;
			}
			p++;
			len++;
		}
		if (*p != '"') {
			break;
		}

		char *s = malloc(len + 1);
		if (s == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate char");
			for (size_t i = 0; i < count; i++) {
				free(arr[i]);
			}
			free(arr);
			return NULL;
		}

		size_t out = 0;
		for (size_t i = 0; i < len; i++) {
			char c = start[i];
			if (c == '\\' && i + 1 < len) {
				char n = start[i + 1];
				if (n == '"' || n == '\\') {
					s[out++] = n;
					i++;
					continue;
				}
			}
			s[out++] = c;
		}
		s[out] = '\0';

		if (count == cap) {
			cap *= 2;
			char **tmp = realloc(arr, cap * sizeof(char *));
			if (tmp == NULL) {
				free(s);
				for (size_t i = 0; i < count; i++) {
					free(arr[i]);
				}
				free(arr);
				return NULL;
			}
			arr = tmp;
		}
		arr[count++] = s;
	}

	*count_out = count;
	return arr;
}

static void free_string_list(char **arr, size_t count) {
	if (arr == NULL) {
		return;
	}
	for (size_t i = 0; i < count; i++) {
		free(arr[i]);
	}
	free(arr);
}

static bool parse_color_line(const char *line, int cpp, struct wlf_xpm_color *out) {
	if ((int)strlen(line) < cpp) {
		return false;
	}

	out->key = strndup(line, (size_t)cpp);
	if (out->key == NULL) {
		return false;
	}

	const char *p = line + cpp;
	const char *color_spec = NULL;
	while (*p) {
		if (*p == 'c' && (p == line + cpp || isspace((unsigned char)p[-1]))) {
			const char *q = p + 1;
			if (*q == '\0' || isspace((unsigned char)*q)) {
				while (*q && isspace((unsigned char)*q)) {
					q++;
				}
				color_spec = q;
				break;
			}
		}
		p++;
	}

	if (color_spec == NULL || *color_spec == '\0') {
		free(out->key);
		out->key = NULL;
		return false;
	}

	char token[256];
	size_t spec_len = strlen(color_spec);
	if (spec_len >= sizeof(token)) {
		spec_len = sizeof(token) - 1;
	}
	memcpy(token, color_spec, spec_len);
	token[spec_len] = '\0';
	trim_ascii(token);
	if (token[0] == '\0') {
		free(out->key);
		out->key = NULL;
		return false;
	}

	out->a = 255;
	if (parse_hex_color(token, &out->r, &out->g, &out->b)) {
		return true;
	}
	if (parse_named_color(token, &out->r, &out->g, &out->b, &out->a)) {
		return true;
	}

	free(out->key);
	out->key = NULL;
	return false;
}

static bool xpm_image_load(struct wlf_image *image, const char *filename, bool enable_16_bit) {
	(void)enable_16_bit;

	size_t file_size = 0;
	char *buf = read_file_all(filename, &file_size);
	if (buf == NULL || file_size == 0) {
		free(buf);
		return false;
	}

	size_t line_count = 0;
	char **lines = extract_quoted_strings(buf, &line_count);
	free(buf);
	if (lines == NULL || line_count == 0) {
		free_string_list(lines, line_count);
		wlf_log(WLF_ERROR, "Invalid XPM content: %s", filename);
		return false;
	}

	int width = 0;
	int height = 0;
	int color_count = 0;
	int cpp = 0;
	if (sscanf(lines[0], "%d %d %d %d", &width, &height, &color_count, &cpp) != 4 ||
		width <= 0 || height <= 0 || color_count <= 0 || cpp <= 0) {
		free_string_list(lines, line_count);
		wlf_log(WLF_ERROR, "Invalid XPM header: %s", filename);
		return false;
	}

	if ((size_t)(1 + color_count + height) > line_count) {
		free_string_list(lines, line_count);
		wlf_log(WLF_ERROR, "Incomplete XPM data: %s", filename);
		return false;
	}

	struct wlf_xpm_color *colors = calloc((size_t)color_count, sizeof(*colors));
	if (colors == NULL) {
		free_string_list(lines, line_count);
		return false;
	}

	for (int i = 0; i < color_count; i++) {
		if (!parse_color_line(lines[1 + i], cpp, &colors[i])) {
			for (int j = 0; j < i; j++) {
				free(colors[j].key);
			}
			free(colors);
			free_string_list(lines, line_count);
			wlf_log(WLF_ERROR, "Invalid XPM color table line %d", i);
			return false;
		}
	}

	bool has_alpha = false;
	for (int i = 0; i < color_count; i++) {
		if (colors[i].a < 255) {
			has_alpha = true;
			break;
		}
	}

	int channels = has_alpha ? 4 : 3;
	size_t data_size = (size_t)width * height * channels;
	unsigned char *data = malloc(data_size);
	if (data == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate unsigned char");
		for (int i = 0; i < color_count; i++) {
			free(colors[i].key);
		}
		free(colors);
		free_string_list(lines, line_count);
		return false;
	}

	for (int y = 0; y < height; y++) {
		const char *px = lines[1 + color_count + y];
		if ((int)strlen(px) < width * cpp) {
			free(data);
			for (int i = 0; i < color_count; i++) {
				free(colors[i].key);
			}
			free(colors);
			free_string_list(lines, line_count);
			wlf_log(WLF_ERROR, "Invalid XPM pixel row %d", y);
			return false;
		}

		for (int x = 0; x < width; x++) {
			const char *key = px + x * cpp;
			int found = -1;
			for (int c = 0; c < color_count; c++) {
				if (strncmp(colors[c].key, key, (size_t)cpp) == 0) {
					found = c;
					break;
				}
			}
			if (found < 0) {
				free(data);
				for (int i = 0; i < color_count; i++) {
					free(colors[i].key);
				}
				free(colors);
				free_string_list(lines, line_count);
				wlf_log(WLF_ERROR, "Unknown XPM color key in row %d", y);
				return false;
			}

			size_t off = ((size_t)y * (size_t)width + (size_t)x) * (size_t)channels;
			data[off + 0] = colors[found].r;
			data[off + 1] = colors[found].g;
			data[off + 2] = colors[found].b;
			if (channels == 4) {
				data[off + 3] = colors[found].a;
			}
		}
	}

	for (int i = 0; i < color_count; i++) {
		free(colors[i].key);
	}
	free(colors);
	free_string_list(lines, line_count);

	image->width = (uint32_t)width;
	image->height = (uint32_t)height;
	image->data = data;
	image->format = has_alpha ? WLF_COLOR_TYPE_RGBA : WLF_COLOR_TYPE_RGB;
	image->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	image->stride = (uint32_t)width * (uint32_t)channels;
	image->has_alpha_channel = has_alpha;
	image->is_opaque = !has_alpha;

	return true;
}

static const char xpm_key_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static int required_cpp(size_t color_count) {
	size_t base = sizeof(xpm_key_chars) - 1;
	size_t cap = 1;
	int cpp = 0;
	while (cap < color_count) {
		if (cpp >= 6) {
			return -1;
		}
		cap *= base;
		cpp++;
	}
	if (cpp == 0) {
		cpp = 1;
	}
	return cpp;
}

static void color_key_from_index(size_t idx, int cpp, char *out) {
	size_t base = sizeof(xpm_key_chars) - 1;
	for (int i = cpp - 1; i >= 0; i--) {
		out[i] = xpm_key_chars[idx % base];
		idx /= base;
	}
	out[cpp] = '\0';
}

static bool xpm_image_save(struct wlf_image *image, const char *filename) {
	if (image == NULL || filename == NULL || image->data == NULL) {
		return false;
	}

	if (image->format != WLF_COLOR_TYPE_RGB && image->format != WLF_COLOR_TYPE_RGBA &&
		image->format != WLF_COLOR_TYPE_GRAY && image->format != WLF_COLOR_TYPE_GRAY_ALPHA) {
		wlf_log(WLF_ERROR, "Unsupported format for XPM save: %d", image->format);
		return false;
	}

	if (image->bit_depth != 0 && image->bit_depth != WLF_IMAGE_BIT_DEPTH_8) {
		wlf_log(WLF_ERROR, "XPM only supports 8-bit channels");
		return false;
	}

	size_t pixel_count = (size_t)image->width * image->height;
	uint32_t *palette = malloc(pixel_count * sizeof(uint32_t));
	if (palette == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate uint32_t");
		return false;
	}
	int channels = wlf_image_get_channels(image);
	size_t palette_count = 0;

	for (uint32_t y = 0; y < image->height; y++) {
		const unsigned char *row = image->data + (size_t)y * image->stride;
		for (uint32_t x = 0; x < image->width; x++) {
			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;
			uint8_t a = 255;

			switch (image->format) {
				case WLF_COLOR_TYPE_RGB:
					r = row[x * channels + 0];
					g = row[x * channels + 1];
					b = row[x * channels + 2];
					break;
				case WLF_COLOR_TYPE_RGBA:
					r = row[x * channels + 0];
					g = row[x * channels + 1];
					b = row[x * channels + 2];
					a = row[x * channels + 3];
					break;
				case WLF_COLOR_TYPE_GRAY:
					r = row[x * channels + 0];
					g = row[x * channels + 0];
					b = row[x * channels + 0];
					break;
				case WLF_COLOR_TYPE_GRAY_ALPHA:
					r = row[x * channels + 0];
					g = row[x * channels + 0];
					b = row[x * channels + 0];
					a = row[x * channels + 1];
					break;
				default:
					break;
			}

			uint32_t rgba = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (uint32_t)a;
			bool exists = false;
			for (size_t i = 0; i < palette_count; i++) {
				if (palette[i] == rgba) {
					exists = true;
					break;
				}
			}
			if (!exists) {
				palette[palette_count++] = rgba;
			}
		}
	}

	int cpp = required_cpp(palette_count);
	if (cpp < 0) {
		free(palette);
		wlf_log(WLF_ERROR, "Too many colors for XPM output");
		return false;
	}

	char **keys = malloc(palette_count * sizeof(char *));
	if (keys == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate char");
		free(palette);
		return false;
	}
	for (size_t i = 0; i < palette_count; i++) {
		keys[i] = malloc((size_t)cpp + 1);
		if (keys[i] == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate size_t");
			for (size_t j = 0; j < i; j++) {
				free(keys[j]);
			}
			free(keys);
			free(palette);
			return false;
		}
		color_key_from_index(i, cpp, keys[i]);
	}

	FILE *fp = fopen(filename, "wb");
	if (fp == NULL) {
		wlf_log_errno(WLF_ERROR, "Open %s failed", filename);
		for (size_t i = 0; i < palette_count; i++) {
			free(keys[i]);
		}
		free(keys);
		free(palette);
		return false;
	}

	fprintf(fp, "/* XPM */\n");
	fprintf(fp, "static char *wlf_xpm[] = {\n");
	fprintf(fp, "\"%u %u %zu %d\",\n", image->width, image->height, palette_count, cpp);

	for (size_t i = 0; i < palette_count; i++) {
		uint8_t r = (uint8_t)((palette[i] >> 24) & 0xFF);
		uint8_t g = (uint8_t)((palette[i] >> 16) & 0xFF);
		uint8_t b = (uint8_t)((palette[i] >> 8) & 0xFF);
		uint8_t a = (uint8_t)(palette[i] & 0xFF);
		if (a < 128) {
			fprintf(fp, "\"%s c None\",\n", keys[i]);
		} else {
			fprintf(fp, "\"%s c #%02X%02X%02X\",\n", keys[i], r, g, b);
		}
	}

	for (uint32_t y = 0; y < image->height; y++) {
		fputc('"', fp);
		const unsigned char *row = image->data + (size_t)y * image->stride;
		for (uint32_t x = 0; x < image->width; x++) {
			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;
			uint8_t a = 255;

			switch (image->format) {
				case WLF_COLOR_TYPE_RGB:
					r = row[x * channels + 0];
					g = row[x * channels + 1];
					b = row[x * channels + 2];
					break;
				case WLF_COLOR_TYPE_RGBA:
					r = row[x * channels + 0];
					g = row[x * channels + 1];
					b = row[x * channels + 2];
					a = row[x * channels + 3];
					break;
				case WLF_COLOR_TYPE_GRAY:
					r = row[x * channels + 0];
					g = row[x * channels + 0];
					b = row[x * channels + 0];
					break;
				case WLF_COLOR_TYPE_GRAY_ALPHA:
					r = row[x * channels + 0];
					g = row[x * channels + 0];
					b = row[x * channels + 0];
					a = row[x * channels + 1];
					break;
				default:
					break;
			}

			uint32_t rgba = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | (uint32_t)a;
			size_t idx = 0;
			for (; idx < palette_count; idx++) {
				if (palette[idx] == rgba) {
					break;
				}
			}
			if (idx >= palette_count) {
				idx = 0;
			}
			fputs(keys[idx], fp);
		}
		if (y + 1 < image->height) {
			fprintf(fp, "\",\n");
		} else {
			fprintf(fp, "\"\n");
		}
	}
	fprintf(fp, "};\n");
	fclose(fp);

	for (size_t i = 0; i < palette_count; i++) {
		free(keys[i]);
	}
	free(keys);
	free(palette);
	return true;
}

static void xpm_image_destroy(struct wlf_image *wlf_image) {
	struct wlf_xpm_image *image = wlf_xpm_image_from_image(wlf_image);
	free(image->base.data);
	image->base.data = NULL;
}

static const struct wlf_image_impl xpm_image_impl = {
	.save = xpm_image_save,
	.load = xpm_image_load,
	.destroy = xpm_image_destroy,
};

struct wlf_xpm_image *wlf_xpm_image_create(void) {
	struct wlf_xpm_image *image = malloc(sizeof(struct wlf_xpm_image));
	if (image == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_xpm_image");
		return NULL;
	}

	wlf_image_init(&image->base, &xpm_image_impl, 0, 0, 0);
	return image;
}

bool wlf_image_is_xpm(const struct wlf_image *image) {
	return image->impl == &xpm_image_impl;
}

struct wlf_xpm_image *wlf_xpm_image_from_image(struct wlf_image *wlf_image) {
	assert(wlf_image->impl == &xpm_image_impl);

	struct wlf_xpm_image *image = wlf_container_of(wlf_image, image, base);

	return image;
}
