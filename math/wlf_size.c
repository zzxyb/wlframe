#include "wlf/math/wlf_size.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct wlf_size wlf_size_create(int width, int height) {
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
	return wlf_size_create(a->width + b->width, a->height + b->height);
}

struct wlf_size wlf_size_subtract(const struct wlf_size *a, const struct wlf_size *b) {
	return wlf_size_create(a->width - b->width, a->height - b->height);
}

struct wlf_size wlf_size_multiply(const struct wlf_size *size, int scalar) {
	return wlf_size_create(size->width * scalar, size->height * scalar);
}

struct wlf_size wlf_size_divide(const struct wlf_size *size, int scalar) {
	if (scalar != 0) {
		return wlf_size_create(size->width / scalar, size->height / scalar);
	}

	// Return original size if scalar is zero (error handling)
	return *size;
}

int wlf_size_area(const struct wlf_size *size) {
	return size->width * size->height;
}

struct wlf_fsize wlf_fsize_create(double width, double height) {
	struct wlf_fsize fsize = {width, height};

	return fsize;
}

char* wlf_fsize_to_str(const struct wlf_fsize *size) {
	char *buffer = malloc(64);
	if (buffer) {
		snprintf(buffer, 64, "FSize(%.2f, %.2f)", size->width, size->height);
	}

	return buffer;
}

bool wlf_fsize_equal(const struct wlf_fsize *a, const struct wlf_fsize *b) {
	return (a->width == b->width) && (a->height == b->height);
}

bool wlf_fsize_nearly_equal(const struct wlf_fsize *a, const struct wlf_fsize *b, double epsilon) {
	return (fabs(a->width - b->width) < epsilon) && (fabs(a->height - b->height) < epsilon);
}

struct wlf_fsize wlf_fsize_add(const struct wlf_fsize *a, const struct wlf_fsize *b) {
	return wlf_fsize_create(a->width + b->width, a->height + b->height);
}

struct wlf_fsize wlf_fsize_subtract(const struct wlf_fsize *a, const struct wlf_fsize *b) {
	return wlf_fsize_create(a->width - b->width, a->height - b->height);
}

struct wlf_fsize wlf_fsize_multiply(const struct wlf_fsize *size, double scalar) {
	return wlf_fsize_create(size->width * scalar, size->height * scalar);
}

struct wlf_fsize wlf_fsize_divide(const struct wlf_fsize *size, double scalar) {
	if (scalar != 0) {
		return wlf_fsize_create(size->width / scalar, size->height / scalar);
	}

	// Return original size if scalar is zero (error handling)
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
	return wlf_size_create(round(size->width), round(size->height));
}

struct wlf_size wlf_fsize_floor(const struct wlf_fsize *size) {
	return wlf_size_create(floor(size->width), floor(size->height));
}

struct wlf_size wlf_fsize_ceil(const struct wlf_fsize *size) {
	return wlf_size_create(ceil(size->width), ceil(size->height));
}
