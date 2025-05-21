#include "wlf/math/wlf_frect.h"
#include "wlf/math/wlf_fpoint.h"
#include "wlf/math/wlf_fsize.h"
#include "wlf/math/wlf_rect.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct wlf_frect wlf_frect_make(double x, double y, double width, double height) {
	struct wlf_frect frect = {
		.x = x,
		.y = y,
		.width = width,
		.height = height,
	};

	return frect;
}

char* wlf_frect_to_str_prec(const struct wlf_frect *rect, int precision) {
	if (rect == NULL) {
		return strdup("FRect(NULL)");
	}

	if (precision < 0 || precision > 15) {
		precision = 3; // Default precision
	}

	char fmt[32];
	snprintf(fmt, sizeof(fmt), "FRect(%%.%df, %%.%df, %%.%df, %%.%df)",
		precision, precision, precision, precision);

	char *buffer = malloc(64);
	if (buffer == NULL) {
		wlf_log(WLF_ERROR, "Memory allocation failed for wlf_frect_to_str_prec");
		return NULL;
	}
	snprintf(buffer, 64, fmt, rect->x, rect->y, rect->width, rect->height);

	return buffer;
}

bool wlf_frect_equal(const struct wlf_frect *a, const struct wlf_frect *b) {
	return (a->x == b->x) && (a->y == b->y) && (a->width == b->width) && (a->height == b->height);
}

bool wlf_frect_nearly_equal(const struct wlf_frect *a, const struct wlf_frect *b, double epsilon) {
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
	return wlf_rect_make(round(rect->x), round(rect->y), round(rect->width), round(rect->height));
}

struct wlf_rect wlf_frect_floor(const struct wlf_frect *rect) {
	return wlf_rect_make(floor(rect->x), floor(rect->y), floor(rect->width), floor(rect->height));
}

struct wlf_rect wlf_frect_ceil(const struct wlf_frect *rect) {
	return wlf_rect_make(ceil(rect->x), ceil(rect->y), ceil(rect->width), ceil(rect->height));
}
