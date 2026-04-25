#include "wlf/allocator/shm/allocator.h"
#include "wlf/buffer/shm/buffer.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>

static void allocator_destroy(struct wlf_allocator *allocator) {
	struct wlf_shm_allocator *shm_allocator =
		wlf_shm_allocator_from_allocator(allocator);
	free(shm_allocator);
}

static struct wlf_buffer *allocator_create_buffer(
		struct wlf_allocator *allocator, uint32_t width, uint32_t height) {
	struct wlf_shm_allocator *shm_allocator =
		wlf_shm_allocator_from_allocator(allocator);

	// Use default format for now
	// TODO: Should be configurable based on requirements
	uint32_t format = DRM_FORMAT_XRGB8888;

	struct wlf_shm_buffer *buffer = wlf_shm_buffer_create(shm_allocator, width, height, format);

	return &buffer->base;
}

static const struct wlf_allocator_impl allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};

struct wlf_allocator *wlf_shm_allocator_create(void) {
	struct wlf_shm_allocator *allocator = calloc(1, sizeof(*allocator));
	if (allocator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_shm_allocator");
		return NULL;
	}

	wlf_allocator_init(&allocator->base, &allocator_impl);

	return &allocator->base;
}

bool wlf_allocator_is_shm(const struct wlf_allocator *allocator) {
	return allocator->impl == &allocator_impl;
}

struct wlf_shm_allocator *wlf_shm_allocator_from_allocator(
		struct wlf_allocator *allocator) {
	assert(allocator->impl == &allocator_impl);

	struct wlf_shm_allocator *shm_allocator =
		wlf_container_of(allocator, shm_allocator, base);

	return shm_allocator;
}
