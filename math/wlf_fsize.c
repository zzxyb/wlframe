#include "wlf/math/wlf_fsize.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

char *wlf_fsize_to_str(const struct wlf_fsize *size) {
	return wlf_fsize_to_str_prec(size, 3);
}

char *wlf_fsize_to_str_prec(const struct wlf_fsize *size, int precision) {
	if (size == NULL) {
		return strdup("(NULL)");
	}

	if (precision < 0 || precision > 15) {
		precision = 3; // Default precision
	}

	char fmt[32];
	snprintf(fmt, sizeof(fmt), "(%%.%df, %%.%df)", precision, precision);

	char *buffer = malloc(64);
	if (buffer == NULL) {
		wlf_log(WLF_ERROR, "Memory allocation failed for wlf_fsize_to_str_prec");
		return NULL;
	}
	snprintf(buffer, 64, fmt, size->width, size->height);

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
	if (scalar != 0.0) {
		return (struct wlf_fsize){.width = size->width / scalar, .height = size->height / scalar};
	}

	wlf_log(WLF_ERROR, "Division by zero in wlf_fsize_divide, returning original size");
	return *size;
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
