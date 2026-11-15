#include "wlf/allocator/xcb-shm/allocator.h"

#include "wlf/buffer/xcb-shm/buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

static void allocator_destroy(struct wlf_allocator *base) {
	struct wlf_xcb_shm_allocator *allocator =
		wlf_xcb_shm_allocator_from_allocator(base);
	free(allocator);
}

static struct wlf_buffer *allocator_create_buffer(
		struct wlf_allocator *base, uint32_t width, uint32_t height,
		const struct wlf_render_format *format) {
	struct wlf_xcb_shm_allocator *allocator =
		wlf_xcb_shm_allocator_from_allocator(base);
	if (format == NULL || width == 0 || height == 0 ||
			width > INT16_MAX || height > INT16_MAX) {
		return NULL;
	}
	struct wlf_xcb_shm_buffer *buffer = wlf_xcb_shm_buffer_create(allocator->xcb,
		(int)width, (int)height, format->format);
	return buffer != NULL ? &buffer->base : NULL;
}

static const struct wlf_allocator_impl allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};

struct wlf_xcb_shm_allocator *wlf_xcb_shm_allocator_create(xcb_connection_t *xcb) {
	if (xcb == NULL || xcb_connection_has_error(xcb)) {
		return NULL;
	}
	struct wlf_xcb_shm_allocator *allocator = calloc(1, sizeof(*allocator));
	if (allocator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_xcb_shm_allocator");
		return NULL;
	}
	allocator->xcb = xcb;
	wlf_allocator_init(&allocator->base, &allocator_impl);
	return allocator;
}

bool wlf_allocator_is_xcb_shm(const struct wlf_allocator *allocator) {
	return allocator != NULL && allocator->impl == &allocator_impl;
}

struct wlf_xcb_shm_allocator *wlf_xcb_shm_allocator_from_allocator(
		struct wlf_allocator *allocator) {
	assert(allocator->impl == &allocator_impl);

	struct wlf_xcb_shm_allocator *shm_allocator =
		wlf_container_of(allocator, shm_allocator, base);
	return shm_allocator;
}
