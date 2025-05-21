#include "wlf/math/wlf_size.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>

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
	return (struct wlf_size){.width = a->width + b->width, .height = a->height + b->height};
}

struct wlf_size wlf_size_subtract(const struct wlf_size *a, const struct wlf_size *b) {
	return (struct wlf_size){.width = a->width - b->width, .height = a->height - b->height};
}

struct wlf_size wlf_size_multiply(const struct wlf_size *size, int scalar) {
	return (struct wlf_size){.width = size->width * scalar, .height = size->height * scalar};
}

struct wlf_size wlf_size_divide(const struct wlf_size *size, int scalar) {
	if (scalar != 0) {
		return (struct wlf_size){.width = size->width / scalar, .height = size->height / scalar};
	}

	wlf_log(WLF_ERROR, "Division by zero in wlf_size_divide, returning original size");
	return *size;
}

int wlf_size_area(const struct wlf_size *size) {
	return size->width * size->height;
}
