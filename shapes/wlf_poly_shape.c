#include "wlf/shapes/wlf_poly_shape.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void shape_destroy(struct wlf_shape *shape) {
	struct wlf_poly_shape *poly_shape = wlf_poly_shape_from_shape(shape);
	free(poly_shape->points);
	free(poly_shape);
}

static struct wlf_shape *shape_clone(struct wlf_shape *shape) {
	struct wlf_poly_shape *poly_shape = wlf_poly_shape_from_shape(shape);
	struct wlf_shape *clone =
		wlf_poly_shape_create(poly_shape->points, poly_shape->count, poly_shape->closed);
	if (clone != NULL) {
		wlf_poly_shape_from_shape(clone)->state = poly_shape->state;
	}
	return clone;
}

static const struct wlf_shape_impl shape_impl = {
	.destroy = shape_destroy,
	.clone = shape_clone,
};

struct wlf_shape *wlf_poly_shape_create(const float *points, int count, bool closed) {
	struct wlf_poly_shape *poly = NULL;

	if (count <= 0 || points == NULL) {
		return NULL;
	}

	poly = malloc(sizeof(*poly));
	if (poly == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_poly_shape");
		return NULL;
	}

	poly->points = malloc((size_t)count * 2 * sizeof(float));
	if (poly->points == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_poly_shape points");
		free(poly);
		return NULL;
	}

	memcpy(poly->points, points, (size_t)count * 2 * sizeof(float));
	wlf_shape_init(&poly->base, &shape_impl);
	poly->count = count;
	poly->closed = closed;
	wlf_shape_state_init(&poly->state);

	return &poly->base;
}

bool wlf_shape_is_poly(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shape_impl;
}

struct wlf_poly_shape *wlf_poly_shape_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shape_impl);

	struct wlf_poly_shape *poly_shape =
		wlf_container_of(shape, poly_shape, base);

	return poly_shape;
}
