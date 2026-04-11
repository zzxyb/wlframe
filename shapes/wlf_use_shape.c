#include "wlf/shapes/wlf_use_shape.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void shape_destroy(struct wlf_shape *shape) {
	struct wlf_use_shape *use_shape = wlf_use_shape_from_shape(shape);
	free(use_shape);
}

static struct wlf_shape *shape_clone(struct wlf_shape *shape) {
	struct wlf_use_shape *src = wlf_use_shape_from_shape(shape);
	struct wlf_shape *clone = wlf_use_shape_create(src->href, src->x, src->y);

	return clone;
}

static const struct wlf_shape_impl shape_impl = {
	.destroy = shape_destroy,
	.clone = shape_clone,
};

struct wlf_shape *wlf_use_shape_create(const char *href, float x, float y) {
	struct wlf_use_shape *use = malloc(sizeof(*use));
	if (use == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_use_shape");
		return NULL;
	}

	wlf_shape_init(&use->base, &shape_impl);
	use->x = x;
	use->y = y;
	use->href[0] = '\0';
	strncpy(use->href, href, sizeof(use->href) - 1);
	use->href[sizeof(use->href) - 1] = '\0';

	return &use->base;
}

bool wlf_shape_is_use(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shape_impl;
}

struct wlf_use_shape *wlf_use_shape_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shape_impl);

	struct wlf_use_shape *use_shape =
		wlf_container_of(shape, use_shape, base);

	return use_shape;
}
