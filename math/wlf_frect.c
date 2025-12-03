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
	struct wlf_frect frect = {
		.x = x,
		.y = y,
		.width = width,
		.height = height,
	};

	return frect;
}

struct wlf_frect wlf_frect_from_point_size(const struct wlf_fpoint *pos, const struct wlf_fsize *size) {
	return wlf_frect_make(pos->x, pos->y, size->width, size->height);
}

struct wlf_frect wlf_frect_from_points(const struct wlf_fpoint *p1, const struct wlf_fpoint *p2) {
	int x = fmin(p1->x, p2->x);
	int y = fmin(p1->y, p2->y);
	int width = fabs(p1->x - p2->x);
	int height = fabs(p1->y - p2->y);

	return wlf_frect_make(x, y, width, height);
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

bool wlf_frect_is_valid(const struct wlf_frect *rect) {
	if (rect == NULL) {
		return false;
	}
	return (rect->width > 0.0) && (rect->height > 0.0);
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

	char *end_ptr = strchr(str, ')');
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
