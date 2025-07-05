#ifndef SWRAST_SW_TEXTURE_H
#define SWRAST_SW_TEXTURE_H

#include "wlf/texture/wlf_texture.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"

#include <pixman.h>

#include <stdint.h>

struct wlf_render;
struct wlf_buffer;

struct wlf_pixel_format_info {
	uint32_t format;

	/* Equivalent of the format if it has an alpha channel,
	 * DRM_FORMAT_INVALID (0) if NA
	 */
	uint32_t opaque_substitute;

	/* Bytes per block (including padding) */
	uint32_t bytes_per_block;
	/* Size of a block in pixels (zero for 1×1) */
	uint32_t block_width, block_height;
};

struct wlf_sw_texture {
	struct wlf_texture base;
	struct wlf_render *render;

	struct wlf_linked_list link;

	pixman_image_t *image;
	pixman_format_code_t format;
	const struct wlf_pixel_format_info *format_info;

	void *data; // if created via texture_from_pixels
	struct wlf_buffer *buffer; // if created via texture_from_buffer
};

#endif // SWRAST_SW_TEXTURE_H
