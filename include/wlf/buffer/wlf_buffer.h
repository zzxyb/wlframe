#ifndef BUFFER_WLF_BUFFER_H
#define BUFFER_WLF_BUFFER_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_region.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_buffer;

struct wlf_buffer_impl {
	void (*destroy)(struct wlf_buffer *buffer);
	bool (*begin_data_ptr_access)(struct wlf_buffer *buffer, uint32_t flags,
		void **data, uint32_t *format, size_t *stride);
	void (*end_data_ptr_access)(struct wlf_buffer *buffer);
	const struct wlf_region *(*opaque_region)(struct wlf_buffer *buffer);
};

struct wlf_buffer {
	const struct wlf_buffer_impl *impl;
	struct {
		struct wlf_signal destroy;
	} events;

	size_t n_locks;
	int width, height;
	bool accessing_data_ptr;
};

void wlf_buffer_init(struct wlf_buffer *buffer,
	const struct wlf_buffer_impl *impl, int width, int height);
void wlf_buffer_finish(struct wlf_buffer *buffer);
void wlf_buffer_lock(struct wlf_buffer *buffer);
void wlf_buffer_unlock(struct wlf_buffer *buffer);
bool wlf_buffer_is_opaque(struct wlf_buffer *buffer);
bool wlf_buffer_begin_data_ptr_access(struct wlf_buffer *buffer, uint32_t flags,
	void **data, uint32_t *format, size_t *stride);
void wlf_buffer_end_data_ptr_access(struct wlf_buffer *buffer);

#endif // BUFFER_WLF_BUFFER_H
