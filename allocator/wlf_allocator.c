#include "wlf/allocator/wlf_allocator.h"

#include <assert.h>
#include <stdlib.h>

void wlf_allocator_init(struct wlf_allocator *allocator,
		const struct wlf_allocator_impl *impl) {
	assert(impl);
	assert(impl->destroy);

	allocator->impl = impl;

	wlf_signal_init(&allocator->events.destroy);
}

struct wlf_allocator *wlf_allocator_autocreate(struct wlf_backend *backend,
		struct wlf_renderer *renderer) {
	return NULL;
}

void wlf_allocator_destroy(struct wlf_allocator *allocator) {
	if (allocator == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&allocator->events.destroy, allocator);

	assert(wlf_linked_list_empty(&allocator->events.destroy.listener_list));

	if (allocator->impl && allocator->impl->destroy) {
		allocator->impl->destroy(allocator);
	} else {
		free(allocator);
	}
}

struct wlf_buffer *wlf_allocator_create_buffer(struct wlf_allocator *allocator,
		uint32_t width, uint32_t height) {
	struct wlf_buffer *buffer =
		allocator->impl->create_buffer(allocator, width, height);

	return buffer;
}
