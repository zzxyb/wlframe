#ifndef BUFFER_WLF_BUFFER_H
#define BUFFER_WLF_BUFFER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define WLF_DMABUF_MAX_PLANES 4

struct wlf_buffer;

enum wlF_buffer_cap {
	WLF_BUFFER_CAP_DATA_PTR = 0,
	WLF_BUFFER_CAP_DMABUF,
	WLF_BUFFER_CAP_SHM,
};

struct wlf_buffer_impl {
	enum wlF_buffer_cap capabilities;
	void (*destroy)(struct wlf_buffer *buffer);
	bool (*begin_data_ptr_access)(struct wlf_buffer *buffer, uint32_t flags,
		void **data, uint32_t *format, size_t *stride);
	void (*end_data_ptr_access)(struct wlf_buffer *buffer);
};

struct wlf_buffer {
	const struct wlf_buffer_impl *impl;
	int width;                /**< Width of the buffer in pixels */
	int height;
};

void wlf_buffer_init(struct wlf_buffer *buffer,
	const struct wlf_buffer_impl *impl, int width, int height);
void wlf_buffer_finish(struct wlf_buffer *buffer);

#endif // BUFFER_WLF_BUFFER_H
