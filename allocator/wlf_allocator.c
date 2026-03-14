#include "wlf/allocator/wlf_allocator.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <wayland-client-protocol.h>

void wlf_allocator_init(struct wlf_allocator *allocator,
		const struct wlf_allocator_impl *impl) {
	assert(impl && impl->destroy && impl->create_buffer);
	*allocator = (struct wlf_allocator){
		.impl = impl,
	};

	wlf_signal_init(&allocator->events.destroy);
}

void wlf_allocator_destroy(struct wlf_allocator *allocator) {
	if (allocator == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&allocator->events.destroy, NULL);

	assert(wlf_linked_list_empty(&allocator->events.destroy.listener_list));

	allocator->impl->destroy(allocator);
}

struct wlf_allocator *wlf_allocator_autocreate(struct wlf_backend *backend,
		struct wlf_renderer *renderer) {
	WLF_UNUSED(backend);
	WLF_UNUSED(renderer);
	struct wlf_allocator *allocator = NULL;
	return allocator;
}

struct wlf_buffer *wlf_allocator_create_buffer(struct wlf_allocator *allocator,
		uint32_t width, uint32_t height, const struct wlf_format_set *format) {
	struct wlf_buffer *buffer =
		allocator->impl->create_buffer(allocator, width, height, format);
	return buffer;
}
