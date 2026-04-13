#include "wlf/shapes/wlf_rect_shape.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static void shape_destroy(struct wlf_shape *shape) {
	struct wlf_rect_shape *rect_shape = wlf_rect_shape_from_shape(shape);
	free(rect_shape);
}

static struct wlf_shape *shape_clone(struct wlf_shape *shape) {
	struct wlf_rect_shape *src = wlf_rect_shape_from_shape(shape);
	struct wlf_shape *clone =
		wlf_rect_shape_create(src->x, src->y, src->width, src->height, src->rx, src->ry);
	if (clone != NULL) {
		wlf_rect_shape_from_shape(clone)->state = src->state;
	}
	return clone;
}

static const struct wlf_shape_impl shape_impl = {
	.destroy = shape_destroy,
	.clone = shape_clone,
};

struct wlf_shape *wlf_rect_shape_create(float x, float y, float width, float height, float rx, float ry) {
	struct wlf_rect_shape *rect = malloc(sizeof(*rect));
	if (rect == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_rect_shape");
		return NULL;
	}

	wlf_shape_init(&rect->base, &shape_impl);
	rect->x = x;
	rect->y = y;
	rect->width = width;
	rect->height = height;
	rect->rx = rx;
	rect->ry = ry;
	wlf_shape_state_init(&rect->state);

	return &rect->base;
}

bool wlf_shape_is_rect(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shape_impl;
}

struct wlf_rect_shape *wlf_rect_shape_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shape_impl);

	struct wlf_rect_shape *rect_shape =
		wlf_container_of(shape, rect_shape, base);

	return rect_shape;
}
