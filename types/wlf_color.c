/**
 * @file        wlf_color.c
 * @brief       Color utility implementation for wlframe.
 * @details     This file provides implementation of color operations,
 *              including creation, conversion, arithmetic, interpolation, and utilities.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#include "wlf/types/wlf_color.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct wlf_color wlf_color_make(double r, double g, double b, double a) {
	struct wlf_color color = {r, g, b, a};
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
	if (!color || precision > 15) {
		return NULL;
	}

	// Allocate buffer for the string representation
	// Format: "(r.xxx, g.xxx, b.xxx, a.xxx)" + null terminator
	char *str = malloc(64);
	if (!str) {
		return NULL;
	}

	snprintf(str, 64, "(%.*f, %.*f, %.*f, %.*f)",
		precision, color->r,
		precision, color->g,
		precision, color->b,
		precision, color->a);

	return str;
}

char* wlf_color_to_str(const struct wlf_color *color) {
	return wlf_color_to_str_prec(color, 3);
}

uint32_t wlf_color_to_hex(const struct wlf_color *color) {
	if (!color) {
		return 0;
	}

	uint8_t r = (uint8_t)(fmax(0.0, fmin(1.0, color->r)) * 255.0);
	uint8_t g = (uint8_t)(fmax(0.0, fmin(1.0, color->g)) * 255.0);
	uint8_t b = (uint8_t)(fmax(0.0, fmin(1.0, color->b)) * 255.0);
	uint8_t a = (uint8_t)(fmax(0.0, fmin(1.0, color->a)) * 255.0);

	return (r << 24) | (g << 16) | (b << 8) | a;
}

uint32_t wlf_color_to_hex_rgb(const struct wlf_color *color) {
	if (!color) {
		return 0;
	}

	uint8_t r = (uint8_t)(fmax(0.0, fmin(1.0, color->r)) * 255.0);
	uint8_t g = (uint8_t)(fmax(0.0, fmin(1.0, color->g)) * 255.0);
	uint8_t b = (uint8_t)(fmax(0.0, fmin(1.0, color->b)) * 255.0);

	return (r << 16) | (g << 8) | b;
}

bool wlf_color_equal(const struct wlf_color *a, const struct wlf_color *b) {
	if (!a || !b) {
		return false;
	}

	return a->r == b->r && a->g == b->g && a->b == b->b && a->a == b->a;
}

bool wlf_color_nearly_equal(const struct wlf_color *a, const struct wlf_color *b, double epsilon) {
	if (!a || !b) {
		return false;
	}

	return fabs(a->r - b->r) < epsilon &&
	       fabs(a->g - b->g) < epsilon &&
	       fabs(a->b - b->b) < epsilon &&
	       fabs(a->a - b->a) < epsilon;
}

struct wlf_color wlf_color_clamp(const struct wlf_color *color) {
	if (!color) {
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
	if (!a || !b) {
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
	if (!color) {
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
	if (!a || !b) {
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
	if (!a || !b) {
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
	if (!color) {
		return WLF_COLOR_TRANSPARENT;
	}

	return wlf_color_make(color->r, color->g, color->b, alpha);
}

bool wlf_color_from_str(const char *str, struct wlf_color *color) {
	if (!str || !color) {
		return false;
	}

	// Skip whitespace and find opening parenthesis
	while (*str && (*str == ' ' || *str == '\t')) {
		str++;
	}

	if (*str != '(') {
		return false;
	}
	str++;

	// Parse four double values separated by commas
	char *endptr;
	double values[4];

	for (int i = 0; i < 4; i++) {
		// Skip whitespace
		while (*str && (*str == ' ' || *str == '\t')) {
			str++;
		}

		values[i] = strtod(str, &endptr);
		if (endptr == str) {
			return false;  // No valid number found
		}
		str = endptr;

		// Skip whitespace
		while (*str && (*str == ' ' || *str == '\t')) {
			str++;
		}

		// Check for comma (except for the last value)
		if (i < 3) {
			if (*str != ',') {
				return false;
			}
			str++;
		}
	}

	// Skip whitespace and check for closing parenthesis
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
