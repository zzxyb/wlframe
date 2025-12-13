#include "wlf/math/wlf_frect.h"
#include "wlf/math/wlf_fpoint.h"
#include "wlf/math/wlf_fsize.h"
#include "wlf/math/wlf_rect.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define WLF_FRECT_STRLEN 256

struct wlf_frect wlf_frect_make(double x, double y, double width, double height) {
	return (struct wlf_frect){
		.x = x,
		.y = y,
		.width = width,
		.height = height,
	};
}

struct wlf_frect wlf_frect_from_point_size(const struct wlf_fpoint *pos, const struct wlf_fsize *size) {
	return wlf_frect_make(pos->x, pos->y, size->width, size->height);
}

struct wlf_frect wlf_frect_from_points(const struct wlf_fpoint *p1, const struct wlf_fpoint *p2) {
	return wlf_frect_make(fmin(p1->x, p2->x), fmin(p1->y, p2->y), fabs(p1->x - p2->x),
		fabs(p1->y - p2->y));
}

char* wlf_frect_to_str_prec(const struct wlf_frect *rect, uint8_t precision) {
	if (rect == NULL) {
		return strdup("(NULL)");
	}

	if (precision > 15) {
		precision = 15;
	}

	char fmt[64];
	snprintf(fmt, sizeof(fmt), "(%%.%df, %%.%df, %%.%df, %%.%df)",
		precision, precision, precision, precision);

	char *buffer = malloc(WLF_FRECT_STRLEN);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate buffer");
		return NULL;
	}

	snprintf(buffer, WLF_FRECT_STRLEN, fmt, rect->x, rect->y, rect->width, rect->height);

	return buffer;
}

bool wlf_frect_equal(const struct wlf_frect *a, const struct wlf_frect *b) {
	if (wlf_frect_is_empty(a)) {
		a = NULL;
	}

	if (wlf_frect_is_empty(b)) {
		b = NULL;
	}

	if (a == NULL || b == NULL) {
		return a == b;
	}

	return a->x == b->x && a->y == b->y &&
		a->width == b->width && a->height == b->height;
}

bool wlf_frect_nearly_equal(const struct wlf_frect *a, const struct wlf_frect *b, double epsilon) {
	if (wlf_frect_is_empty(a)) {
		a = NULL;
	}

	if (wlf_frect_is_empty(b)) {
		b = NULL;
	}

	if (a == NULL || b == NULL) {
		return a == b;
	}

	return (fabs(a->x - b->x) < epsilon) && (fabs(a->y - b->y) < epsilon) &&
		(fabs(a->width - b->width) < epsilon) && (fabs(a->height - b->height) < epsilon);
}

struct wlf_frect wlf_rect_to_frect(const struct wlf_rect *rect) {
	return wlf_frect_make((double)rect->x, (double)rect->y, (double)rect->width, (double)rect->height);
}

struct wlf_rect wlf_frect_to_rect(const struct wlf_frect *rect) {
	return wlf_rect_make((int)rect->x, (int)rect->y, (int)rect->width, (int)rect->height);
}

struct wlf_rect wlf_frect_round(const struct wlf_frect *rect) {
	return wlf_rect_make(
		(int)round(rect->x),
		(int)round(rect->y),
		(int)round(rect->width),
		(int)round(rect->height)
	);
}

struct wlf_rect wlf_frect_floor(const struct wlf_frect *rect) {
	return wlf_rect_make(
		(int)floor(rect->x),
		(int)floor(rect->y),
		(int)floor(rect->width),
		(int)floor(rect->height)
	);
}

struct wlf_rect wlf_frect_ceil(const struct wlf_frect *rect) {
	return wlf_rect_make(
		(int)ceil(rect->x),
		(int)ceil(rect->y),
		(int)ceil(rect->width),
		(int)ceil(rect->height)
	);
}

bool wlf_frect_from_str(const char *str, struct wlf_frect *rect) {
	if (str == NULL || rect == NULL) {
		return false;
	}

	while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
		str++;
	}

	if (*str == '\0') {
		return false;
	}

	if (*str != '(') {
		return false;
	}
	str++;

	double x, y, width, height;
	int parsed = sscanf(str, "%lf,%lf,%lf,%lf", &x, &y, &width, &height);

	if (parsed != 4) {
		parsed = sscanf(str, "%lf, %lf, %lf, %lf", &x, &y, &width, &height);
	}

	if (parsed != 4) {
		return false;
	}

	const char *end_ptr = strchr(str, ')');
	if (end_ptr == NULL) {
		return false;
	}

	end_ptr++;
	while (*end_ptr == ' ' || *end_ptr == '\t' || *end_ptr == '\n' || *end_ptr == '\r') {
		end_ptr++;
	}

	if (*end_ptr != '\0') {
		return false;
	}

	rect->x = x;
	rect->y = y;
	rect->width = width;
	rect->height = height;

	return true;
}

bool wlf_frect_is_empty(const struct wlf_frect *rect) {
	return rect == NULL || rect->width <= 0 || rect->height <= 0;
}

bool wlf_frect_intersection(struct wlf_frect *dest, const struct wlf_frect *a,
		const struct wlf_frect *b) {
	bool a_empty = wlf_frect_is_empty(a);
	bool b_empty = wlf_frect_is_empty(b);

	if (a_empty || b_empty) {
		*dest = (struct wlf_frect){0};
		return false;
	}

	double x1 = fmax(a->x, b->x);
	double y1 = fmax(a->y, b->y);
	double x2 = fmin(a->x + a->width, b->x + b->width);
	double y2 = fmin(a->y + a->height, b->y + b->height);

	dest->x = x1;
	dest->y = y1;
	dest->width = x2 - x1;
	dest->height = y2 - y1;

	if (wlf_frect_is_empty(dest)) {
		*dest = (struct wlf_frect){0};
		return false;
	}

	return true;
}

bool wlf_frect_contains_point(const struct wlf_frect *rect, double x, double y) {
	if (wlf_frect_is_empty(rect)) {
		return false;
	}

	return x >= rect->x && x < rect->x + rect->width &&
		y >= rect->y && y < rect->y + rect->height;
}

bool wlf_frect_contains_frect(const struct wlf_frect *bigger, const struct wlf_frect *smaller) {
	if (wlf_frect_is_empty(bigger) || wlf_frect_is_empty(smaller)) {
		return false;
	}

	return smaller->x >= bigger->x &&
		smaller->x + smaller->width <= bigger->x + bigger->width &&
		smaller->y >= bigger->y &&
		smaller->y + smaller->height <= bigger->y + bigger->height;
}

void wlf_frect_transform(struct wlf_frect *dest, const struct wlf_frect *rect,
		enum wlf_output_transform transform, double width, double height) {
	struct wlf_frect src = {0};
	if (rect != NULL) {
		src = *rect;
	}

	if (transform % 2 == 0) {
		dest->width = src.width;
		dest->height = src.height;
	} else {
		dest->width = src.height;
		dest->height = src.width;
	}

	switch (transform) {
	case WLF_OUTPUT_TRANSFORM_NORMAL:
		dest->x = src.x;
		dest->y = src.y;
		break;
	case WLF_OUTPUT_TRANSFORM_90:
		dest->x = height - src.y - src.height;
		dest->y = src.x;
		break;
	case WLF_OUTPUT_TRANSFORM_180:
		dest->x = width - src.x - src.width;
		dest->y = height - src.y - src.height;
		break;
	case WLF_OUTPUT_TRANSFORM_270:
		dest->x = src.y;
		dest->y = width - src.x - src.width;
		break;
	case WLF_OUTPUT_TRANSFORM_FLIPPED:
		dest->x = width - src.x - src.width;
		dest->y = src.y;
		break;
	case WLF_OUTPUT_TRANSFORM_FLIPPED_90:
		dest->x = src.y;
		dest->y = src.x;
		break;
	case WLF_OUTPUT_TRANSFORM_FLIPPED_180:
		dest->x = src.x;
		dest->y = height - src.y - src.height;
		break;
	case WLF_OUTPUT_TRANSFORM_FLIPPED_270:
		dest->x = height - src.y - src.height;
		dest->y = width - src.x - src.width;
		break;
	}
}
