#include "wlf/math/wlf_rect.h"
#include "wlf/math/wlf_point.h"
#include "wlf/math/wlf_size.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define WLF_RECT_STRLEN 64

struct wlf_rect wlf_rect_make(int x, int y, int width, int height) {
	return (struct wlf_rect){
		.x = x,
		.y = y,
		.width = width,
		.height = height,
	};
}

char* wlf_rect_to_str(const struct wlf_rect *rect) {
	if (rect == NULL) {
		return strdup("(NULL)");
	}

	char *buffer = malloc(WLF_RECT_STRLEN);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate buffer");
		return NULL;
	}

	snprintf(buffer, WLF_RECT_STRLEN, "(%d, %d, %d, %d)", rect->x, rect->y, rect->width, rect->height);

	return buffer;
}

struct wlf_rect wlf_rect_from_point_size(const struct wlf_point *pos, const struct wlf_size *size) {
	return wlf_rect_make(pos->x, pos->y, size->width, size->height);
}

struct wlf_rect wlf_rect_from_points(const struct wlf_point *p1, const struct wlf_point *p2) {
	int x = fmin(p1->x, p2->x);
	int y = fmin(p1->y, p2->y);
	int width = abs(p1->x - p2->x);
	int height = abs(p1->y - p2->y);

	return wlf_rect_make(x, y, width, height);
}

bool wlf_rect_equal(const struct wlf_rect *a, const struct wlf_rect *b) {
	if (wlf_rect_is_empty(a)) {
		a = NULL;
	}

	if (wlf_rect_is_empty(b)) {
		b = NULL;
	}

	if (a == NULL || b == NULL) {
		return a == b;
	}

	return a->x == b->x && a->y == b->y &&
		a->width == b->width && a->height == b->height;
}

bool wlf_rect_is_empty(const struct wlf_rect *rect) {
	return rect == NULL || rect->width <= 0 || rect->height <= 0;
}

struct wlf_point wlf_rect_get_center(const struct wlf_rect *rect) {
	return (struct wlf_point){
		.x = rect->x + rect->width / 2,
		.y = rect->y + rect->height / 2
	};
}

struct wlf_point wlf_rect_get_top_left(const struct wlf_rect *rect) {
	return (struct wlf_point){
		.x = rect->x,
		.y = rect->y
	};
}

struct wlf_point wlf_rect_get_bottom_right(const struct wlf_rect *rect) {
	return (struct wlf_point){
		.x = rect->x + rect->width,
		.y = rect->y + rect->height
	};
}

struct wlf_rect wlf_rect_offset(const struct wlf_rect *rect, const struct wlf_point *offset) {
	return wlf_rect_make(rect->x + offset->x, rect->y + offset->y, rect->width, rect->height);
}

struct wlf_rect wlf_rect_inflate(const struct wlf_rect *rect, int dx, int dy) {
	return wlf_rect_make(rect->x - dx, rect->y - dy, rect->width + 2 * dx, rect->height + 2 * dy);
}

struct wlf_rect wlf_rect_scale(const struct wlf_rect *rect, double sx, double sy) {
	return wlf_rect_make(rect->x, rect->y, (int)(rect->width * sx), (int)(rect->height * sy));
}

bool wlf_rect_from_str(const char *str, struct wlf_rect *rect) {
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

	int x, y, width, height;
	int parsed = sscanf(str, "%d,%d,%d,%d", &x, &y, &width, &height);

	if (parsed != 4) {
		parsed = sscanf(str, "%d, %d, %d, %d", &x, &y, &width, &height);
	}

	if (parsed != 4) {
		return false;
	}

	const char *end_ptr = strchr(str, ')');
	if (end_ptr == NULL) {
		return false;
	}

	const char *check_ptr = end_ptr - 1;
	while (check_ptr > str && (*check_ptr == ' ' || *check_ptr == '\t')) {
		check_ptr--;
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

bool wlf_rect_intersection(struct wlf_rect *dest, const struct wlf_rect *a,
		const struct wlf_rect *b) {
	if (dest == NULL) {
		return false;
	}

	*dest = wlf_rect_make(0, 0, 0, 0);
	if (wlf_rect_is_empty(a) || wlf_rect_is_empty(b)) {
		return false;
	}

	int left = a->x > b->x ? a->x : b->x;
	int top = a->y > b->y ? a->y : b->y;
	int right_a = a->x + a->width;
	int right_b = b->x + b->width;
	int bottom_a = a->y + a->height;
	int bottom_b = b->y + b->height;
	int right = right_a < right_b ? right_a : right_b;
	int bottom = bottom_a < bottom_b ? bottom_a : bottom_b;

	if (left >= right || top >= bottom) {
		return false;
	}

	*dest = wlf_rect_make(left, top, right - left, bottom - top);
	return true;
}

bool wlf_rect_contains_point(const struct wlf_rect *rect, int x, int y) {
	if (wlf_rect_is_empty(rect)) {
		return false;
	}

	return x >= rect->x && x < rect->x + rect->width &&
		y >= rect->y && y < rect->y + rect->height;
}

bool wlf_rect_contains_frect(const struct wlf_rect *bigger, const struct wlf_rect *smaller) {
	if (wlf_rect_is_empty(bigger) || wlf_rect_is_empty(smaller)) {
		return false;
	}

	return smaller->x >= bigger->x &&
		smaller->y >= bigger->y &&
		smaller->x + smaller->width <= bigger->x + bigger->width &&
		smaller->y + smaller->height <= bigger->y + bigger->height;
}
