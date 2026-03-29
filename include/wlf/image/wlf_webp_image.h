/**
 * @file        wlf_webp_image.h
 * @brief       WebP image extensions and animation metadata for wlframe.
 * @details     This file defines WebP-specific image structures based on
 *              @ref wlf_image, including animation metadata and per-frame
 *              pixel/timestamp information.
 *
 *              Typical usage:
 *                  - Create a WebP image container via wlf_webp_image_create().
 *                  - Detect whether a generic image or file is WebP.
 *                  - Access animation frames and loop/background information.
 *
 * @author      YaoBing Xiao
 * @date        2026-03-30
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-03-30, initial version\n
 */

#ifndef IMAGE_WLF_WEBP_IMAGE_H
#define IMAGE_WLF_WEBP_IMAGE_H

#include "wlf/image/wlf_image.h"

struct wlf_webp_animation_info;

/**
 * @brief WebP image object derived from @ref wlf_image.
 */
struct wlf_webp_image {
	struct wlf_image base; /**< Base image object. */
	struct wlf_webp_animation_info *ani_info; /**< Optional animation metadata. */
};

struct wlf_webp_frame;

/**
 * @brief Animation metadata for an animated WebP image.
 */
struct wlf_webp_animation_info {
	int frame_count; /**< Number of frames in the animation. */
	int canvas_w; /**< Canvas width in pixels. */
	int canvas_h; /**< Canvas height in pixels. */
	int loop_count; /**< Playback loop count (0 usually means infinite). */
	uint32_t bgcolor; /**< Background color used for compositing. */
	uint8_t *_pixbuf; /**< Internal decoded pixel buffer. */
	struct wlf_webp_frame *frames; /**< Array of animation frames. */
};

/**
 * @brief One decoded frame in an animated WebP image.
 */
struct wlf_webp_frame {
	uint8_t *pixels; /**< Frame pixel buffer. */
	int timestamp; /**< Frame start time in milliseconds. */
	int width; /**< Frame width in pixels. */
	int height; /**< Frame height in pixels. */
};

/**
 * @brief Create a new WebP image object.
 * @return Pointer to the newly allocated WebP image, or NULL on failure.
 */
struct wlf_webp_image *wlf_webp_image_create(void);

/**
 * @brief Check whether a generic image is a WebP image.
 * @param image Pointer to a generic image object.
 * @return true if the image type is WebP, otherwise false.
 */
bool wlf_image_is_webp(const struct wlf_image *image);

/**
 * @brief Cast a generic image pointer to a WebP image pointer.
 * @param wlf_image Pointer to a generic image object.
 * @return Pointer to @ref wlf_webp_image if compatible, otherwise NULL.
 */
struct wlf_webp_image *wlf_webp_image_from_image(struct wlf_image *wlf_image);

/**
 * @brief Check whether a file is a WebP image by its content/signature.
 * @param file_name Path to the file.
 * @return true if the file is a valid WebP image, otherwise false.
 */
bool wlf_file_is_webp(const char *file_name);

#endif // IMAGE_WLF_WEBP_IMAGE_H
