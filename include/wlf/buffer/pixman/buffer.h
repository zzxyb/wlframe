/**
 * @file        buffer.h
 * @brief       Pixman buffer implementation for wlframe.
 * @details     This file defines the pixman-backed buffer structure and related
 *              utilities for DRM/pixman pixel format conversion. It provides the
 *              bridge between wlframe's generic buffer abstraction and the pixman
 *              rendering backend.
 * @author      YaoBing Xiao
 * @date        2026-03-08
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-08, initial version\n
 */

#ifndef PIXMAN_BUFFER_H
#define PIXMAN_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/renderer/pixman/renderer.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_linked_list.h"

#include <pixman-1/pixman.h>

/**
 * @brief Mapping between a DRM pixel format and its corresponding pixman format.
 *
 * Used to look up the pixman format code for a given DRM format identifier,
 * and vice versa.
 */
struct wlf_pixman_pixel_format {
	uint32_t drm_format;                  /**< DRM fourcc format identifier */
	pixman_format_code_t pixman_format;   /**< Corresponding pixman format code */
};

/**
 * @brief Pixman-backed buffer object.
 *
 * Wraps a generic @ref wlf_buffer with a pixman image, allowing it to be used
 * by the pixman renderer. Each instance is tracked by the renderer's buffer list
 * and is destroyed automatically when the underlying buffer is destroyed.
 */
struct wlf_pixman_buffer {
	struct wlf_buffer *buffer;              /**< Underlying generic buffer */
	struct wlf_pixman_renderer *renderer;   /**< Renderer that owns this buffer */

	pixman_image_t *image;                  /**< Pixman image backed by the buffer data */

	struct wlf_listener buffer_destroy;     /**< Listener for buffer destroy event */

	struct wlf_linked_list link;            /**< List node in wlf_pixman_renderer.buffers */
};

/**
 * @brief Creates a pixman buffer wrapping an existing wlframe buffer.
 *
 * Allocates and initializes a @ref wlf_pixman_buffer for use with the given
 * pixman renderer. The new buffer is added to the renderer's buffer list and
 * will be destroyed automatically when the underlying @p wlf_buffer is destroyed.
 *
 * @param renderer    The pixman renderer to associate with this buffer.
 * @param wlf_buffer  The generic wlframe buffer to wrap.
 * @return Pointer to the newly created pixman buffer, or NULL on failure.
 */
struct wlf_pixman_buffer *wlf_pixman_buffer_create(
	struct wlf_pixman_renderer *renderer, struct wlf_buffer *wlf_buffer);

/**
 * @brief Destroys a pixman render buffer and releases all associated resources.
 *
 * Removes the buffer from the renderer's list, unregisters the destroy listener,
 * unreferences the pixman image, and frees the buffer object.
 *
 * @param buffer The pixman render buffer to destroy.
 */
void wlf_pixman_buffer_destroy(struct wlf_pixman_buffer *buffer);

/**
 * @brief Looks up an existing pixman render buffer wrapping the given wlf_buffer.
 *
 * Searches the renderer's internal buffer list for a @ref wlf_pixman_buffer
 * whose underlying buffer matches @p wlf_buffer.
 *
 * @param renderer   The pixman renderer owning the buffer list.
 * @param wlf_buffer The generic wlframe buffer to look up.
 * @return Pointer to the matching pixman render buffer, or NULL if not found.
 */
struct wlf_pixman_buffer *wlf_pixman_buffer_get(
	struct wlf_pixman_renderer *renderer, struct wlf_buffer *wlf_buffer);

/**
 * @brief Looks up the pixman format code for a given DRM fourcc format.
 *
 * @param fmt DRM fourcc pixel format identifier.
 * @return Corresponding pixman format code, or 0 if not supported.
 */
pixman_format_code_t get_pixman_format_from_drm(uint32_t fmt);

/**
 * @brief Looks up the DRM fourcc format for a given pixman format code.
 *
 * @param fmt Pixman format code.
 * @return Corresponding DRM fourcc format identifier, or 0 if not supported.
 */
uint32_t get_drm_format_from_pixman(pixman_format_code_t fmt);

/**
 * @brief Returns the list of DRM formats supported by the pixman backend.
 *
 * @param[out] len Number of formats in the returned array.
 * @return Pointer to an array of DRM fourcc format identifiers.
 */
const uint32_t *get_pixman_drm_formats(size_t *len);

#endif // PIXMAN_BUFFER_H
