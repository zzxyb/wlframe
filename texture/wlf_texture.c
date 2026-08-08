#include "wlf/texture/wlf_texture.h"
#include "wlf/image/wlf_image.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>

void wlf_texture_init(struct wlf_texture *texture, struct wlf_renderer *renderer,
		const struct wlf_texture_impl *impl, uint32_t width, uint32_t height) {
	assert(renderer);

	*texture = (struct wlf_texture){
		.renderer = renderer,
		.impl = impl,
		.width = width,
		.height = height,
	};
}

void wlf_texture_destroy(struct wlf_texture *texture) {
	if (texture && texture->impl && texture->impl->destroy) {
		texture->impl->destroy(texture);
	} else {
		free(texture);
	}
}

bool wlf_texture_read_pixels(struct wlf_texture *texture,
		const struct wlf_texture_read_pixels_options *options) {
	if (!texture->impl->read_pixels) {
		return false;
	}

	return texture->impl->read_pixels(texture, options);
}

uint32_t wlf_texture_preferred_read_format(struct wlf_texture *texture) {
	if (!texture->impl->preferred_read_format) {
		return WLF_FORMAT_INVALID;
	}

	return texture->impl->preferred_read_format(texture);
}

struct wlf_texture *wlf_texture_from_pixels(struct wlf_renderer *renderer,
		uint32_t fmt, uint32_t stride, uint32_t width, uint32_t height,
		const void *data) {
	assert(width > 0);
	assert(height > 0);
	assert(stride > 0);
	assert(data);

	struct wlf_readonly_data_buffer *buffer =
		wlf_readonly_data_buffer_create(fmt, stride, width, height, data);
	if (buffer == NULL) {
		return NULL;
	}

	struct wlf_texture *texture =
		wlf_texture_from_buffer(renderer, &buffer->base);

	wlf_readonly_data_buffer_drop(buffer);

	return texture;
}

struct wlf_texture *wlf_texture_from_image(struct wlf_renderer *renderer,
		const struct wlf_image *image) {
	if (renderer == NULL || image == NULL || image->data == NULL ||
			image->width == 0 || image->height == 0 || image->stride == 0) {
		return NULL;
	}

	/* wlf_image_load() requests 8-bit output from all decoders. Deriving the
	 * channel count from the decoded row also handles PNG transforms (palette,
	 * tRNS and gray-to-RGB) without exposing decoder-specific details here. */
	uint32_t channels = image->stride / image->width;
	if (channels < 1 || channels > 4 ||
			image->stride < image->width * channels) {
		wlf_log(WLF_ERROR, "unsupported image pixel layout");
		return NULL;
	}

	size_t rgba_stride = (size_t)image->width * 4;
	if (image->height > SIZE_MAX / rgba_stride) {
		wlf_log(WLF_ERROR, "image dimensions are too large");
		return NULL;
	}
	uint8_t *rgba = malloc(rgba_stride * image->height);
	if (rgba == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate image upload buffer");
		return NULL;
	}

	for (uint32_t y = 0; y < image->height; y++) {
		const uint8_t *src = image->data + (size_t)y * image->stride;
		uint8_t *dst = rgba + (size_t)y * rgba_stride;
		for (uint32_t x = 0; x < image->width; x++) {
			uint8_t r, g, b, a = 255;
			switch (channels) {
			case 1:
				r = g = b = src[x];
				break;
			case 2:
				r = g = b = src[x * 2];
				a = src[x * 2 + 1];
				break;
			case 3:
				r = src[x * 3];
				g = src[x * 3 + 1];
				b = src[x * 3 + 2];
				break;
			default:
				r = src[x * 4];
				g = src[x * 4 + 1];
				b = src[x * 4 + 2];
				a = src[x * 4 + 3];
				break;
			}
			dst[x * 4] = (uint8_t)((r * a + 127) / 255);
			dst[x * 4 + 1] = (uint8_t)((g * a + 127) / 255);
			dst[x * 4 + 2] = (uint8_t)((b * a + 127) / 255);
			dst[x * 4 + 3] = a;
		}
	}

	struct wlf_texture *texture = wlf_texture_from_pixels(renderer,
		WLF_FORMAT_ABGR8888, rgba_stride, image->width, image->height, rgba);
	free(rgba);
	return texture;
}

bool wlf_texture_update_from_buffer(struct wlf_texture *texture,
		struct wlf_buffer *buffer, const pixman_region32_t *damage) {
	if (!texture->impl->update_from_buffer) {
		return false;
	}

	if (texture->width != (uint32_t)buffer->width ||
		texture->height != (uint32_t)buffer->height) {
		return false;
	}

	const pixman_box32_t *extents = damage != NULL ?
		pixman_region32_extents((pixman_region32_t *)damage) : NULL;
	if (extents != NULL &&
			(extents->x1 < 0 || extents->y1 < 0 ||
			(uint32_t)extents->x2 > buffer->width ||
			(uint32_t)extents->y2 > buffer->height)) {
		return false;
	}

	return texture->impl->update_from_buffer(texture, buffer, damage);
}

struct wlf_texture *wlf_texture_from_buffer(struct wlf_renderer *renderer,
		struct wlf_buffer *buffer) {
	if (!renderer->impl->texture_from_buffer) {
		return NULL;
	}

	return renderer->impl->texture_from_buffer(renderer, buffer);
}

void *wlf_texture_read_pixel_options_get_data(
		const struct wlf_texture_read_pixels_options *options) {
	const struct wlf_pixel_format_info *fmt = wlf_get_pixel_format_info(options->format);

	return (char *)options->data +
		pixel_format_info_min_stride(fmt, options->dst_x) +
		options->dst_y * options->stride;
}

void wlf_texture_read_pixels_options_get_src_box(
		const struct wlf_texture_read_pixels_options *options,
		const struct wlf_texture *texture, struct wlf_rect *box) {
	if (wlf_rect_is_empty(&options->src_box)) {
		*box = (struct wlf_rect){
			.x = 0,
			.y = 0,
			.width = texture->width,
			.height = texture->height,
		};
		return;
	}

	*box = options->src_box;
}
