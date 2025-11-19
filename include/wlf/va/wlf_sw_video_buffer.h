/**
 * @file        wlf_sw_video_buffer.h
 * @brief       Software video buffer implementation.
 * @details     This file defines software-based video buffer type.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-26
 * @version     v2.1
 * @par Copyright:
 * @par History:
 *      version: v2.1, YaoBing Xiao, 2026-01-26, split into separate file\n
 */

#ifndef VA_WLF_SW_VIDEO_BUFFER_H
#define VA_WLF_SW_VIDEO_BUFFER_H

#include "wlf/va/wlf_video_buffer.h"
#include <wayland-client.h>

/**
 * @struct wlf_sw_video_buffer
 * @brief Software video buffer.
 */
struct wlf_sw_video_buffer {
	struct wlf_video_buffer base;         /**< Base video buffer */

	/* Software resources */
	void *data;                           /**< Pixel data */
	size_t size;                          /**< Buffer size */
	size_t stride;                        /**< Row stride */
	uint32_t pixel_format;                /**< Pixel format (e.g., ARGB8888) */

	/* wl_shm resources */
	int shm_fd;
	struct wl_shm_pool *shm_pool;
	struct wl_buffer *wl_buffer;
};

/**
 * @brief Create a software video buffer.
 *
 * @param width Buffer width.
 * @param height Buffer height.
 * @param pixel_format Pixel format.
 * @return Pointer to created buffer, or NULL on failure.
 */
struct wlf_sw_video_buffer *wlf_sw_video_buffer_create(
	uint32_t width, uint32_t height,
	uint32_t pixel_format);

/**
 * @brief Get software video buffer from base video buffer.
 *
 * @param buffer Base video buffer.
 * @return Software video buffer, or NULL if not a software buffer.
 */
static inline struct wlf_sw_video_buffer *wlf_sw_video_buffer_from_video_buffer(
	struct wlf_video_buffer *buffer);

/* Software video buffer implementation */
extern const struct wlf_video_buffer_impl sw_video_buffer_impl;

/* Inline implementation */
static inline struct wlf_sw_video_buffer *wlf_sw_video_buffer_from_video_buffer(
	struct wlf_video_buffer *buffer) {
	if (buffer && buffer->impl == &sw_video_buffer_impl) {
		return (struct wlf_sw_video_buffer *)buffer;
	}
	return NULL;
}

#endif /* VA_WLF_SW_VIDEO_BUFFER_H */
