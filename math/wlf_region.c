#include "wlf/math/wlf_region.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static struct wlf_frect rect_union(const struct wlf_frect *a, const struct wlf_frect *b) {
	if (a == NULL) {
		return b ? *b : (struct wlf_frect){
			.x = 0,
			.y = 0,
			.width = 0,
			.height= 0,
		};
	}

	if (b == NULL) {
		return *a;
	}

	double x1 = (a->x < b->x) ? a->x : b->x;
	double y1 = (a->y < b->y) ? a->y : b->y;
	double x2 = ((a->x + a->width) > (b->x + b->width)) ? (a->x + a->width) : (b->x + b->width);
	double y2 = ((a->y + a->height) > (b->y + b->height)) ? (a->y + a->height) : (b->y + b->height);

	struct wlf_frect out;
	out.x = x1;
	out.y = y1;
	out.width = x2 - x1;
	out.height = y2 - y1;

	return out;
}

void wlf_region_init(struct wlf_region *region) {
	region->data = malloc(sizeof(struct wlf_region_data));
	if (region->data) {
		region->data->size = 4;
		region->data->numRects = 0;
		region->data->rects = calloc(region->data->size, sizeof(struct wlf_frect));
		if (region->data->rects == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_region_data rects");
			free(region->data);
			region->data = NULL;
			return;
		}
	} else {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_region_data");
	}

	region->extents = (struct wlf_frect){
		.x= 0,
		.y = 0,
		.width = 0,
		.height = 0,
	};
}

void wlf_region_fini(struct wlf_region *region) {
	if (region == NULL) {
		return;
	}

	if (region->data == NULL) {
		return;
	}

	free(region->data->rects);
	free(region->data);
	region->data = NULL;
}

char* wlf_region_to_str(const struct wlf_region *region) {
	if (region == NULL || region->data == NULL) {
		return strdup("{NULL}");
	}

	size_t str_size = 256 + region->data->numRects * 64;
	char *str = malloc(str_size);
	if (str == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate region string");
		return NULL;
	}

	int offset = snprintf(str, str_size, "{\n");

	for (long i = 0; i < region->data->numRects; ++i) {
		const struct wlf_frect *r = &region->data->rects[i];
		int written = snprintf(str + offset, str_size - offset,
			"[%.3f,%.3f,%.3f,%.3f]", r->x, r->y, r->width, r->height);
		if (written < 0 || (size_t)written >= str_size - offset) {
			break;
		}
		offset += written;

		if (i < region->data->numRects - 1) {
			written = snprintf(str + offset, str_size - offset, ",\n");
		} else {
			written = snprintf(str + offset, str_size - offset, "\n");
		}
		if (written < 0 || (size_t)written >= str_size - offset) {
			break;
		}
		offset += written;
	}

	snprintf(str + offset, str_size - offset, "}");

	return str;
}

bool wlf_region_from_str(const char *str, struct wlf_region *out_region) {
	if (str == NULL || out_region == NULL) {
		return false;
	}

	wlf_region_init(out_region);

	const char *p = str;

	while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '{')) {
		p++;
	}

	while (*p) {
		while (*p && *p != '[') {
			p++;
		}

		if (!*p) {
			break;
		}

		p++;

		double x, y, w, h;
		int matched = sscanf(p, "%lf,%lf,%lf,%lf", &x, &y, &w, &h);
		if (matched != 4) {
			wlf_region_fini(out_region);
			return false;
		}

		struct wlf_frect rect = {
			.x = x,
			.y = y,
			.width = w,
			.height = h,
		};
		if (!wlf_region_add_rect(out_region, &rect)) {
			wlf_region_fini(out_region);
			return false;
		}

		while (*p && *p != ']') {
			p++;
		}

		if (*p) {
			p++;
		}

		while (*p && (*p == ',' || *p == ' ' || *p == '\t' || *p == '\n')) {
			p++;
		}

		if (*p == '}') {
			break;
		}
	}

	return true;
}

struct wlf_frect wlf_region_bounding_rect(const struct wlf_region *region) {
	if (region == NULL || region->data == NULL || region->data->numRects == 0) {
		return (struct wlf_frect){
			.x = 0,
			.y = 0,
			.width = 0,
			.height = 0,
		};
	}

	return region->extents;
}

bool wlf_region_is_nil(const struct wlf_region *region) {
	return region->data == NULL || region->data->numRects == 0;
}

bool wlf_region_add_rect(struct wlf_region *region, const struct wlf_frect *rect) {
	if (region->data == NULL) {
		return false;
	}

	if (region->data->numRects >= region->data->size) {
		long new_size = region->data->size * 2;
		struct wlf_frect *new_rects = realloc(region->data->rects, new_size * sizeof(struct wlf_frect));
		if (new_rects == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to reallocate memory for wlf_region_data");
			return false;
		}
		region->data->rects = new_rects;
		region->data->size = new_size;
	}

	region->data->rects[region->data->numRects++] = *rect;

	if (region->data->numRects == 1) {
		region->extents = *rect;
	} else {
		region->extents = rect_union(&region->extents, rect);
	}

	return true;
}

bool wlf_region_contains_point(const struct wlf_region *region, double x, double y) {
	if (region->data == NULL) {
		return false;
	}

	for (long i = 0; i < region->data->numRects; ++i) {
		const struct wlf_frect *r = &region->data->rects[i];
		if (x >= r->x && x < r->x + r->width &&
				y >= r->y && y < r->y + r->height) {
			return true;
		}
	}

	return false;
}

void wlf_region_intersects_rect(const struct wlf_region *region, const struct wlf_frect *rect,
		struct wlf_region *result) {
	wlf_region_fini(result);
	wlf_region_init(result);

	if (region == NULL || rect == NULL) {
		return;
	}

	struct wlf_region rect_region;
	wlf_region_init(&rect_region);
	wlf_region_add_rect(&rect_region, rect);

	wlf_region_intersect(region, &rect_region, result);
}

void wlf_region_union(struct wlf_region *dst, const struct wlf_region *src) {
	if (src->data == NULL) {
		return;
	}

	for (long i = 0; i < src->data->numRects; ++i) {
		wlf_region_add_rect(dst, &src->data->rects[i]);
	}
}

void wlf_region_intersect(const struct wlf_region *dst, const struct wlf_region *src, struct wlf_region *result) {
	wlf_region_fini(result);
	wlf_region_init(result);

	if (dst->data == NULL || src->data == NULL) {
		return;
	}

	struct wlf_frect *results_rect = malloc(sizeof(struct wlf_frect) * dst->data->numRects * src->data->numRects);
	if (!results_rect) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for intersection results_rect");
		return;
	}

	long count = 0;

	for (long i = 0; i < dst->data->numRects; ++i) {
		for (long j = 0; j < src->data->numRects; ++j) {
			const struct wlf_frect *a = &dst->data->rects[i];
			const struct wlf_frect *b = &src->data->rects[j];

			double x1 = (a->x > b->x) ? a->x : b->x;
			double y1 = (a->y > b->y) ? a->y : b->y;
			double x2_a = a->x + a->width;
			double x2_b = b->x + b->width;
			double x2 = (x2_a < x2_b) ? x2_a : x2_b;
			double y2_a = a->y + a->height;
			double y2_b = b->y + b->height;
			double y2 = (y2_a < y2_b) ? y2_a : y2_b;

			if (x1 < x2 && y1 < y2) {
				results_rect[count++] = (struct wlf_frect){
					.x = x1,
					.y = y1,
					.width = x2 - x1,
					.height = y2 - y1
				};
			}
		}
	}

	for (long i = 0; i < count; ++i) {
		wlf_region_add_rect(result, &results_rect[i]);
	}

	free(results_rect);
}
