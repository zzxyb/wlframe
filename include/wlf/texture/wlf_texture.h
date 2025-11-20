#ifndef TEXTURE_WLF_TEXTURE_H
#define TEXTURE_WLF_TEXTURE_H

#include "wlf/math/wlf_region.h"
#include "wlf/math//wlf_rect.h"

#include <stdint.h>

struct wlf_renderer;
struct wlf_texture;
struct wlf_buffer;

struct wlf_texture_read_pixels_options {
	/** Memory location to read pixels into */
	void *data;
	/** Format used for writing the pixel data */
	uint32_t format;
	/** Stride in bytes for the data */
	uint32_t stride;
	/** Destination offsets */
	uint32_t dst_x, dst_y;
	/** Source box of the texture to read from. If empty, the full texture is assumed. */
	const struct wlf_rect src_box;
};

struct wlf_texture_impl {
	bool (*update_from_buffer)(struct wlf_texture *texture,
		struct wlf_buffer *buffer, const struct wlf_region *damage);
	bool (*read_pixels)(struct wlf_texture *texture,
		const struct wlf_texture_read_pixels_options *options);
	uint32_t (*preferred_read_format)(struct wlf_texture *texture);
	void (*destroy)(struct wlf_texture *texture);
};

struct wlf_texture {
	const struct wlf_texture_impl *impl;
	struct wlf_renderer *render;

	uint32_t width, height;
};

bool wlf_texture_read_pixels(struct wlf_texture *texture,
	const struct wlf_texture_read_pixels_options *options);
uint32_t wlf_texture_preferred_read_format(struct wlf_texture *texture);
struct wlf_texture *wlf_texture_from_pixels(struct wlf_renderer *render,
	uint32_t fmt, uint32_t stride, uint32_t width, uint32_t height,
	const void *data);
bool wlf_texture_update_from_buffer(struct wlf_texture *texture,
	struct wlf_buffer *buffer, const struct wlf_region *damage);
void wlf_texture_destroy(struct wlf_texture *texture);
struct wlf_texture *wlf_texture_from_buffer(struct wlf_renderer *render,
	struct wlf_buffer *buffer);

#endif // TEXTURE_WLF_TEXTURE_H
