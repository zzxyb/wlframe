#include "wlf/allocator/metal/allocator.h"

#include "wlf/buffer/metal/buffer.h"
#include "wlf/renderer/metal/device.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>

static void allocator_destroy(struct wlf_allocator *base) {
	struct wlf_mtl_allocator *allocator =
		wlf_mtl_allocator_from_allocator(base);
	free(allocator);
}

static struct wlf_buffer *allocator_create_buffer(
		struct wlf_allocator *base, uint32_t width, uint32_t height,
		const struct wlf_render_format *format) {
	struct wlf_mtl_allocator *allocator =
		wlf_mtl_allocator_from_allocator(base);
	struct wlf_mtl_buffer *buffer = wlf_mtl_buffer_create(
		allocator->device, width, height, format);
	return buffer != NULL ? &buffer->base : NULL;
}

static const struct wlf_allocator_impl allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};

struct wlf_mtl_allocator *wlf_mtl_allocator_create(
		struct wlf_mtl_device *device) {
	if (device == NULL || device->device == NULL) {
		return NULL;
	}

	struct wlf_mtl_allocator *allocator = calloc(1, sizeof(*allocator));
	if (allocator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_mtl_allocator");
		return NULL;
	}

	allocator->device = device;
	wlf_allocator_init(&allocator->base, &allocator_impl);
	return allocator;
}

bool wlf_allocator_is_mtl(const struct wlf_allocator *allocator) {
	return allocator != NULL && allocator->impl == &allocator_impl;
}

struct wlf_mtl_allocator *wlf_mtl_allocator_from_allocator(
		struct wlf_allocator *allocator) {
	assert(wlf_allocator_is_mtl(allocator));
	struct wlf_mtl_allocator *metal = NULL;
	return wlf_container_of(allocator, metal, base);
}
