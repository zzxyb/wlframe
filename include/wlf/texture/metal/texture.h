/**
 * @file texture.h
 * @brief Metal texture implementation.
 */

#ifndef WLF_TEXTURE_METAL_TEXTURE_H
#define WLF_TEXTURE_METAL_TEXTURE_H

#include "wlf/texture/wlf_texture.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_mtl_renderer;

struct wlf_mtl_texture {
	struct wlf_texture base;
	struct wlf_mtl_renderer *renderer;
	struct wlf_linked_list link;
	void *texture; /**< Retained id<MTLTexture>. */
	uint32_t format;
};

struct wlf_mtl_texture *wlf_mtl_texture_from_buffer(
	struct wlf_mtl_renderer *renderer, struct wlf_buffer *buffer);

bool wlf_texture_is_mtl(const struct wlf_texture *texture);

struct wlf_mtl_texture *wlf_mtl_texture_from_texture(
	struct wlf_texture *texture);

void *wlf_mtl_texture_get_handle(const struct wlf_mtl_texture *texture);

#endif /* WLF_TEXTURE_METAL_TEXTURE_H */
