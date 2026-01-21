/**
 * @file        wlf_gbm_allocator.c
 * @brief       GBM-based buffer allocator implementation.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 */

#include "wlf/allocator/wlf_gbm_allocator.h"
#include "wlf/buffer/wlf_gbm_buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>
#include <unistd.h>
#include <xf86drm.h>

// Forward declaration
struct wlf_gbm_buffer *wlf_gbm_buffer_create(struct wlf_gbm_allocator *alloc,
	int width, int height, uint32_t format, uint64_t modifier);

static const struct wlf_allocator_impl allocator_impl;

static struct wlf_gbm_allocator *get_gbm_alloc_from_alloc(
		struct wlf_allocator *wlf_alloc) {
	assert(wlf_alloc->impl == &allocator_impl);
	return wlf_container_of(wlf_alloc, struct wlf_gbm_allocator, base);
}

struct wlf_allocator *wlf_gbm_allocator_create(int fd) {
	uint64_t cap;
	if (drmGetCap(fd, DRM_CAP_PRIME, &cap) ||
			!(cap & DRM_PRIME_CAP_EXPORT)) {
		wlf_log(WLF_ERROR, "PRIME export not supported");
		return NULL;
	}

	struct wlf_gbm_allocator *alloc = calloc(1, sizeof(*alloc));
	if (alloc == NULL) {
		return NULL;
	}

	alloc->fd = fd;
	wlf_linked_list_init(&alloc->buffers);

	alloc->gbm_device = gbm_create_device(fd);
	if (alloc->gbm_device == NULL) {
		wlf_log(WLF_ERROR, "gbm_create_device failed");
		free(alloc);
		return NULL;
	}

	wlf_log(WLF_DEBUG, "Created GBM allocator with backend %s",
		gbm_device_get_backend_name(alloc->gbm_device));
	char *drm_name = drmGetDeviceNameFromFd2(fd);
	wlf_log(WLF_DEBUG, "Using DRM node %s", drm_name ? drm_name : "<unknown>");
	free(drm_name);

	// Initialize base allocator
	alloc->base.impl = &allocator_impl;
	wlf_signal_init(&alloc->base.events.destroy);

	return &alloc->base;
}

/**
 * @brief Destroys a GBM allocator.
 */
static void allocator_destroy(struct wlf_allocator *wlf_alloc) {
	struct wlf_gbm_allocator *alloc = get_gbm_alloc_from_alloc(wlf_alloc);

	// The gbm_bo objects need to be destroyed before the gbm_device
	struct wlf_gbm_buffer *buf, *buf_tmp;
	wlf_linked_list_for_each_safe(buf, buf_tmp, &alloc->buffers, link) {
		if (buf->gbm_bo != NULL) {
			gbm_bo_destroy(buf->gbm_bo);
			buf->gbm_bo = NULL;
		}
		wlf_linked_list_remove(&buf->link);
		wlf_linked_list_init(&buf->link);
	}

	gbm_device_destroy(alloc->gbm_device);
	close(alloc->fd);
	free(alloc);
}

/**
 * @brief Creates a buffer using the GBM allocator.
 *
 * Note: This simplified version uses DRM_FORMAT_XRGB8888 and DRM_FORMAT_MOD_LINEAR
 * as defaults. A more complete implementation should accept format/modifier parameters.
 */
static struct wlf_buffer *allocator_create_buffer(
		struct wlf_allocator *wlf_alloc, uint32_t width, uint32_t height) {
	struct wlf_gbm_allocator *alloc = get_gbm_alloc_from_alloc(wlf_alloc);

	// Use default format and modifier for now
	// TODO: Should be configurable based on renderer/display requirements
	uint32_t format = DRM_FORMAT_XRGB8888;
	uint64_t modifier = DRM_FORMAT_MOD_LINEAR;

	struct wlf_gbm_buffer *buffer = wlf_gbm_buffer_create(alloc, width, height,
		format, modifier);
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

struct wlf_gbm_allocator *wlf_gbm_allocator_from_allocator(
		struct wlf_allocator *allocator) {
	if (allocator == NULL || allocator->impl != &allocator_impl) {
		return NULL;
	}
	return get_gbm_alloc_from_alloc(allocator);
}

bool wlf_allocator_is_gbm(struct wlf_allocator *allocator) {
	if (allocator == NULL) {
		return false;
	}
	return allocator->impl == &allocator_impl;
}
