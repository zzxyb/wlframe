#include "wlf/math/wlf_fsize.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define WLF_FSIZE_STRLEN 128

char *wlf_fsize_to_str(const struct wlf_fsize *size) {
	return wlf_fsize_to_str_prec(size, 3);
}

char *wlf_fsize_to_str_prec(const struct wlf_fsize *size, uint8_t precision) {
	if (size == NULL) {
		return strdup("(NULL)");
	}

	if (precision > 15) {
		precision = 15;
	}

	char fmt[32];
	snprintf(fmt, sizeof(fmt), "(%%.%df, %%.%df)", precision, precision);

	char *buffer = malloc(WLF_FSIZE_STRLEN);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate buffer");
		return NULL;
	}

	snprintf(buffer, WLF_FSIZE_STRLEN, fmt, size->width, size->height);

	return buffer;
}

bool wlf_fsize_equal(const struct wlf_fsize *a, const struct wlf_fsize *b) {
	return (a->width == b->width) && (a->height == b->height);
}

bool wlf_fsize_nearly_equal(const struct wlf_fsize *a, const struct wlf_fsize *b, double epsilon) {
	return (fabs(a->width - b->width) < epsilon) && (fabs(a->height - b->height) < epsilon);
}

struct wlf_fsize wlf_fsize_add(const struct wlf_fsize *a, const struct wlf_fsize *b) {
	return (struct wlf_fsize){.width = a->width + b->width, .height = a->height + b->height};
}

struct wlf_fsize wlf_fsize_subtract(const struct wlf_fsize *a, const struct wlf_fsize *b) {
	return (struct wlf_fsize){.width = a->width - b->width, .height = a->height - b->height};
}

struct wlf_fsize wlf_fsize_multiply(const struct wlf_fsize *size, double scalar) {
	return (struct wlf_fsize){.width = size->width * scalar, .height = size->height * scalar};
}

struct wlf_fsize wlf_fsize_divide(const struct wlf_fsize *size, double scalar) {
	assert(scalar != 0.0);
	return (struct wlf_fsize){.width = size->width / scalar, .height = size->height / scalar};
}

double wlf_fsize_area(const struct wlf_fsize *size) {
	return size->width * size->height;
}

struct wlf_fsize wlf_size_to_fsize(const struct wlf_size *size) {
	struct wlf_fsize fsize = {(double)size->width, (double)size->height};
	return fsize;
}

struct wlf_size wlf_fsize_to_size(const struct wlf_fsize *size) {
	struct wlf_size isize = {(int)size->width, (int)size->height};
	return isize;
}

struct wlf_size wlf_fsize_round(const struct wlf_fsize *size) {
	return (struct wlf_size){.width = round(size->width), .height = round(size->height)};
}

struct wlf_size wlf_fsize_floor(const struct wlf_fsize *size) {
	return (struct wlf_size){.width = floor(size->width), .height = floor(size->height)};
}

struct wlf_size wlf_fsize_ceil(const struct wlf_fsize *size) {
	return (struct wlf_size){.width = ceil(size->width), .height = ceil(size->height)};
}

bool wlf_fsize_from_str(const char *str, struct wlf_fsize *size) {
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
	double width = strtod(str, &endptr);
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

	double height = strtod(str, &endptr);
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

	size->width = width;
	size->height = height;
	return true;
}

bool wlf_fsize_is_valid(const struct wlf_fsize *size) {
	if (size == NULL) {
		return false;
	}

	return (size->width > 0.0) && (size->height > 0.0);
}
