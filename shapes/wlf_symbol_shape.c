#include "wlf/shapes/wlf_symbol_shape.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void shape_destroy(struct wlf_shape *shape) {
	struct wlf_symbol_shape *symbol_shape = wlf_symbol_shape_from_shape(shape);
	free(symbol_shape);
}

static struct wlf_shape *shape_clone(struct wlf_shape *shape) {
	struct wlf_symbol_shape *src = wlf_symbol_shape_from_shape(shape);
	struct wlf_shape *clone = wlf_symbol_shape_create(src->id);

	return clone;
}

static const struct wlf_shape_impl shape_impl = {
	.destroy = shape_destroy,
	.clone = shape_clone,
};

struct wlf_shape *wlf_symbol_shape_create(const char *id) {
	struct wlf_symbol_shape *symbol = malloc(sizeof(*symbol));
	if (symbol == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_symbol_shape");
		return NULL;
	}

	wlf_shape_init(&symbol->base, &shape_impl);
	symbol->id[0] = '\0';
	strncpy(symbol->id, id, sizeof(symbol->id) - 1);
	symbol->id[sizeof(symbol->id) - 1] = '\0';

	return &symbol->base;
}

bool wlf_shape_is_symbol(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shape_impl;
}

struct wlf_symbol_shape *wlf_symbol_shape_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shape_impl);

	struct wlf_symbol_shape *symbol_shape =
		wlf_container_of(shape, symbol_shape, base);

	return symbol_shape;
}
