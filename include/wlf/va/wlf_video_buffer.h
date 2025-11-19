/**
 * @file        wlf_video_buffer.h
 * @brief       Video buffer abstraction for hardware decode backends.
 * @details     This file defines video buffer types that extend wlf_buffer
 *              for different backends (Vulkan, VA-API, Software).
 *
 * @author      YaoBing Xiao
 * @date        2026-01-23
 * @version     v2.0
 * @par Copyright:
 * @par History:
 *      version: v2.0, YaoBing Xiao, 2026-01-23, multi-backend buffer support\n
 */

#ifndef VA_WLF_VIDEO_BUFFER_H
#define VA_WLF_VIDEO_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/va/wlf_video_common.h"
#include <wayland-client.h>

struct wlf_video_buffer;

/**
 * @struct wlf_video_buffer_impl
 * @brief Extended buffer implementation for video buffers.
 */
struct wlf_video_buffer_impl {
	struct wlf_buffer_impl base;  /**< Base buffer implementation */

	/**
	 * @brief Export video buffer to wl_buffer for Wayland compositing.
	 * @param buffer Video buffer to export.
	 * @param wl_display Wayland display connection.
	 * @return wl_buffer handle, or NULL on failure.
	 */
	struct wl_buffer *(*export_to_wl_buffer)(
		struct wlf_video_buffer *buffer,
		struct wl_display *wl_display);
};

/**
 * @struct wlf_video_buffer
 * @brief Base video buffer structure.
 */
struct wlf_video_buffer {
	struct wlf_buffer base;               /**< Base buffer */
	const struct wlf_video_buffer_impl *impl;  /**< Video buffer impl */

	enum wlf_video_codec codec;           /**< Video codec */
	enum wlf_video_chroma_format chroma;  /**< Chroma format */
	uint32_t bit_depth;                   /**< Bit depth */

	struct wl_display *wl_display;        /**< Wayland display for export */
};

/**
 * @brief Initialize a video buffer.
 *
 * @param buffer Video buffer to initialize.
 * @param impl Video buffer implementation.
 * @param width Buffer width in pixels.
 * @param height Buffer height in pixels.
 */
void wlf_video_buffer_init(struct wlf_video_buffer *buffer,
	const struct wlf_video_buffer_impl *impl,
	uint32_t width, uint32_t height);

/**
 * @brief Set Wayland display for video buffer.
 *
 * @param buffer Video buffer.
 * @param wl_display Wayland display.
 */
void wlf_video_buffer_set_wayland_display(
	struct wlf_video_buffer *buffer,
	struct wl_display *wl_display);

/**
 * @brief Export video buffer to wl_buffer.
 *
 * @param buffer Video buffer.
 * @return wl_buffer handle, or NULL on failure.
 */
struct wl_buffer *wlf_video_buffer_export_to_wl_buffer(
	struct wlf_video_buffer *buffer);

/**
 * @brief Get video buffer from base buffer.
 *
 * @param buffer Base buffer.
 * @return Video buffer, or NULL if not a video buffer.
 */
struct wlf_video_buffer *wlf_video_buffer_from_buffer(
	struct wlf_buffer *buffer);

/**
 * @brief Check if buffer is a video buffer.
 *
 * @param buffer Base buffer.
 * @return true if it's a video buffer.
 */
bool wlf_buffer_is_video_buffer(struct wlf_buffer *buffer);

#endif /* VA_WLF_VIDEO_BUFFER_H */
