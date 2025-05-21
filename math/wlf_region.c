#include "wlf/math/wlf_region.h"
#include "wlf/math/wlf_rect.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>

static struct wlf_rect rect_union(const struct wlf_rect *a, const struct wlf_rect *b) {
	if (a == NULL) {
		return b ? *b : (struct wlf_rect){0, 0, 0, 0};
	}

	if (b == NULL) {
		return *a;
	}

	int x1 = (a->x < b->x) ? a->x : b->x;
	int y1 = (a->y < b->y) ? a->y : b->y;
	int x2 = ((a->x + a->width) > (b->x + b->width)) ? (a->x + a->width) : (b->x + b->width);
	int y2 = ((a->y + a->height) > (b->y + b->height)) ? (a->y + a->height) : (b->y + b->height);

	struct wlf_rect out;
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
		region->data->rects = calloc(region->data->size, sizeof(struct wlf_rect));
	} else {
		wlf_log(WLF_ERROR, "Failed to allocate memory for wlf_region_data");
	}

	region->extents = (struct wlf_rect){0, 0, 0, 0};
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
		return strdup("Region: NULL");
	}

	size_t str_size = 256 + region->data->numRects * 64;
	char *str = malloc(str_size);
	if (!str) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for region string representation");
		return NULL;
	}

	int offset = snprintf(str, str_size,
		"Region: %ld rects, extents: [%d, %d, %d, %d]",
		region->data->numRects,
		region->extents.x, region->extents.y,
		region->extents.width, region->extents.height);

	for (long i = 0; i < region->data->numRects; ++i) {
		const struct wlf_rect *r = &region->data->rects[i];
		int written = snprintf(str + offset, str_size - offset,
			"\n  rect[%ld]: [%d, %d, %d, %d]", i, r->x, r->y, r->width, r->height);
		if (written < 0 || (size_t)written >= str_size - offset) {
			break;
		}
		offset += written;
	}

	return str;
}

bool wlf_region_from_str(const char *str, struct wlf_region *out_region) {
	if (str == NULL || out_region == NULL) {
		return false;
	}

	wlf_region_init(out_region);

	const char *p = str;
	while (*p) {
		while (*p && *p != '[') p++;
		if (!*p) {
			break;
		}

		int x, y, w, h;
		int matched = sscanf(p, "[%d,%d,%d,%d]", &x, &y, &w, &h);
		if (matched != 4) {
			wlf_region_fini(out_region);
			return false;
		}

		struct wlf_rect rect = {
			.x = x,
			.y= y,
			.width = w,
			.height = h
		};
		if (!wlf_region_add_rect(out_region, &rect)) {
			wlf_region_fini(out_region);
			return false;
		}

		while (*p && *p != ']') p++;
		if (*p) {
			p++;
		}
	}

	return true;
}

struct wlf_rect wlf_region_bounding_rect(const struct wlf_region *region) {
	if (region == NULL || region->data == NULL || region->data->numRects == 0) {
		return (struct wlf_rect){0, 0, 0, 0};
	}

	return region->extents;
}

bool wlf_region_is_nil(const struct wlf_region *region) {
	return region->data == NULL || region->data->numRects == 0;
}

bool wlf_region_add_rect(struct wlf_region *region, const struct wlf_rect *rect) {
	if (!region->data) {
		return false;
	}

	if (region->data->numRects >= region->data->size) {
		long new_size = region->data->size * 2;
		struct wlf_rect *new_rects = realloc(region->data->rects, new_size * sizeof(struct wlf_rect));
		if (!new_rects) {
			wlf_log(WLF_ERROR, "Failed to reallocate memory for wlf_region_data");
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

bool wlf_region_contains_point(const struct wlf_region *region, int x, int y) {
	if (region->data == NULL) {
		return false;
	}

	for (long i = 0; i < region->data->numRects; ++i) {
		const struct wlf_rect *r = &region->data->rects[i];
		if (x >= r->x && x < r->x + r->width &&
				y >= r->y && y < r->y + r->height) {
			return true;
		}
	}

	return false;
}

bool wlf_region_intersects_rect(const struct wlf_region *region, const struct wlf_rect *rect) {
	if (region->data == NULL || rect == NULL) {
		return false;
	}

	for (long i = 0; i < region->data->numRects; ++i) {
		const struct wlf_rect *r = &region->data->rects[i];
		int r_x2 = r->x + r->width;
		int r_y2 = r->y + r->height;
		int rect_x2 = rect->x + rect->width;
		int rect_y2 = rect->y + rect->height;

		if (!(rect_x2 <= r->x || rect->x >= r_x2 || rect_y2 <= r->y || rect->y >= r_y2)) {
			return true;
		}
	}

	return false;
}

void wlf_region_union(struct wlf_region *dst, const struct wlf_region *src) {
	if (src->data == NULL) {
		return;
	}

	for (long i = 0; i < src->data->numRects; ++i) {
		wlf_region_add_rect(dst, &src->data->rects[i]);
	}
}

void wlf_region_intersect(struct wlf_region *dst, const struct wlf_region *src) {
	if (dst->data == NULL || src->data == NULL) {
		return;
	}

	struct wlf_rect *results = malloc(sizeof(struct wlf_rect) * dst->data->numRects * src->data->numRects);
	if (!results) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for intersection results");
		return;
	}
	long count = 0;

	for (long i = 0; i < dst->data->numRects; ++i) {
		for (long j = 0; j < src->data->numRects; ++j) {
			const struct wlf_rect *a = &dst->data->rects[i];
			const struct wlf_rect *b = &src->data->rects[j];

			int x1 = (a->x > b->x) ? a->x : b->x;
			int y1 = (a->y > b->y) ? a->y : b->y;
			int x2_a = a->x + a->width;
			int x2_b = b->x + b->width;
			int x2 = (x2_a < x2_b) ? x2_a : x2_b;
			int y2_a = a->y + a->height;
			int y2_b = b->y + b->height;
			int y2 = (y2_a < y2_b) ? y2_a : y2_b;

			if (x1 < x2 && y1 < y2) {
				results[count++] = (struct wlf_rect){
					.x = x1,
					.y = y1,
					.width = x2 - x1,
					.height = y2 - y1
				};
			}
		}
	}

	wlf_region_fini(dst);
	wlf_region_init(dst);
	for (long i = 0; i < count; ++i) {
		wlf_region_add_rect(dst, &results[i]);
	}

	free(results);
}
