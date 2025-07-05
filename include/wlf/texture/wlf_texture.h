#ifndef TEXTURE_WLF_TEXTURE_H
#define TEXTURE_WLF_TEXTURE_H

#include "wlf/math/wlf_region.h"
#include "wlf/math/wlf_rect.h"

#include <stdint.h>

struct wlf_render;
struct wlf_texture;
struct wlf_buffer;

struct wlf_texture_read_pixels_options {
	void *data;
	uint32_t format;
	uint32_t stride;
	uint32_t dst_x, dst_y;
	const struct wlf_rect src_box;
};

struct wlf_texture_impl {
	bool (*update_from_buffer)(struct wlf_texture *texture,
		struct wlf_buffer *buffer, const struct wlf_region *damage);
	bool (*read_pixels)(struct wlf_texture *texture,
		const struct wlf_texture_read_pixels_options *options);
	void (*destroy)(struct wlf_texture *texture);
};

struct wlf_texture {
	uint32_t width, height;
	struct wlf_render *render;
};

void wlf_texture_init(struct wlf_texture *texture, struct wlf_render *render,
	const struct wlf_texture_impl *impl, uint32_t width, uint32_t height);

/**
 * @brief Get texture size
 * @param texture Texture pointer
 * @return Size as width and height
 */
static inline uint32_t wlf_texture_get_width(const struct wlf_texture *texture) {
    return texture ? texture->width : 0;
}

static inline uint32_t wlf_texture_get_height(const struct wlf_texture *texture) {
    return texture ? texture->height : 0;
}

#endif // TEXTURE_WLF_TEXTURE_H
