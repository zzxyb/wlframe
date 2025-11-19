/**
 * @file        wlf_va_video_buffer.h
 * @brief       VA-API video buffer implementation.
 * @details     This file defines VA-API-based video buffer type.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-26
 * @version     v2.1
 * @par Copyright:
 * @par History:
 *      version: v2.1, YaoBing Xiao, 2026-01-26, split into separate file\n
 */

#ifndef VA_WLF_VA_VIDEO_BUFFER_H
#define VA_WLF_VA_VIDEO_BUFFER_H

#include "wlf/va/wlf_video_buffer.h"
#include <va/va.h>

/**
 * @struct wlf_va_video_buffer
 * @brief VA-API video buffer.
 */
struct wlf_va_video_buffer {
	struct wlf_video_buffer base;         /**< Base video buffer */

	/* VA-API resources */
	VADisplay va_display;
	VASurfaceID surface_id;
	VAImageFormat va_format;

	/* Cached wl_buffer */
	struct wl_buffer *wl_buffer;
};

/**
 * @brief Create a VA-API video buffer.
 *
 * @param va_display VA display.
 * @param surface_id VA surface ID.
 * @param width Buffer width.
 * @param height Buffer height.
 * @return Pointer to created buffer, or NULL on failure.
 */
struct wlf_va_video_buffer *wlf_va_video_buffer_create(
	VADisplay va_display,
	VASurfaceID surface_id,
	uint32_t width, uint32_t height);

/**
 * @brief Get VA-API video buffer from base video buffer.
 *
 * @param buffer Base video buffer.
 * @return VA-API video buffer, or NULL if not a VA-API buffer.
 */
static inline struct wlf_va_video_buffer *wlf_va_video_buffer_from_video_buffer(
	struct wlf_video_buffer *buffer);

/* VA-API video buffer implementation */
extern const struct wlf_video_buffer_impl va_video_buffer_impl;

/* Inline implementation */
static inline struct wlf_va_video_buffer *wlf_va_video_buffer_from_video_buffer(
	struct wlf_video_buffer *buffer) {
	if (buffer && buffer->impl == &va_video_buffer_impl) {
		return (struct wlf_va_video_buffer *)buffer;
	}
	return NULL;
}

#endif /* VA_WLF_VA_VIDEO_BUFFER_H */
