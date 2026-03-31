/**
 * @file        wlf_gif_image.h
 * @brief       GIF image handling and utility functions for wlframe.
 * @details     This file defines GIF-specific image/frame structures and
 *              functions, providing a unified interface for creating,
 *              converting, loading, and saving GIF images (including
 *              animated GIFs) within wlframe.
 */

#ifndef IMAGE_WLF_GIF_IMAGE_H
#define IMAGE_WLF_GIF_IMAGE_H

#include "wlf/image/wlf_image.h"

/**
 * @brief One decoded GIF frame as a full-canvas RGBA image.
 */
struct wlf_gif_frame {
	unsigned char *pixels;  /**< RGBA pixels, size = width * height * 4 */
	uint32_t width;         /**< Frame canvas width in pixels */
	uint32_t height;        /**< Frame canvas height in pixels */
	uint32_t stride;        /**< Bytes per row (usually width * 4) */
	uint32_t delay_ms;      /**< Frame delay in milliseconds */
	uint8_t disposal_method;/**< GIF disposal method (0..3) */
};

/**
 * @brief GIF image structure, extending wlf_image with animation metadata.
 */
struct wlf_gif_image {
	struct wlf_image base;          /**< Base image structure */
	uint32_t loop_count;            /**< Animation loop count (0 means infinite) */
	uint32_t delay_ms;              /**< First frame delay time in milliseconds */
	uint32_t frame_count;           /**< Number of frames */
	struct wlf_gif_frame *frames;   /**< Optional frame array for animation */
};

/**
 * @brief Create a new wlf_gif_image object.
 * @return Pointer to a newly allocated wlf_gif_image structure, or NULL on failure.
 */
struct wlf_gif_image *wlf_gif_image_create(void);

/**
 * @brief Check if a wlf_image is a GIF image.
 * @param image Pointer to the wlf_image structure to check.
 * @return true if the image is a GIF image, false otherwise.
 */
bool wlf_image_is_gif(const struct wlf_image *image);

/**
 * @brief Convert a wlf_image pointer to a wlf_gif_image pointer.
 * @param wlf_image Pointer to the base wlf_image structure.
 * @return Pointer to the corresponding wlf_gif_image structure.
 */
struct wlf_gif_image *wlf_gif_image_from_image(struct wlf_image *wlf_image);

#endif // IMAGE_WLF_GIF_IMAGE_H
