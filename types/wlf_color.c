#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_log.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WLF_COLOR_STRLEN 256

struct wlf_color wlf_color_make(double r, double g, double b, double a) {
	struct wlf_color color = {
		.r = r,
		.g = g,
		.b = b,
		.a = a};
	return color;
}

struct wlf_color wlf_color_make_rgb(double r, double g, double b) {
	return wlf_color_make(r, g, b, 1.0);
}

struct wlf_color wlf_color_from_rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return wlf_color_make(
		r / 255.0,
		g / 255.0,
		b / 255.0,
		a / 255.0
	);
}

struct wlf_color wlf_color_from_rgb8(uint8_t r, uint8_t g, uint8_t b) {
	return wlf_color_from_rgba8(r, g, b, 255);
}

struct wlf_color wlf_color_from_hex(uint32_t hex) {
	uint8_t r = (hex >> 24) & 0xFF;
	uint8_t g = (hex >> 16) & 0xFF;
	uint8_t b = (hex >> 8) & 0xFF;
	uint8_t a = hex & 0xFF;
	return wlf_color_from_rgba8(r, g, b, a);
}

struct wlf_color wlf_color_from_hex_rgb(uint32_t hex) {
	uint8_t r = (hex >> 16) & 0xFF;
	uint8_t g = (hex >> 8) & 0xFF;
	uint8_t b = hex & 0xFF;
	return wlf_color_from_rgb8(r, g, b);
}

char* wlf_color_to_str_prec(const struct wlf_color *color, uint8_t precision) {
	if (color == NULL) {
		return strdup("(NULL)");
	}

	if (precision > 15) {
		precision = 15;
	}

	char fmt[64];
	snprintf(fmt, sizeof(fmt), "(%%.%df, %%.%df, %%.%df, %%.%df)", precision, precision, precision, precision);

	char *buffer = malloc(WLF_COLOR_STRLEN);
	if (buffer == NULL) {
		wlf_log(WLF_ERROR, "Memory allocation failed for wlf_color_to_str_prec");
		return NULL;
	}

	snprintf(buffer, WLF_COLOR_STRLEN, fmt, color->r, color->g, color->b, color->a);

	return buffer;
}

char* wlf_color_to_str(const struct wlf_color *color) {
	return wlf_color_to_str_prec(color, 3);
}

uint32_t wlf_color_to_hex(const struct wlf_color *color) {
	if (color == NULL) {
		return 0;
	}

	uint8_t r = (uint8_t)(fmax(0.0, fmin(1.0, color->r)) * 255.0);
	uint8_t g = (uint8_t)(fmax(0.0, fmin(1.0, color->g)) * 255.0);
	uint8_t b = (uint8_t)(fmax(0.0, fmin(1.0, color->b)) * 255.0);
	uint8_t a = (uint8_t)(fmax(0.0, fmin(1.0, color->a)) * 255.0);

	return (r << 24) | (g << 16) | (b << 8) | a;
}

uint32_t wlf_color_to_hex_rgb(const struct wlf_color *color) {
	if (color == NULL) {
		return 0;
	}

	uint8_t r = (uint8_t)(fmax(0.0, fmin(1.0, color->r)) * 255.0);
	uint8_t g = (uint8_t)(fmax(0.0, fmin(1.0, color->g)) * 255.0);
	uint8_t b = (uint8_t)(fmax(0.0, fmin(1.0, color->b)) * 255.0);

	return (r << 16) | (g << 8) | b;
}

bool wlf_color_equal(const struct wlf_color *a, const struct wlf_color *b) {
	if (a == NULL || b == NULL) {
		return false;
	}

	return a->r == b->r && a->g == b->g && a->b == b->b && a->a == b->a;
}

bool wlf_color_nearly_equal(const struct wlf_color *a, const struct wlf_color *b, double epsilon) {
	if (a == NULL || b == NULL) {
		return false;
	}

	return fabs(a->r - b->r) < epsilon &&
	       fabs(a->g - b->g) < epsilon &&
	       fabs(a->b - b->b) < epsilon &&
	       fabs(a->a - b->a) < epsilon;
}

struct wlf_color wlf_color_clamp(const struct wlf_color *color) {
	if (color == NULL) {
		return WLF_COLOR_TRANSPARENT;
	}

	return wlf_color_make(
		fmax(0.0, fmin(1.0, color->r)),
		fmax(0.0, fmin(1.0, color->g)),
		fmax(0.0, fmin(1.0, color->b)),
		fmax(0.0, fmin(1.0, color->a))
	);
}

struct wlf_color wlf_color_lerp(const struct wlf_color *a, const struct wlf_color *b, double t) {
	if (a == NULL || b == NULL) {
		return WLF_COLOR_TRANSPARENT;
	}

	t = fmax(0.0, fmin(1.0, t));

	return wlf_color_make(
		a->r + t * (b->r - a->r),
		a->g + t * (b->g - a->g),
		a->b + t * (b->b - a->b),
		a->a + t * (b->a - a->a)
	);
}

struct wlf_color wlf_color_scale(const struct wlf_color *color, double scalar) {
	if (color == NULL) {
		return WLF_COLOR_TRANSPARENT;
	}

	return wlf_color_make(
		color->r * scalar,
		color->g * scalar,
		color->b * scalar,
		color->a * scalar
	);
}

struct wlf_color wlf_color_add(const struct wlf_color *a, const struct wlf_color *b) {
	if (a == NULL || b == NULL) {
		return WLF_COLOR_TRANSPARENT;
	}

	return wlf_color_make(
		a->r + b->r,
		a->g + b->g,
		a->b + b->b,
		a->a + b->a
	);
}

struct wlf_color wlf_color_multiply(const struct wlf_color *a, const struct wlf_color *b) {
	if (a == NULL || b == NULL) {
		return WLF_COLOR_TRANSPARENT;
	}

	return wlf_color_make(
		a->r * b->r,
		a->g * b->g,
		a->b * b->b,
		a->a * b->a
	);
}

struct wlf_color wlf_color_with_alpha(const struct wlf_color *color, double alpha) {
	if (color == NULL) {
		return WLF_COLOR_TRANSPARENT;
	}

	return wlf_color_make(color->r, color->g, color->b, alpha);
}

bool wlf_color_from_str(const char *str, struct wlf_color *color) {
	if (str == NULL || color == NULL) {
		return false;
	}

	while (*str && (*str == ' ' || *str == '\t')) {
		str++;
	}

	if (*str != '(') {
		return false;
	}
	str++;

	char *endptr;
	double values[4];

	for (int i = 0; i < 4; i++) {
		while (*str && (*str == ' ' || *str == '\t')) {
			str++;
		}

		values[i] = strtod(str, &endptr);
		if (endptr == str) {
			return false;
		}
		str = endptr;

		while (*str && (*str == ' ' || *str == '\t')) {
			str++;
		}

		if (i < 3) {
			if (*str != ',') {
				return false;
			}
			str++;
		}
	}

	while (*str && (*str == ' ' || *str == '\t')) {
		str++;
	}

	if (*str != ')') {
		return false;
	}

	color->r = values[0];
	color->g = values[1];
	color->b = values[2];
	color->a = values[3];

	return true;
}
