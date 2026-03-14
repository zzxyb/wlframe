#include "wlf/allocator/vulkan/allocator.h"
#include "wlf/utils/wlf_signal.h"

#include <stdlib.h>
#include <assert.h>

static void allocator_destroy(struct wlf_allocator *wlf_alloc) {
	struct wlf_vk_allocator *alloc =
		wlf_vk_allocator_from_allocator(wlf_alloc);
	free(alloc);
}

static struct wlf_buffer *allocator_create_buffer(
		struct wlf_allocator *wlf_alloc, uint32_t width, uint32_t height
		, const struct wlf_format_set *format_set) {
	return NULL;
}

static const struct wlf_allocator_impl allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};

struct wlf_allocator *wlf_vk_allocator_create(void) {
	struct wlf_vk_allocator *allocator = calloc(1, sizeof(*allocator));
	if (allocator == NULL) {
		return NULL;
	}

	wlf_allocator_init(&allocator->base, &allocator_impl);
	wlf_signal_init(&allocator->base.events.destroy);

	return &allocator->base;
}

struct wlf_vk_allocator *wlf_vk_allocator_from_allocator(
		struct wlf_allocator *allocator) {
	assert(allocator->impl == &allocator_impl);
	struct wlf_vk_allocator *vk_allocator =
		wlf_container_of(allocator, vk_allocator, base);

	return vk_allocator;
}

bool wlf_allocator_is_vk(const struct wlf_allocator *allocator) {
	return allocator->impl == &allocator_impl;
}
