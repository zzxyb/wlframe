#include "wlf/shapes/wlf_ellipse_shape.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static void shape_destroy(struct wlf_shape *shape) {
	struct wlf_ellipse_shape *ellipse_shape = wlf_ellipse_shape_from_shape(shape);
	free(ellipse_shape);
}

static struct wlf_shape *shape_clone(struct wlf_shape *shape) {
	struct wlf_ellipse_shape *ellipse_shape = wlf_ellipse_shape_from_shape(shape);
	struct wlf_shape *clone =
		wlf_ellipse_shape_create(ellipse_shape->cx, ellipse_shape->cy,
			ellipse_shape->rx, ellipse_shape->ry);
	if (clone != NULL) {
		wlf_ellipse_shape_from_shape(clone)->state = ellipse_shape->state;
	}

	return clone;
}

static const struct wlf_shape_impl shape_impl = {
	.destroy = shape_destroy,
	.clone = shape_clone,
};

struct wlf_shape *wlf_ellipse_shape_create(float cx, float cy, float rx, float ry) {
	struct wlf_ellipse_shape *ellipse = malloc(sizeof(*ellipse));
	if (ellipse == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_ellipse_shape");
		return NULL;
	}

	wlf_shape_init(&ellipse->base, &shape_impl);
	ellipse->cx = cx;
	ellipse->cy = cy;
	ellipse->rx = rx;
	ellipse->ry = ry;
	wlf_shape_state_init(&ellipse->state);

	return &ellipse->base;
}

bool wlf_shape_is_ellipse(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shape_impl;
}

struct wlf_ellipse_shape *wlf_ellipse_shape_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shape_impl);

	struct wlf_ellipse_shape *ellipse_shape =
		wlf_container_of(shape, ellipse_shape, base);

	return ellipse_shape;
}
