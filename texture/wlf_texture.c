#include "wlf/texture/wlf_texture.h"
#include "wlf/types/wlf_pixel_format.h"

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

bool wlf_texture_update_from_buffer(struct wlf_texture *texture,
		struct wlf_buffer *buffer, const struct wlf_region *damage) {
	if (!texture->impl->update_from_buffer) {
		return false;
	}

	if (texture->width != (uint32_t)buffer->width ||
		texture->height != (uint32_t)buffer->height) {
		return false;
	}

	struct wlf_frect extents = wlf_region_bounding_rect(damage);
	if (extents.x < 0 || extents.y < 0 || extents.x + extents.width > buffer->width ||
		extents.y + extents.height > buffer->height) {
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
