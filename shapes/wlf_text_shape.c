#include "wlf/shapes/wlf_text_shape.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void shape_destroy(struct wlf_shape *shape) {
	struct wlf_text_shape *text_shape = wlf_text_shape_from_shape(shape);
	free(text_shape);
}

static struct wlf_shape *shape_clone(struct wlf_shape *shape) {
	struct wlf_text_shape *src = wlf_text_shape_from_shape(shape);
	struct wlf_shape *clone = wlf_text_shape_create(src->x, src->y,
		src->text, src->font_family, src->font_size, src->text_anchor);
	if (clone != NULL) {
		wlf_text_shape_from_shape(clone)->state = src->state;
	}
	return clone;
}

static const struct wlf_shape_impl shape_impl = {
	.destroy = shape_destroy,
	.clone = shape_clone,
};

struct wlf_shape *wlf_text_shape_create(float x, float y,
	const char *text, const char *font_family, float font_size,
	enum wlf_text_anchor text_anchor) {
	struct wlf_text_shape *shape = malloc(sizeof(*shape));
	if (shape == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_text_shape");
		return NULL;
	}

	wlf_shape_init(&shape->base, &shape_impl);
	shape->x = x;
	shape->y = y;
	shape->font_size = font_size;
	shape->text_anchor = text_anchor;
	shape->font_family[0] = '\0';
	shape->text[0] = '\0';

	if (font_family != NULL) {
		strncpy(shape->font_family, font_family, sizeof(shape->font_family) - 1);
		shape->font_family[sizeof(shape->font_family) - 1] = '\0';
	} else {
		strncpy(shape->font_family, "sans-serif", sizeof(shape->font_family) - 1);
		shape->font_family[sizeof(shape->font_family) - 1] = '\0';
	}

	if (text != NULL) {
		strncpy(shape->text, text, sizeof(shape->text) - 1);
		shape->text[sizeof(shape->text) - 1] = '\0';
	}

	wlf_shape_state_init(&shape->state);
	shape->state.has_stroke = false;

	return &shape->base;
}

bool wlf_shape_is_text(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shape_impl;
}

struct wlf_text_shape *wlf_text_shape_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shape_impl);

	struct wlf_text_shape *text_shape =
		wlf_container_of(shape, text_shape, base);

	return text_shape;
}
