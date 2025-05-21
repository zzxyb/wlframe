#include "wlf/math/wlf_size.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define WLF_SIZE_STRLEN 32

char* wlf_size_to_str(const struct wlf_size *size) {
	if (size == NULL) {
		return strdup("(NULL)");
	}

	char *buffer = malloc(WLF_SIZE_STRLEN);
	if (buffer == NULL) {
		wlf_log(WLF_ERROR, "Memory allocation failed for wlf_size_to_str");
		return NULL;
	}

	snprintf(buffer, WLF_SIZE_STRLEN, "(%d, %d)", size->width, size->height);

	return buffer;
}

bool wlf_size_equal(const struct wlf_size *a, const struct wlf_size *b) {
	return (a->width == b->width) && (a->height == b->height);
}

bool wlf_size_is_empty(const struct wlf_size *size) {
	return (size->width == 0) || (size->height == 0);
}

struct wlf_size wlf_size_add(const struct wlf_size *a, const struct wlf_size *b) {
	return (struct wlf_size){.width = a->width + b->width, .height = a->height + b->height};
}

struct wlf_size wlf_size_subtract(const struct wlf_size *a, const struct wlf_size *b) {
	return (struct wlf_size){.width = a->width - b->width, .height = a->height - b->height};
}

struct wlf_size wlf_size_multiply(const struct wlf_size *size, double scalar) {
	return (struct wlf_size){.width = (int)round(size->width * scalar), .height = (int)round(size->height * scalar)};
}

struct wlf_size wlf_size_divide(const struct wlf_size *size, double scalar) {
	assert(scalar != 0.0);
	return (struct wlf_size){.width = (int)round(size->width / scalar), .height = (int)round(size->height / scalar)};
}

int wlf_size_area(const struct wlf_size *size) {
	return size->width * size->height;
}

bool wlf_size_from_str(const char *str, struct wlf_size *size) {
	if (str == NULL || size == NULL) {
		return false;
	}

	while (*str == ' ' || *str == '\t') {
		str++;
	}

	if (*str != '(') {
		return false;
	}
	str++;

	char *endptr;
	long width = strtol(str, &endptr, 10);
	if (endptr == str) {
		return false;
	}
	str = endptr;

	while (*str == ' ' || *str == '\t') {
		str++;
	}

	if (*str != ',') {
		return false;
	}
	str++;

	while (*str == ' ' || *str == '\t') {
		str++;
	}

	long height = strtol(str, &endptr, 10);
	if (endptr == str) {
		return false;
	}
	str = endptr;

	while (*str == ' ' || *str == '\t') {
		str++;
	}

	if (*str != ')') {
		return false;
	}

	size->width = (int)width;
	size->height = (int)height;

	return true;
}

bool wlf_size_is_valid(const struct wlf_size *size) {
	if (size == NULL) {
		return false;
	}

	return (size->width > 0) && (size->height > 0);
}
