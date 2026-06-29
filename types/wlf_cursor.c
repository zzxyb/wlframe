#include "wlf/types/wlf_cursor.h"

#include <assert.h>
#include <stddef.h>

void wlf_cursor_init(struct wlf_cursor *cursor,
		const struct wlf_cursor_impl *impl) {
	assert(cursor != NULL && impl != NULL && impl->destroy != NULL &&
		impl->set_shape != NULL);
	*cursor = (struct wlf_cursor){
		.impl = impl,
		.shape = WLF_CURSOR_SHAPE_DEFAULT,
	};
}

void wlf_cursor_destroy(struct wlf_cursor *cursor) {
	if (cursor != NULL) {
		cursor->impl->destroy(cursor);
	}
}

bool wlf_cursor_set_shape(struct wlf_cursor *cursor, uint32_t serial,
		enum wlf_cursor_shape shape) {
	if (cursor == NULL || serial == 0 || shape < WLF_CURSOR_SHAPE_DEFAULT ||
			shape > WLF_CURSOR_SHAPE_ALL_RESIZE) {
		return false;
	}
	if (!cursor->impl->set_shape(cursor, serial, shape)) {
		return false;
	}
	cursor->shape = shape;
	return true;
}
