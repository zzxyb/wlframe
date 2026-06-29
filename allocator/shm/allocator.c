#include "wlf/allocator/shm/allocator.h"
#include "wlf/buffer/shm/buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/types/wlf_pixel_format.h"

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>

static void allocator_destroy(struct wlf_allocator *allocator) {
	struct wlf_shm_allocator *shm_allocator =
		wlf_shm_allocator_from_allocator(allocator);
	free(shm_allocator);
}

static struct wlf_buffer *allocator_create_buffer(
		struct wlf_allocator *allocator, uint32_t width, uint32_t height,
		const struct wlf_render_format *format) {
	const struct wlf_pixel_format_info *info =
		wlf_get_pixel_format_info(format->format);
	if (info == NULL) {
		wlf_log(WLF_ERROR, "Unsupported pixel format 0x%"PRIX32, format->format);
		return NULL;
	}

	struct wlf_shm_allocator *shm_allocator =
		wlf_shm_allocator_from_allocator(allocator);
	struct wlf_buffer *buffer =
		wlf_shm_buffer_create(shm_allocator, width, height, format->format);

	return buffer;
}

static const struct wlf_allocator_impl allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};

struct wlf_allocator *wlf_shm_allocator_create(struct wl_shm *wl_shm) {
	struct wlf_shm_allocator *allocator = calloc(1, sizeof(*allocator));
	if (allocator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_shm_allocator");
		return NULL;
	}

	allocator->wl_shm = wl_shm;
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
