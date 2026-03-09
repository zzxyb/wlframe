/**
 * @file        texture.h
 * @brief       Pixman-backed texture implementation for wlframe.
 * @details     This file defines the internal structure and helper functions for
 *              textures rendered using the pixman software renderer. It extends
 *              the generic wlf_texture interface with pixman-specific image data.
 * @author      YaoBing Xiao
 * @date        2026-03-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-09, initial version\n
 */

#ifndef PIXMAN_TEXTURE_H
#define PIXMAN_TEXTURE_H

#include "wlf/texture/wlf_texture.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/buffer/wlf_buffer.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/types/wlf_pixel_format.h"

#include <pixman-1/pixman.h>

struct wlf_pixman_renderer;

/**
 * @brief A pixman-backed texture object.
 *
 * Extends @ref wlf_texture with pixman-specific image state. A texture is
 * either backed by a raw pixel data pointer (created via texture_from_pixels)
 * or by a @ref wlf_buffer (created via texture_from_buffer), but never both.
 */
struct wlf_pixman_texture {
	struct wlf_texture wlf_texture;          /**< Base texture object (must be first). */
	struct wlf_pixman_renderer *renderer;    /**< Pixman renderer that owns this texture. */
	struct wlf_linked_list link;             /**< Link node in wlf_pixman_renderer.textures. */

	pixman_image_t *image;                   /**< Pixman image holding the pixel data. */
	pixman_format_code_t format;             /**< Pixman pixel format of the image. */
	const struct wlf_pixel_format_info *format_info;  /**< wlf pixel format metadata. */

	void *data;                  /**< Raw pixel data pointer; non-NULL if created via texture_from_pixels. */
	struct wlf_buffer *buffer;   /**< Source buffer; non-NULL if created via texture_from_buffer. */
};

/**
 * @brief Creates a new pixman texture with the given format and dimensions.
 *
 * Allocates and initializes a @ref wlf_pixman_texture backed by an empty
 * pixman image. The caller is responsible for populating the image data.
 *
 * @param renderer   Pixman renderer to associate the texture with.
 * @param drm_format DRM fourcc pixel format for the texture.
 * @param width      Texture width in pixels.
 * @param height     Texture height in pixels.
 * @return Pointer to the new texture, or NULL on failure.
 */
struct wlf_pixman_texture *wlf_pixman_texture_create(
	struct wlf_pixman_renderer *renderer, uint32_t drm_format, uint32_t width,
	uint32_t height);

/**
 * @brief Checks whether a texture is a pixman-backed texture.
 *
 * @param texture Texture to check.
 * @return true if the texture was created by the pixman renderer.
 */
bool wlf_texture_is_pixman(const struct wlf_texture *texture);

/**
 * @brief Downcasts a generic texture to a pixman texture.
 *
 * The caller must ensure the texture is actually a @ref wlf_pixman_texture
 * (e.g. by checking wlf_texture_is_pixman() first).
 *
 * @param texture Generic texture to downcast.
 * @return Pointer to the underlying wlf_pixman_texture.
 */
struct wlf_pixman_texture *wlf_pixman_texture_from_texture(
	struct wlf_texture *texture);

#endif // PIXMAN_TEXTURE_H
