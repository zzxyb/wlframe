#include "wlf/math/wlf_region.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>

#include "wlf_region.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static struct wlf_rect rect_union(const struct wlf_rect *a, const struct wlf_rect *b) {
    struct wlf_rect out;
    out.x1 = a->x1 < b->x1 ? a->x1 : b->x1;
    out.y1 = a->y1 < b->y1 ? a->y1 : b->y1;
    out.x2 = a->x2 > b->x2 ? a->x2 : b->x2;
    out.y2 = a->y2 > b->y2 ? a->y2 : b->y2;
    return out;
}

struct wlf_region_t* wlf_region_create(void) {
    wlf_region_t *region = malloc(sizeof(wlf_region_t));
    if (region) {
        wlf_region_init(region);
    }
    return region;
}

void wlf_region_destroy(struct wlf_region_t *region) {
    if (region) {
        wlf_region_fini(region);
        free(region);
    }
}

void wlf_region_init(struct wlf_region_t *region) {
    region->data = malloc(sizeof(struct wlf_region_data));
    if (region->data) {
        region->data->size = 4;
        region->data->numRects = 0;
        region->data->rects = calloc(region->data->size, sizeof(struct wlf_rect));
    }
    region->extents = (struct wlf_rect){0, 0, 0, 0};
}

void wlf_region_fini(struct wlf_region_t *region) {
    if (region->data) {
        free(region->data->rects);
        free(region->data);
        region->data = NULL;
    }
}

bool wlf_region_is_nil(const struct wlf_region_t *region) {
    return !region->data || region->data->numRects == 0;
}

bool wlf_region_add_rect(struct wlf_region_t *region, const struct wlf_rect *rect) {
    if (!region->data) {
		return false;
	}

    if (region->data->numRects >= region->data->size) {
        long new_size = region->data->size * 2;
        struct wlf_rect *new_rects = realloc(region->data->rects, new_size * sizeof(struct wlf_rect));
        if (!new_rects) {
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

bool wlf_region_contains_point(const struct wlf_region_t *region, int x, int y) {
    if (!region->data) return false;

    for (long i = 0; i < region->data->numRects; ++i) {
        struct wlf_rect r = region->data->rects[i];
        if (x >= r.x1 && x < r.x2 && y >= r.y1 && y < r.y2) {
            return true;
        }
    }
    return false;
}

bool wlf_region_intersects_rect(const struct wlf_region_t *region, const struct wlf_rect *rect) {
    if (!region->data) return false;

    for (long i = 0; i < region->data->numRects; ++i) {
        const struct wlf_rect *r = &region->data->rects[i];
        if (!(rect->x2 <= r->x1 || rect->x1 >= r->x2 || rect->y2 <= r->y1 || rect->y1 >= r->y2)) {
            return true;
        }
    }
    return false;
}

void wlf_region_union(struct wlf_region_t *dst, const struct wlf_region_t *src) {
    if (!src->data) return;

    for (long i = 0; i < src->data->numRects; ++i) {
        wlf_region_add_rect(dst, &src->data->rects[i]);
    }
}

void wlf_region_intersect(struct wlf_region_t *dst, const struct wlf_region_t *src) {
    if (!dst->data || !src->data) return;

    struct wlf_rect *results = malloc(sizeof(struct wlf_rect) * dst->data->numRects * src->data->numRects);
    long count = 0;

    for (long i = 0; i < dst->data->numRects; ++i) {
        for (long j = 0; j < src->data->numRects; ++j) {
            const struct wlf_rect *a = &dst->data->rects[i];
            const struct wlf_rect *b = &src->data->rects[j];

            int x1 = a->x1 > b->x1 ? a->x1 : b->x1;
            int y1 = a->y1 > b->y1 ? a->y1 : b->y1;
            int x2 = a->x2 < b->x2 ? a->x2 : b->x2;
            int y2 = a->y2 < b->y2 ? a->y2 : b->y2;

            if (x1 < x2 && y1 < y2) {
                results[count++] = (struct wlf_rect){x1, y1, x2, y2};
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
