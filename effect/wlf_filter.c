#include "wlf/effect/wlf_filter.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static bool filter_contains(const struct wlf_filter *filter,
		const struct wlf_shape *effect) {
	const struct wlf_shape *iter;
	wlf_linked_list_for_each(iter, &filter->effects, link) {
		if (iter == effect) return true;
	}
	return false;
}

static void filter_destroy(struct wlf_shape *shape) {
	struct wlf_filter *filter = wlf_filter_from_shape(shape);
	struct wlf_shape *iter, *tmp;
	wlf_linked_list_for_each_safe(iter, tmp, &filter->effects, link) {
		wlf_linked_list_remove(&iter->link);
		wlf_shape_destroy(iter);
	}
	free(filter);
}

static struct wlf_shape *filter_clone(struct wlf_shape *shape) {
	struct wlf_filter *src = wlf_filter_from_shape(shape);
	struct wlf_filter *dst = wlf_filter_create(src->id);
	struct wlf_shape *iter;
	if (dst == NULL) return NULL;
	dst->units = src->units;
	dst->primitive_units = src->primitive_units;
	dst->x = src->x;
	dst->y = src->y;
	dst->width = src->width;
	dst->height = src->height;
	wlf_linked_list_for_each(iter, &src->effects, link) {
		struct wlf_shape *copy = wlf_shape_clone(iter);
		if (copy == NULL) {
			wlf_shape_destroy(&dst->base);
			return NULL;
		}
		wlf_filter_add(dst, copy);
	}
	return &dst->base;
}

static const struct wlf_shape_impl filter_impl = {
	.destroy = filter_destroy,
	.clone = filter_clone,
};

struct wlf_filter *wlf_filter_create(const char *id) {
	struct wlf_filter *filter = calloc(1, sizeof(*filter));
	if (filter == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_filter");
		return NULL;
	}

	wlf_shape_init(&filter->base, &filter_impl);
	wlf_linked_list_init(&filter->effects);
	filter->units = WLF_FILTER_UNITS_OBJECT_BOUNDING_BOX;
	filter->primitive_units = WLF_FILTER_UNITS_USER_SPACE;
	filter->x = -0.1f;
	filter->y = -0.1f;
	filter->width = 1.2f;
	filter->height = 1.2f;
	if (id != NULL) {
		strncpy(filter->id, id, sizeof(filter->id) - 1);
	}
	return filter;
}

void wlf_filter_add(struct wlf_filter *filter, struct wlf_shape *effect) {
	assert(filter);
	assert(effect);
	if (effect == &filter->base ||
			(effect->link.prev != NULL && effect->link.prev != &effect->link) ||
			(effect->link.next != NULL && effect->link.next != &effect->link)) return;
	wlf_linked_list_insert(filter->effects.prev, &effect->link);
}

void wlf_filter_remove(struct wlf_filter *filter, struct wlf_shape *effect) {
	assert(filter);
	assert(effect);
	if (filter_contains(filter, effect)) wlf_linked_list_remove(&effect->link);
}

int wlf_filter_effect_count(const struct wlf_filter *filter) {
	assert(filter);
	return wlf_linked_list_length(&filter->effects);
}

bool wlf_shape_is_filter(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &filter_impl;
}

struct wlf_filter *wlf_filter_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &filter_impl);
	return wlf_container_of(shape, (struct wlf_filter *)NULL, base);
}
