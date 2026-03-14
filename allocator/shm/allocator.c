#include "wlf/allocator/shm/allocator.h"
#include "wlf/buffer/shm/buffer.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>

static void allocator_destroy(struct wlf_allocator *wlf_alloc) {
	struct wlf_shm_allocator *alloc =
		wlf_shm_allocator_from_allocator(wlf_alloc);
	free(alloc);
}

static struct wlf_buffer *allocator_create_buffer(
		struct wlf_allocator *wlf_alloc, uint32_t width, uint32_t height
		, const struct wlf_format_set *format_set) {
	struct wlf_shm_allocator *alloc = wlf_shm_allocator_from_allocator(wlf_alloc);
	uint32_t drm_format = DRM_FORMAT_XRGB8888;

	if (format_set != NULL && format_set->len > 0) {
		const struct wlf_format *preferred =
			wlf_format_set_get(format_set, DRM_FORMAT_XRGB8888);
		drm_format = preferred != NULL ? preferred->format : format_set->formats[0].format;
	}

	struct wlf_shm_buffer *buffer =
		wlf_shm_buffer_create(alloc, width, height, drm_format);
	if (buffer == NULL) {
		return NULL;
	}
	return &buffer->base;
}

static const struct wlf_allocator_impl allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};

struct wlf_allocator *wlf_shm_allocator_create(void) {
	struct wlf_shm_allocator *allocator = calloc(1, sizeof(*allocator));
	if (allocator == NULL) {
		return NULL;
	}

	wlf_allocator_init(&allocator->base, &allocator_impl);
	wlf_signal_init(&allocator->base.events.destroy);

	return &allocator->base;
}

struct wlf_shm_allocator *wlf_shm_allocator_from_allocator(
		struct wlf_allocator *wlf_allocator) {
	assert(wlf_allocator->impl == &allocator_impl);
	struct wlf_shm_allocator *allocator =
		wlf_container_of(wlf_allocator, allocator, base);
	return allocator;
}

bool wlf_allocator_is_shm(const struct wlf_allocator *allocator) {
	return allocator->impl == &allocator_impl;
}
