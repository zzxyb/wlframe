/**
 * @file buffer.h
 * @brief Metal-backed wlframe buffer.
 */

#ifndef WLF_BUFFER_METAL_BUFFER_H
#define WLF_BUFFER_METAL_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/types/wlf_format_set.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_mtl_device;

/** A CPU-readable Metal texture suitable for rendering and presentation. */
struct wlf_mtl_buffer {
	struct wlf_buffer base;
	struct wlf_mtl_device *device; /**< Borrowed device wrapper. */
	void *texture; /**< Retained id<MTLTexture>. */
	uint32_t format;
	void *mapped_data;
	size_t mapped_stride;
	uint32_t mapped_flags;
	pixman_region32_t opaque;
	bool has_opaque_region;
};

struct wlf_mtl_buffer *wlf_mtl_buffer_create(struct wlf_mtl_device *device,
	uint32_t width, uint32_t height, const struct wlf_render_format *format);

bool wlf_buffer_is_mtl(const struct wlf_buffer *buffer);

struct wlf_mtl_buffer *wlf_mtl_buffer_from_buffer(
	struct wlf_buffer *buffer);

void *wlf_mtl_buffer_get_texture(const struct wlf_mtl_buffer *buffer);

uint32_t wlf_mtl_buffer_get_format(const struct wlf_mtl_buffer *buffer);

#endif /* WLF_BUFFER_METAL_BUFFER_H */
