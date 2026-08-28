#include "wlf/allocator/egl/allocator.h"

#include "wlf/buffer/egl/buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>

static void allocator_destroy(struct wlf_allocator *base) {
	struct wlf_egl_allocator *allocator =
		wlf_egl_allocator_from_allocator(base);
	free(allocator);
}

static struct wlf_buffer *allocator_create_buffer(
		struct wlf_allocator *base, uint32_t width, uint32_t height,
		const struct wlf_render_format *format) {
	struct wlf_egl_allocator *allocator =
		wlf_egl_allocator_from_allocator(base);
	return wlf_egl_buffer_create(allocator->egl, allocator->surface,
		width, height, format);
}

static const struct wlf_allocator_impl allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};

struct wlf_egl_allocator *wlf_egl_allocator_create(struct wlf_egl *egl,
		struct wl_surface *surface) {
	if (egl == NULL || surface == NULL) {
		return NULL;
	}

	struct wlf_egl_allocator *allocator = calloc(1, sizeof(*allocator));
	if (allocator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_egl_allocator");
		return NULL;
	}

	allocator->egl = egl;
	allocator->surface = surface;
	wlf_allocator_init(&allocator->base, &allocator_impl);

	return allocator;
}

bool wlf_allocator_is_egl(const struct wlf_allocator *allocator) {
	return allocator != NULL && allocator->impl == &allocator_impl;
}

struct wlf_egl_allocator *wlf_egl_allocator_from_allocator(
		struct wlf_allocator *allocator) {
	assert(allocator->impl == &allocator_impl);

	struct wlf_egl_allocator *egl_allocator =
		wlf_container_of(allocator, egl_allocator, base);
	return egl_allocator;
}
