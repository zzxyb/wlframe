/**
 * @file        wlf_texture.h
 * @brief       Texture abstraction for wlframe.
 * @details     This file defines the texture interface used by wlframe renderers.
 *              A texture represents GPU-resident image data associated with a renderer,
 *              and can be created from raw pixel data or a wlf_buffer. It supports
 *              pixel readback and incremental updates from damaged buffer regions.
 * @author      YaoBing Xiao
 * @date        2026-03-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-09, initial version\n
 */

#ifndef TEXTURE_WLF_TEXTURE_H
#define TEXTURE_WLF_TEXTURE_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/math/wlf_region.h"
#include "wlf/math/wlf_rect.h"

#include <stdint.h>

struct wlf_texture;

/**
 * @brief Options for reading pixels back from a texture.
 *
 * Specifies the destination memory, pixel format, layout, and the sub-region
 * of the texture to read from.
 */
struct wlf_texture_read_pixels_options {
	void *data;                  /**< Memory location to read pixels into. */
	uint32_t format;             /**< DRM fourcc format used for writing the pixel data. */
	uint32_t stride;             /**< Row stride in bytes for @p data. */
	uint32_t dst_x, dst_y;       /**< Destination offsets within @p data to start writing. */
	/** Source box of the texture to read from. If empty, the full texture is assumed. */
	const struct wlf_rect src_box;
};

/**
 * @brief Virtual method table for texture implementations.
 *
 * Backend-specific renderers (e.g. pixman, vulkan) provide this interface
 * to implement texture operations.
 */
struct wlf_texture_impl {
	/**
	 * @brief Updates the texture contents from a damaged region of a buffer.
	 * @param texture Texture to update.
	 * @param buffer  Source buffer containing new pixel data.
	 * @param damage  Region of the buffer that has changed.
	 * @return true on success, false on failure.
	 */
	bool (*update_from_buffer)(struct wlf_texture *texture,
		struct wlf_buffer *buffer, const struct wlf_region *damage);

	/**
	 * @brief Reads pixels from the texture into CPU memory.
	 * @param texture Texture to read from.
	 * @param options Pixel readback parameters.
	 * @return true on success, false on failure.
	 */
	bool (*read_pixels)(struct wlf_texture *texture,
		const struct wlf_texture_read_pixels_options *options);

	/**
	 * @brief Returns the preferred DRM fourcc format for reading pixels.
	 * @param texture Texture to query.
	 * @return Preferred DRM fourcc pixel format identifier.
	 */
	uint32_t (*preferred_read_format)(struct wlf_texture *texture);

	/**
	 * @brief Destroys the texture and releases all backend resources.
	 * @param texture Texture to destroy.
	 */
	void (*destroy)(struct wlf_texture *texture);
};

/**
 * @brief A texture object holding GPU-resident image data.
 *
 * Textures are created by a renderer and are bound to its lifetime.
 * They can be populated from raw pixel data or imported from a @ref wlf_buffer.
 */
struct wlf_texture {
	const struct wlf_texture_impl *impl;  /**< Virtual method table. */
	struct wlf_renderer *renderer;        /**< Renderer that owns this texture. */

	uint32_t width, height;               /**< Texture dimensions in pixels. */
};

/**
 * @brief Initializes a texture object.
 *
 * Sets up the base @ref wlf_texture fields. Must be called by backend
 * implementations as part of their texture creation path.
 *
 * @param texture  Texture to initialize.
 * @param renderer Renderer that will own this texture.
 * @param impl     Virtual method table provided by the backend.
 * @param width    Texture width in pixels.
 * @param height   Texture height in pixels.
 */
void wlf_texture_init(struct wlf_texture *texture, struct wlf_renderer *renderer,
	const struct wlf_texture_impl *impl, uint32_t width, uint32_t height);

/**
 * @brief Destroys a texture and releases all associated resources.
 *
 * @param texture Texture to destroy.
 */
void wlf_texture_destroy(struct wlf_texture *texture);

/**
 * @brief Reads pixels from a texture into CPU-accessible memory.
 *
 * @param texture Texture to read from.
 * @param options Pixel readback parameters including destination buffer and format.
 * @return true on success, false on failure.
 */
bool wlf_texture_read_pixels(struct wlf_texture *texture,
	const struct wlf_texture_read_pixels_options *options);

/**
 * @brief Returns the preferred DRM fourcc pixel format for reading this texture.
 *
 * The preferred format minimizes any conversion overhead during readback.
 *
 * @param texture Texture to query.
 * @return Preferred DRM fourcc format identifier.
 */
uint32_t wlf_texture_preferred_read_format(struct wlf_texture *texture);

/**
 * @brief Creates a texture from raw pixel data in CPU memory.
 *
 * Uploads the provided pixel data to the renderer, creating a new texture object.
 *
 * @param renderer The renderer to create the texture on.
 * @param fmt      DRM fourcc pixel format of @p data.
 * @param stride   Row stride in bytes of @p data.
 * @param width    Width of the texture in pixels.
 * @param height   Height of the texture in pixels.
 * @param data     Pointer to the raw pixel data.
 * @return Pointer to the newly created texture, or NULL on failure.
 */
struct wlf_texture *wlf_texture_from_pixels(struct wlf_renderer *renderer,
	uint32_t fmt, uint32_t stride, uint32_t width, uint32_t height,
	const void *data);

/**
 * @brief Updates the texture contents from a damaged region of a buffer.
 *
 * Only the pixels within @p damage are re-uploaded, allowing efficient
 * incremental updates.
 *
 * @param texture Texture to update.
 * @param buffer  Source buffer containing updated pixel data.
 * @param damage  Region describing which parts of the buffer have changed.
 * @return true on success, false on failure.
 */
bool wlf_texture_update_from_buffer(struct wlf_texture *texture,
	struct wlf_buffer *buffer, const struct wlf_region *damage);

/**
 * @brief Creates a texture by importing the contents of a wlf_buffer.
 *
 * The buffer's pixel data is uploaded to the renderer. The resulting texture
 * is independent of the buffer after creation.
 *
 * @param renderer The renderer to create the texture on.
 * @param buffer   Source buffer to import pixel data from.
 * @return Pointer to the newly created texture, or NULL on failure.
 */
struct wlf_texture *wlf_texture_from_buffer(struct wlf_renderer *renderer,
	struct wlf_buffer *buffer);

/**
 * @brief Returns the destination data pointer from pixel readback options.
 *
 * Provides a typed accessor for the @p data field of
 * @ref wlf_texture_read_pixels_options.
 *
 * @param options Pixel readback options.
 * @return Pointer to the destination memory buffer.
 */
void *wlf_texture_read_pixel_options_get_data(
	const struct wlf_texture_read_pixels_options *options);

/**
 * @brief Resolves the source box from pixel readback options.
 *
 * If @p options->src_box is non-empty, copies it into @p box.
 * Otherwise fills @p box with the full texture extent
 * (origin at (0, 0), size equal to @p texture dimensions).
 *
 * @param options Pixel readback options containing the (possibly empty) src_box.
 * @param texture Texture being read, used to derive the full extent fallback.
 * @param box     Output rectangle receiving the resolved source box.
 */
void wlf_texture_read_pixels_options_get_src_box(
	const struct wlf_texture_read_pixels_options *options,
	const struct wlf_texture *texture, struct wlf_rect *box);

#endif // TEXTURE_WLF_TEXTURE_H
