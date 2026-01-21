/**
 * @file        wlf_shm_allocator.c
 * @brief       Shared memory buffer allocator implementation.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 */

#include "wlf/allocator/wlf_shm_allocator.h"
#include "wlf/buffer/wlf_shm_buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>

static const struct wlf_allocator_impl allocator_impl;

static struct wlf_shm_allocator *get_shm_alloc_from_alloc(
		struct wlf_allocator *wlf_alloc) {
	assert(wlf_alloc->impl == &allocator_impl);
	return wlf_container_of(wlf_alloc, struct wlf_shm_allocator, base);
}

struct wlf_allocator *wlf_shm_allocator_create(void) {
	struct wlf_shm_allocator *alloc = calloc(1, sizeof(*alloc));
	if (alloc == NULL) {
		return NULL;
	}

	// Initialize base allocator
	alloc->base.impl = &allocator_impl;
	wlf_signal_init(&alloc->base.events.destroy);

	wlf_log(WLF_DEBUG, "Created SHM allocator");
	return &alloc->base;
}

/**
 * @brief Destroys a SHM allocator.
 */
static void allocator_destroy(struct wlf_allocator *wlf_alloc) {
	struct wlf_shm_allocator *alloc = get_shm_alloc_from_alloc(wlf_alloc);
	free(alloc);
}

/**
 * @brief Creates a buffer using the SHM allocator.
 *
 * Note: This simplified version uses DRM_FORMAT_XRGB8888 as default.
 * A more complete implementation should accept format parameters.
 */
static struct wlf_buffer *allocator_create_buffer(
		struct wlf_allocator *wlf_alloc, uint32_t width, uint32_t height) {
	struct wlf_shm_allocator *alloc = get_shm_alloc_from_alloc(wlf_alloc);

	// Use default format for now
	// TODO: Should be configurable based on requirements
	uint32_t format = DRM_FORMAT_XRGB8888;

	struct wlf_shm_buffer *buffer = wlf_shm_buffer_create(alloc, width, height, format);
	if (buffer == NULL) {
		return NULL;
	}
	return &buffer->base;
}

static const struct wlf_allocator_impl allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};

// Public API implementations

struct wlf_shm_allocator *wlf_shm_allocator_from_allocator(
		struct wlf_allocator *allocator) {
	if (allocator == NULL || allocator->impl != &allocator_impl) {
		return NULL;
	}
	return get_shm_alloc_from_alloc(allocator);
}

bool wlf_allocator_is_shm(struct wlf_allocator *allocator) {
	if (allocator == NULL) {
		return false;
	}
	return allocator->impl == &allocator_impl;
}
