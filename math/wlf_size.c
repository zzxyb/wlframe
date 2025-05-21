#include "wlf/math/wlf_size.h"

#include <stdlib.h>
#include <stdio.h>

struct wlf_size wlf_size_make(int width, int height) {
	struct wlf_size size = {width, height};

	return size;
}

char* wlf_size_to_str(const struct wlf_size *size) {
	char *buffer = malloc(64);
	if (buffer) {
		snprintf(buffer, 64, "Size(%d, %d)", size->width, size->height);
	}

	return buffer;
}

bool wlf_size_equal(const struct wlf_size *a, const struct wlf_size *b) {
	return (a->width == b->width) && (a->height == b->height);
}

bool wlf_size_is_empty(const struct wlf_size *size) {
	return (size->width == 0) || (size->height == 0);
}

bool wlf_size_is_valid(const struct wlf_size *size) {
	return (size->width > 0) && (size->height > 0);
}

struct wlf_size wlf_size_add(const struct wlf_size *a, const struct wlf_size *b) {
	return wlf_size_make(a->width + b->width, a->height + b->height);
}

struct wlf_size wlf_size_subtract(const struct wlf_size *a, const struct wlf_size *b) {
	return wlf_size_make(a->width - b->width, a->height - b->height);
}

struct wlf_size wlf_size_multiply(const struct wlf_size *size, int scalar) {
	return wlf_size_make(size->width * scalar, size->height * scalar);
}

struct wlf_size wlf_size_divide(const struct wlf_size *size, int scalar) {
	if (scalar != 0) {
		return wlf_size_make(size->width / scalar, size->height / scalar);
	}

	// Return original size if scalar is zero (error handling)
	return *size;
}

int wlf_size_area(const struct wlf_size *size) {
	return size->width * size->height;
}
