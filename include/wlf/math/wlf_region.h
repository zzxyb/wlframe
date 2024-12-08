#ifndef WLF_MATH_REGION_H
#define WLF_MATH_REGION_H

#include "wlf_rect.h"
#include <stddef.h>
#include <stdbool.h>

struct wlf_rect;

struct wlf_region_data {
    long size;
    long numRects;
    struct wlf_rect *rects;
};

struct wlf_region {
    struct wlf_rect extents;
    struct wlf_region_data *data;
};

struct wlf_region_t* wlf_region_create(void);
void wlf_region_destroy(struct wlf_region_t *region);

void wlf_region_init(struct wlf_region_t *region);
void wlf_region_fini(struct wlf_region_t *region);

bool wlf_region_is_nil(const struct wlf_region_t *region);
bool wlf_region_add_rect(struct wlf_region_t *region, const struct wlf_rect *rect);
bool wlf_region_contains_point(const struct wlf_region_t *region, int x, int y);
bool wlf_region_intersects_rect(const struct wlf_region_t *region, const struct wlf_rect *rect);

void wlf_region_union(struct wlf_region_t *dst, const struct wlf_region_t *src);
void wlf_region_intersect(struct wlf_region_t *dst, const struct wlf_region_t *src);

#endif