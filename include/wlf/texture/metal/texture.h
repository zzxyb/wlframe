/**
 * @file        texture.h
 * @brief       Metal-backed texture implementation for wlframe.
 * @details     This file defines the internal structure and helper functions for
 *              textures rendered using the Metal GPU renderer. It extends the
 *              generic wlf_texture interface with Metal-specific texture data.
 * @author      YaoBing Xiao
 * @date        2026-03-17
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-17, initial version\n
 */

#ifndef METAL_TEXTURE_H
#define METAL_TEXTURE_H

#include "wlf/texture/wlf_texture.h"
#include "wlf/renderer/metal/renderer.h"
#include "wlf/buffer/wlf_buffer.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/types/wlf_pixel_format.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_mtl_renderer;

/**
 * @brief A Metal-backed texture object.
 *
 * Extends @ref wlf_texture with Metal-specific state. The underlying
 * pixel data is stored in a MTLTexture on the GPU.
 */
struct wlf_mtl_texture {
	struct wlf_texture wlf_texture;       /**< Base texture object (must be first). */
	struct wlf_mtl_renderer *renderer;    /**< Metal renderer that owns this texture. */
	struct wlf_linked_list link;          /**< Link node in wlf_mtl_renderer.textures. */

	void *texture;                        /**< Metal texture handle (id<MTLTexture>). */
	uint32_t drm_format;                  /**< DRM fourcc pixel format of the texture. */
	const struct wlf_pixel_format_info *format_info;  /**< wlf pixel format metadata. */

	struct wlf_buffer *buffer;            /**< Source buffer; non-NULL if created via texture_from_buffer. */
};

/**
 * @brief Creates a new Metal texture with the given format and dimensions.
 *
 * Allocates and initializes a @ref wlf_mtl_texture backed by a new
 * MTLTexture on the GPU. The caller is responsible for uploading pixel data.
 *
 * @param renderer   Metal renderer to associate the texture with.
 * @param drm_format DRM fourcc pixel format for the texture.
 * @param width      Texture width in pixels.
 * @param height     Texture height in pixels.
 * @return Pointer to the new texture, or NULL on failure.
 */
struct wlf_mtl_texture *wlf_mtl_texture_create(
	struct wlf_mtl_renderer *renderer, uint32_t drm_format, uint32_t width,
	uint32_t height);

/**
 * @brief Checks whether a texture is a Metal-backed texture.
 *
 * @param texture Texture to check.
 * @return true if the texture was created by the Metal renderer.
 */
bool wlf_texture_is_mtl(const struct wlf_texture *texture);

/**
 * @brief Downcasts a generic texture to a Metal texture.
 *
 * The caller must ensure the texture is actually a @ref wlf_mtl_texture
 * (e.g. by checking wlf_texture_is_mtl() first).
 *
 * @param texture Generic texture to downcast.
 * @return Pointer to the underlying wlf_mtl_texture, or NULL if not Metal.
 */
struct wlf_mtl_texture *wlf_mtl_texture_from_texture(
	struct wlf_texture *texture);

#endif // METAL_TEXTURE_H
