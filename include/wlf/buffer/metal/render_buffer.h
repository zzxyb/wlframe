/**
 * @file        render_buffer.h
 * @brief       Metal buffer implementation for wlframe.
 * @details     This file defines the Metal-backed buffer structure and related
 *              utilities for DRM/Metal pixel format conversion. It provides the
 *              bridge between wlframe's generic buffer abstraction and the Metal
 *              rendering backend.
 * @author      YaoBing Xiao
 * @date        2026-03-17
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-17, initial version\n
 */

#ifndef METAL_RENDER_BUFFER_H
#define METAL_RENDER_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/renderer/metal/renderer.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdint.h>

/**
 * @brief Mapping between a DRM pixel format and its corresponding Metal pixel format.
 *
 * Used to look up the MTLPixelFormat value for a given DRM format identifier,
 * and vice versa.
 */
struct wlf_mtl_pixel_format {
	uint32_t drm_format;   /**< DRM fourcc format identifier */
	uint32_t mtl_format;   /**< Corresponding MTLPixelFormat value */
};

/**
 * @brief Metal-backed render buffer object.
 *
 * Wraps a generic @ref wlf_buffer with a Metal texture, allowing it to be used
 * by the Metal renderer. Each instance is tracked by the renderer's buffer list
 * and is destroyed automatically when the underlying buffer is destroyed.
 */
struct wlf_mtl_render_buffer {
	struct wlf_buffer *buffer;            /**< Underlying generic buffer */
	struct wlf_mtl_renderer *renderer;   /**< Renderer that owns this buffer */

	void *texture;                        /**< Metal texture handle (id<MTLTexture>) */

	struct wlf_listener buffer_destroy;  /**< Listener for buffer destroy event */

	struct wlf_linked_list link;         /**< List node in wlf_mtl_renderer.buffers */
};

/**
 * @brief Creates a Metal render buffer wrapping an existing wlframe buffer.
 *
 * Allocates and initializes a @ref wlf_mtl_render_buffer for use with the given
 * Metal renderer. Pixel data is uploaded to a new MTLTexture. The buffer is added
 * to the renderer's buffer list and will be destroyed automatically when the
 * underlying @p wlf_buffer is destroyed.
 *
 * @param renderer    The Metal renderer to associate with this buffer.
 * @param wlf_buffer  The generic wlframe buffer to wrap.
 * @return Pointer to the newly created Metal render buffer, or NULL on failure.
 */
struct wlf_mtl_render_buffer *wlf_mtl_render_buffer_create(
	struct wlf_mtl_renderer *renderer, struct wlf_buffer *wlf_buffer);

/**
 * @brief Destroys a Metal render buffer and releases all associated resources.
 *
 * Removes the buffer from the renderer's list, unregisters the destroy listener,
 * releases the Metal texture, and frees the buffer object.
 *
 * @param buffer The Metal render buffer to destroy.
 */
void wlf_mtl_render_buffer_destroy(struct wlf_mtl_render_buffer *buffer);

/**
 * @brief Looks up an existing Metal render buffer wrapping the given wlf_buffer.
 *
 * Searches the renderer's internal buffer list for a @ref wlf_mtl_render_buffer
 * whose underlying buffer matches @p wlf_buffer.
 *
 * @param renderer   The Metal renderer owning the buffer list.
 * @param wlf_buffer The generic wlframe buffer to look up.
 * @return Pointer to the matching Metal render buffer, or NULL if not found.
 */
struct wlf_mtl_render_buffer *wlf_mtl_render_buffer_get(
	struct wlf_mtl_renderer *renderer, struct wlf_buffer *wlf_buffer);

/**
 * @brief Looks up the MTLPixelFormat value for a given DRM fourcc format.
 *
 * @param fmt DRM fourcc pixel format identifier.
 * @return Corresponding MTLPixelFormat value, or 0 if not supported.
 */
uint32_t get_mtl_format_from_drm(uint32_t fmt);

/**
 * @brief Returns the list of DRM formats supported by the Metal backend.
 *
 * @param[out] len Number of formats in the returned array.
 * @return Pointer to an array of DRM fourcc format identifiers.
 */
const uint32_t *get_mtl_drm_formats(size_t *len);

#endif // METAL_RENDER_BUFFER_H
