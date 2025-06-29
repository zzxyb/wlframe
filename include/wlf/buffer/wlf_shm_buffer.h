#ifndef BUFFER_WLF_SHM_BUFFER_H
#define BUFFER_WLF_SHM_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"

struct wlf_shm_attributes {
	int fd;
	uint32_t format;
	int width, height;
	int stride;
	off_t offset;
};

struct wlf_shm_buffer {
	struct wlf_buffer base;
};

bool wlf_shm_buffer_from_attribs(struct wlf_buffer *buffer,
	struct wlf_shm_attributes *attribs);

#endif // BUFFER_WLF_SHM_BUFFER_H
