#ifndef ALLOCATOR_WLF_ALLOCATOR_H
#define ALLOCATOR_WLF_ALLOCATOR_H

#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_signal.h"

struct wlf_allocator;
struct wlf_backend;
struct wlf_buffer;
struct wlf_render;

enum wlf_allocator_type {
	WLF_ALLOCATOR_VK = 0,
};

struct wlf_allocator_impl{
	enum wlf_allocator_type type;
	void (*destroy)(struct wlf_allocator *allocator);
};

struct wlf_allocator {
	const struct wlf_allocator_impl *impl;
	struct {
		struct wlf_signal destroy;
	} events;
};

struct wlf_allocator *wlf_allocator_autocreate(struct wlf_backend *backend, struct wlf_render *render);
void wlf_allocator_destroy(struct wlf_allocator *allocator);

#endif // ALLOCATOR_WLF_ALLOCATOR_H
