#include "wlf/shapes/wlf_line_shape.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static void shape_destroy(struct wlf_shape *shape) {
	struct wlf_line_shape *line_shape = wlf_line_shape_from_shape(shape);
	free(line_shape);
}

static struct wlf_shape *shape_clone(struct wlf_shape *shape) {
	struct wlf_line_shape *line_shape = wlf_line_shape_from_shape(shape);
	struct wlf_shape *clone =
		wlf_line_shape_create(line_shape->x1, line_shape->y1, line_shape->x2, line_shape->y2);
	if (clone != NULL) {
		wlf_line_shape_from_shape(clone)->state = line_shape->state;
	}
	return clone;
}

static const struct wlf_shape_impl shape_impl = {
	.destroy = shape_destroy,
	.clone = shape_clone,
};

struct wlf_shape *wlf_line_shape_create(float x1, float y1, float x2, float y2) {
	struct wlf_line_shape *line = malloc(sizeof(*line));
	if (line == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_line_shape");
		return NULL;
	}

	wlf_shape_init(&line->base, &shape_impl);
	line->x1 = x1;
	line->y1 = y1;
	line->x2 = x2;
	line->y2 = y2;
	wlf_shape_state_init(&line->state);

	return &line->base;
}

bool wlf_shape_is_line(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shape_impl;
}

struct wlf_line_shape *wlf_line_shape_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shape_impl);

	struct wlf_line_shape *line_shape =
		wlf_container_of(shape, line_shape, base);

	return line_shape;
}
