#ifndef ALLOCATOR_WLF_ALLOCATOR_H
#define ALLOCATOR_WLF_ALLOCATOR_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/renderer/wlf_renderer.h"

#include <stdint.h>

struct wlf_buffer;
struct wlf_allocator;

struct wlf_allocator_impl {
	struct wlf_buffer *(*create_buffer)(struct wlf_allocator *allocator,
		uint32_t width, uint32_t height);
	void (*destroy)(struct wlf_allocator *alloc);
};

struct wlf_allocator {
	const struct wlf_allocator_impl *impl;

	struct {
		struct wlf_signal destroy;
	} events;
};

void wlf_allocator_init(struct wlf_allocator *allocator,
	const struct wlf_allocator_impl *impl);

struct wlf_allocator *wlf_allocator_autocreate(struct wlf_backend *backend,
	struct wlf_renderer *renderer);

void wlf_allocator_destroy(struct wlf_allocator *allocator);

struct wlf_buffer *wlf_allocator_create_buffer(struct wlf_allocator *allocator,
	uint32_t width, uint32_t height);

#endif // ALLOCATOR_WLF_ALLOCATOR_H
