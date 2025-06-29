#ifndef BUFFER_WLF_DMA_BUFFER_H
#define BUFFER_WLF_DMA_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"

struct wlf_dmabuf_attributes {
	int32_t width, height;
	uint32_t format;
	uint64_t modifier;

	int n_planes;
	uint32_t offset[WLF_DMABUF_MAX_PLANES];
	uint32_t stride[WLF_DMABUF_MAX_PLANES];
	int fd[WLF_DMABUF_MAX_PLANES];
};

struct wlf_dma_buffer {
	struct wlf_buffer base;
};

bool wlf_dma_buffer_from_attribs(struct wlf_buffer *buffer,
	struct wlf_dmabuf_attributes *attribs);

#endif // BUFFER_WLF_DMA_BUFFER_H
