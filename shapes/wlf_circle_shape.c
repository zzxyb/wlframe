#include "wlf/shapes/wlf_circle_shape.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static void shape_destroy(struct wlf_shape *shape) {
	struct wlf_circle_shape *circle_shape = wlf_circle_shape_from_shape(shape);
	free(circle_shape);
}

static struct wlf_shape *shape_clone(struct wlf_shape *shape) {
	struct wlf_circle_shape *circle_shape = wlf_circle_shape_from_shape(shape);
	struct wlf_shape *clone =
		wlf_circle_shape_create(circle_shape->cx, circle_shape->cy, circle_shape->r);
	if (clone != NULL) {
		wlf_circle_shape_from_shape(clone)->state = circle_shape->state;
	}

	return clone;
}

static const struct wlf_shape_impl shape_impl = {
	.destroy = shape_destroy,
	.clone = shape_clone,
};

struct wlf_shape *wlf_circle_shape_create(float cx, float cy, float r) {
	struct wlf_circle_shape *circle = malloc(sizeof(*circle));
	if (circle == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_circle_shape");
		return NULL;
	}

	wlf_shape_init(&circle->base, &shape_impl);
	circle->cx = cx;
	circle->cy = cy;
	circle->r = r;
	wlf_shape_state_init(&circle->state);

	return &circle->base;
}

bool wlf_shape_is_circle(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shape_impl;
}

struct wlf_circle_shape *wlf_circle_shape_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shape_impl);

	struct wlf_circle_shape *circle_shape =
		wlf_container_of(shape, circle_shape, base);

	return circle_shape;
}
