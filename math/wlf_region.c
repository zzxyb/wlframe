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

static bool frect_intersect(const struct wlf_frect *a, const struct wlf_frect *b,
		struct wlf_frect *out) {
	if (a == NULL || b == NULL || out == NULL) {
		return false;
	}

	double x1 = (a->x > b->x) ? a->x : b->x;
	double y1 = (a->y > b->y) ? a->y : b->y;
	double x2_a = a->x + a->width;
	double x2_b = b->x + b->width;
	double x2 = (x2_a < x2_b) ? x2_a : x2_b;
	double y2_a = a->y + a->height;
	double y2_b = b->y + b->height;
	double y2 = (y2_a < y2_b) ? y2_a : y2_b;

	if (x1 >= x2 || y1 >= y2) {
		return false;
	}

	*out = (struct wlf_frect){
		.x = x1,
		.y = y1,
		.width = x2 - x1,
		.height = y2 - y1,
	};
	return true;
}

static bool region_append_subtract_rect(struct wlf_region *dst, const struct wlf_frect *base,
		const struct wlf_frect *cut) {
	if (dst == NULL || base == NULL) {
		return false;
	}

	struct wlf_frect overlap;
	if (cut == NULL || !frect_intersect(base, cut, &overlap)) {
		return wlf_region_add_rect(dst, base);
	}

	double base_right = base->x + base->width;
	double base_bottom = base->y + base->height;
	double overlap_right = overlap.x + overlap.width;
	double overlap_bottom = overlap.y + overlap.height;

	struct wlf_frect pieces[4];
	long piece_count = 0;

	if (base->y < overlap.y) {
		pieces[piece_count++] = (struct wlf_frect){
			.x = base->x,
			.y = base->y,
			.width = base->width,
			.height = overlap.y - base->y,
		};
	}

	if (overlap_bottom < base_bottom) {
		pieces[piece_count++] = (struct wlf_frect){
			.x = base->x,
			.y = overlap_bottom,
			.width = base->width,
			.height = base_bottom - overlap_bottom,
		};
	}

	if (base->x < overlap.x) {
		pieces[piece_count++] = (struct wlf_frect){
			.x = base->x,
			.y = overlap.y,
			.width = overlap.x - base->x,
			.height = overlap.height,
		};
	}

	if (overlap_right < base_right) {
		pieces[piece_count++] = (struct wlf_frect){
			.x = overlap_right,
			.y = overlap.y,
			.width = base_right - overlap_right,
			.height = overlap.height,
		};
	}

	for (long i = 0; i < piece_count; ++i) {
		if (pieces[i].width <= 0.0 || pieces[i].height <= 0.0) {
			continue;
		}
		if (!wlf_region_add_rect(dst, &pieces[i])) {
			return false;
		}
	}

	return true;
}

static void region_clear_extents(struct wlf_region *region) {
	if (region == NULL) {
		return;
	}

	region->extents = (struct wlf_frect){
		.x = 0,
		.y = 0,
		.width = 0,
		.height = 0,
	};
}

void wlf_region_init(struct wlf_region *region) {
	if (region == NULL) {
		return;
	}

	region_clear_extents(region);
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
	region_clear_extents(region);
}

void wlf_region_clear(struct wlf_region *region) {
	if (region == NULL || region->data == NULL) {
		return;
	}

	region->data->numRects = 0;
	region_clear_extents(region);
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

	struct wlf_region parsed;
	wlf_region_init(&parsed);
	if (parsed.data == NULL) {
		return false;
	}

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
			wlf_region_fini(&parsed);
			return false;
		}

		struct wlf_frect rect = {
			.x = x,
			.y = y,
			.width = w,
			.height = h,
		};
		if (!wlf_region_add_rect(&parsed, &rect)) {
			wlf_region_fini(&parsed);
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

	*out_region = parsed;
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
	return region == NULL || region->data == NULL || region->data->numRects == 0;
}

bool wlf_region_not_empty(const struct wlf_region *region) {
	return !wlf_region_is_nil(region);
}

bool wlf_region_add_rect(struct wlf_region *region, const struct wlf_frect *rect) {
	if (region == NULL || rect == NULL || region->data == NULL) {
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
	if (region == NULL || region->data == NULL) {
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

enum wlf_region_overlap wlf_region_contains_rect(const struct wlf_region *region,
		const struct wlf_frect *rect) {
	if (region == NULL || rect == NULL || region->data == NULL || region->data->numRects == 0) {
		return WLF_REGION_OUT;
	}

	struct wlf_region remaining;
	wlf_region_init(&remaining);
	if (remaining.data == NULL) {
		return WLF_REGION_OUT;
	}
	if (!wlf_region_add_rect(&remaining, rect)) {
		wlf_region_fini(&remaining);
		return WLF_REGION_OUT;
	}

	bool any_overlap = false;

	for (long i = 0; i < region->data->numRects; ++i) {
		struct wlf_region next;
		wlf_region_init(&next);
		if (next.data == NULL) {
			wlf_region_fini(&remaining);
			return WLF_REGION_PART;
		}

		const struct wlf_frect *cover = &region->data->rects[i];
		for (long j = 0; j < remaining.data->numRects; ++j) {
			const struct wlf_frect *piece = &remaining.data->rects[j];
			struct wlf_frect overlap;
			if (frect_intersect(piece, cover, &overlap)) {
				any_overlap = true;
			}

			if (!region_append_subtract_rect(&next, piece, cover)) {
				wlf_region_fini(&remaining);
				wlf_region_fini(&next);
				return WLF_REGION_PART;
			}
		}

		wlf_region_fini(&remaining);
		remaining = next;
		if (remaining.data->numRects == 0) {
			wlf_region_fini(&remaining);
			return WLF_REGION_IN;
		}
	}

	enum wlf_region_overlap overlap = any_overlap ? WLF_REGION_PART : WLF_REGION_OUT;
	wlf_region_fini(&remaining);
	return overlap;
}

void wlf_region_intersects_rect(const struct wlf_region *region, const struct wlf_frect *rect,
		struct wlf_region *result) {
	if (result == NULL) {
		return;
	}

	struct wlf_region intersection;
	wlf_region_init(&intersection);
	if (intersection.data == NULL) {
		return;
	}

	if (region == NULL || rect == NULL) {
		wlf_region_fini(result);
		*result = intersection;
		return;
	}

	struct wlf_region rect_region;
	wlf_region_init(&rect_region);
	if (rect_region.data == NULL) {
		wlf_region_fini(&intersection);
		return;
	}
	if (!wlf_region_add_rect(&rect_region, rect)) {
		wlf_region_fini(&rect_region);
		wlf_region_fini(&intersection);
		return;
	}

	wlf_region_intersect(region, &rect_region, &intersection);
	wlf_region_fini(&rect_region);

	wlf_region_fini(result);
	*result = intersection;
}

bool wlf_region_union(struct wlf_region *new_region, const struct wlf_region *reg1,
		const struct wlf_region *reg2) {
	if (new_region == NULL) {
		return false;
	}

	struct wlf_region merged;
	wlf_region_init(&merged);
	if (merged.data == NULL) {
		return false;
	}

	const struct wlf_region *regions[] = {reg1, reg2};
	for (size_t region_index = 0; region_index < sizeof(regions) / sizeof(regions[0]); ++region_index) {
		const struct wlf_region *region = regions[region_index];
		if (region == NULL || region->data == NULL) {
			continue;
		}

		for (long rect_index = 0; rect_index < region->data->numRects; ++rect_index) {
			if (!wlf_region_add_rect(&merged, &region->data->rects[rect_index])) {
				wlf_region_fini(&merged);
				return false;
			}
		}
	}

	wlf_region_fini(new_region);
	*new_region = merged;
	return true;
}

void wlf_region_intersect(const struct wlf_region *dst, const struct wlf_region *src, struct wlf_region *result) {
	if (result == NULL) {
		return;
	}

	struct wlf_region intersection;
	wlf_region_init(&intersection);
	if (intersection.data == NULL) {
		return;
	}

	if (dst == NULL || src == NULL || dst->data == NULL || src->data == NULL) {
		wlf_region_fini(result);
		*result = intersection;
		return;
	}

	struct wlf_frect *results_rect = malloc(sizeof(struct wlf_frect) * dst->data->numRects * src->data->numRects);
	if (!results_rect) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for intersection results_rect");
		wlf_region_fini(&intersection);
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
		if (!wlf_region_add_rect(&intersection, &results_rect[i])) {
			free(results_rect);
			wlf_region_fini(&intersection);
			return;
		}
	}

	free(results_rect);
	wlf_region_fini(result);
	*result = intersection;
}

bool wlf_region_subtract(struct wlf_region *dst, const struct wlf_region *minuend,
		const struct wlf_region *subtrahend) {
	if (dst == NULL) {
		return false;
	}

	struct wlf_region result;
	wlf_region_init(&result);
	if (result.data == NULL) {
		return false;
	}

	if (minuend == NULL || minuend->data == NULL || minuend->data->numRects == 0) {
		wlf_region_fini(dst);
		*dst = result;
		return true;
	}

	for (long i = 0; i < minuend->data->numRects; ++i) {
		struct wlf_region pieces;
		wlf_region_init(&pieces);
		if (pieces.data == NULL) {
			wlf_region_fini(&result);
			return false;
		}
		if (!wlf_region_add_rect(&pieces, &minuend->data->rects[i])) {
			wlf_region_fini(&pieces);
			wlf_region_fini(&result);
			return false;
		}

		if (subtrahend != NULL && subtrahend->data != NULL) {
			for (long j = 0; j < subtrahend->data->numRects; ++j) {
				struct wlf_region next;
				wlf_region_init(&next);
				if (next.data == NULL) {
					wlf_region_fini(&pieces);
					wlf_region_fini(&result);
					return false;
				}

				for (long k = 0; k < pieces.data->numRects; ++k) {
					if (!region_append_subtract_rect(&next, &pieces.data->rects[k],
							&subtrahend->data->rects[j])) {
						wlf_region_fini(&next);
						wlf_region_fini(&pieces);
						wlf_region_fini(&result);
						return false;
					}
				}

				wlf_region_fini(&pieces);
				pieces = next;
				if (pieces.data->numRects == 0) {
					break;
				}
			}
		}

		for (long j = 0; j < pieces.data->numRects; ++j) {
			if (!wlf_region_add_rect(&result, &pieces.data->rects[j])) {
				wlf_region_fini(&pieces);
				wlf_region_fini(&result);
				return false;
			}
		}

		wlf_region_fini(&pieces);
	}

	wlf_region_fini(dst);
	*dst = result;
	return true;
}

void wlf_region_init_rect(struct wlf_region *region, const struct wlf_frect *rect) {
	if (region == NULL) {
		return;
	}

	wlf_region_init(region);
	if (rect == NULL || region->data == NULL) {
		return;
	}

	wlf_region_add_rect(region, rect);
}

void wlf_region_union_rect(struct wlf_region *dst, struct wlf_region *src, const struct wlf_frect *rect) {
	if (dst == NULL) {
		return;
	}

	struct wlf_region merged;
	wlf_region_init(&merged);
	if (merged.data == NULL) {
		return;
	}

	if (src != NULL && src->data != NULL) {
		for (long i = 0; i < src->data->numRects; ++i) {
			if (!wlf_region_add_rect(&merged, &src->data->rects[i])) {
				wlf_region_fini(&merged);
				return;
			}
		}
	}

	if (rect != NULL && !wlf_region_add_rect(&merged, rect)) {
		wlf_region_fini(&merged);
		return;
	}

	wlf_region_fini(dst);
	*dst = merged;
}

void wlf_region_translate(struct wlf_region *region, double dx, double dy) {
	if (region == NULL || region->data == NULL) {
		return;
	}

	for (long i = 0; i < region->data->numRects; ++i) {
		region->data->rects[i].x += dx;
		region->data->rects[i].y += dy;
	}

	if (region->data->numRects > 0) {
		region->extents.x += dx;
		region->extents.y += dy;
	}
}

bool wlf_region_reset(struct wlf_region *region, const struct wlf_frect *rect) {
	if (region == NULL || region->data == NULL) {
		return false;
	}

	wlf_region_clear(region);
	if (rect == NULL) {
		return true;
	}

	return wlf_region_add_rect(region, rect);
}

long wlf_region_n_rects(const struct wlf_region *region) {
	if (region == NULL || region->data == NULL) {
		return 0;
	}

	return region->data->numRects;
}

const struct wlf_frect *wlf_region_rectangles(const struct wlf_region *region, long *n_rects) {
	long count = wlf_region_n_rects(region);
	if (n_rects != NULL) {
		*n_rects = count;
	}

	if (count == 0) {
		return NULL;
	}

	return region->data->rects;
}

bool wlf_region_equal(const struct wlf_region *region1, const struct wlf_region *region2) {
	long count1 = wlf_region_n_rects(region1);
	long count2 = wlf_region_n_rects(region2);
	if (count1 != count2) {
		return false;
	}
	if (count1 == 0) {
		return true;
	}

	for (long i = 0; i < count1; ++i) {
		if (memcmp(&region1->data->rects[i], &region2->data->rects[i],
				sizeof(struct wlf_frect)) != 0) {
			return false;
		}
	}

	return memcmp(&region1->extents, &region2->extents, sizeof(struct wlf_frect)) == 0;
}

bool wlf_region_copy(struct wlf_region *dst, const struct wlf_region *src) {
	if (dst == NULL) {
		return false;
	}

	struct wlf_region copied;
	wlf_region_init(&copied);
	if (copied.data == NULL) {
		return false;
	}

	if (src != NULL && src->data != NULL) {
		for (long i = 0; i < src->data->numRects; ++i) {
			if (!wlf_region_add_rect(&copied, &src->data->rects[i])) {
				wlf_region_fini(&copied);
				return false;
			}
		}
	}

	wlf_region_fini(dst);
	*dst = copied;
	return true;
}
