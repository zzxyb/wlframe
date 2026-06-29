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

static bool wlf_image_get_texture_format(const struct wlf_image *image,
		uint32_t *format, uint32_t *channels) {
	switch (image->format) {
	case WLF_COLOR_TYPE_RGB:
		/* RGB byte data is BGR888 in DRM's little-endian notation. */
		*format = WLF_FORMAT_BGR888;
		*channels = 3;
		return true;
	case WLF_COLOR_TYPE_RGBA:
		/* RGBA byte order is represented by ABGR8888 in DRM notation. */
		*format = WLF_FORMAT_ABGR8888;
		*channels = 4;
		return true;
	case WLF_COLOR_TYPE_GRAY:
		*format = WLF_FORMAT_R8;
		*channels = 1;
		return true;
	case WLF_COLOR_TYPE_GRAY_ALPHA:
		/* DRM GR88 is a two-channel color format, not gray + alpha. */
		*format = WLF_FORMAT_INVALID;
		*channels = 2;
		return true;
	default:
		return false;
	}
}

struct wlf_texture *wlf_texture_from_image(struct wlf_renderer *renderer,
		const struct wlf_image *image) {
	if (renderer == NULL || image == NULL || image->data == NULL ||
			image->width == 0 || image->height == 0 || image->stride == 0) {
		return NULL;
	}

	uint32_t format, channels;
	if (!wlf_image_get_texture_format(image, &format, &channels)) {
		wlf_log(WLF_ERROR, "unsupported image format");
		return NULL;
	}
	if (image->bit_depth != 0 && image->bit_depth != WLF_IMAGE_BIT_DEPTH_8) {
		wlf_log(WLF_ERROR, "unsupported image bit depth");
		return NULL;
	}

	size_t min_stride = (size_t)image->width * channels;
	if (image->stride < min_stride) {
		wlf_log(WLF_ERROR, "invalid image row stride");
		return NULL;
	}

	/* Keep the image's native format whenever the renderer supports it. */
	bool has_alpha = image->format == WLF_COLOR_TYPE_RGBA ||
		image->format == WLF_COLOR_TYPE_GRAY_ALPHA;
	if (format != WLF_FORMAT_INVALID && (!has_alpha || image->is_opaque)) {
		struct wlf_texture *texture = wlf_texture_from_pixels(renderer,
			format, image->stride, image->width, image->height, image->data);
		if (texture != NULL) {
			return texture;
		}
	}

	/* The fallback below produces premultiplied ABGR8888. This is required for
	 * the current Pixman and GLES texture blend paths when the source contains
	 * straight alpha, or when the renderer lacks the native image format. */
	size_t width = (size_t)image->width;
	if (width > SIZE_MAX / 4) {
		wlf_log(WLF_ERROR, "image dimensions are too large");
		return NULL;
	}
	size_t rgba_stride = width * 4;
	if (image->height > SIZE_MAX / rgba_stride) {
		wlf_log(WLF_ERROR, "image dimensions are too large");
		return NULL;
	}
	uint8_t *rgba = malloc(rgba_stride * image->height);
	if (rgba == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate image upload buffer");
		return NULL;
	}

	switch (channels) {
	case 1:
		for (uint32_t y = 0; y < image->height; y++) {
			const uint8_t *src = image->data + (size_t)y * image->stride;
			uint8_t *dst = rgba + (size_t)y * rgba_stride;
			for (uint32_t x = 0; x < image->width; x++) {
				size_t dst_offset = (size_t)x * 4;
				uint8_t value = src[x];
				dst[dst_offset] = value;
				dst[dst_offset + 1] = value;
				dst[dst_offset + 2] = value;
				dst[dst_offset + 3] = 255;
			}
		}
		break;
	case 2:
		for (uint32_t y = 0; y < image->height; y++) {
			const uint8_t *src = image->data + (size_t)y * image->stride;
			uint8_t *dst = rgba + (size_t)y * rgba_stride;
			for (uint32_t x = 0; x < image->width; x++) {
				size_t src_offset = (size_t)x * 2;
				size_t dst_offset = (size_t)x * 4;
				uint8_t a = src[src_offset + 1];
				uint8_t value = (uint8_t)((src[src_offset] * a + 127) / 255);
				dst[dst_offset] = value;
				dst[dst_offset + 1] = value;
				dst[dst_offset + 2] = value;
				dst[dst_offset + 3] = a;
			}
		}
		break;
	case 3:
		for (uint32_t y = 0; y < image->height; y++) {
			const uint8_t *src = image->data + (size_t)y * image->stride;
			uint8_t *dst = rgba + (size_t)y * rgba_stride;
			for (uint32_t x = 0; x < image->width; x++) {
				size_t src_offset = (size_t)x * 3;
				size_t dst_offset = (size_t)x * 4;
				dst[dst_offset] = src[src_offset];
				dst[dst_offset + 1] = src[src_offset + 1];
				dst[dst_offset + 2] = src[src_offset + 2];
				dst[dst_offset + 3] = 255;
			}
		}
		break;
	case 4:
		for (uint32_t y = 0; y < image->height; y++) {
			const uint8_t *src = image->data + (size_t)y * image->stride;
			uint8_t *dst = rgba + (size_t)y * rgba_stride;
			for (uint32_t x = 0; x < image->width; x++) {
				size_t src_offset = (size_t)x * 4;
				size_t dst_offset = (size_t)x * 4;
				uint8_t a = src[src_offset + 3];
				dst[dst_offset] = (uint8_t)((src[src_offset] * a + 127) / 255);
				dst[dst_offset + 1] =
					(uint8_t)((src[src_offset + 1] * a + 127) / 255);
				dst[dst_offset + 2] =
					(uint8_t)((src[src_offset + 2] * a + 127) / 255);
				dst[dst_offset + 3] = a;
			}
		}
		break;
	default:
		free(rgba);
		return NULL;
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
