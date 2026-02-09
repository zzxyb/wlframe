/**
 * @file        wlf_ra_framebuffer.h
 * @brief       Remote Assistance framebuffer management.
 * @details     Provides framebuffer structure and functions for capturing
 *              and managing screen frame data in remote assistance.
 * @author      YaoBing Xiao
 * @date        2026-02-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-09, initial version\n
 */

#ifndef RA_WLF_RA_FRAMEBUFFER_H
#define RA_WLF_RA_FRAMEBUFFER_H

#include "wlf_ra_types.h"
#include "wlf/utils/wlf_signal.h"
#include <stddef.h>

/**
 * @brief Framebuffer structure - represents a screen capture buffer.
 */
struct wlf_ra_framebuffer {
	char *data;                          /**< Frame buffer data */
	int width;                           /**< Width in pixels */
	int height;                          /**< Height in pixels */
	int stride;                          /**< Bytes per row (padded width) */
	int depth;                           /**< Bits per pixel */
	struct wlf_ra_pixel_format format;   /**< Pixel format */

	void *backend_data;                  /**< Backend-specific data */

	struct {
		struct wlf_signal update;        /**< Signal emitted when framebuffer is updated */
		struct wlf_signal destroy;       /**< Signal emitted when framebuffer is destroyed */
	} events;
};

/**
 * @brief Creates a new framebuffer.
 * @param width Width in pixels.
 * @param height Height in pixels.
 * @param depth Bits per pixel.
 * @return New framebuffer instance or NULL on error.
 */
struct wlf_ra_framebuffer *wlf_ra_framebuffer_create(int width, int height, int depth);

/**
 * @brief Destroys a framebuffer.
 * @param fb Framebuffer to destroy.
 */
void wlf_ra_framebuffer_destroy(struct wlf_ra_framebuffer *fb);

/**
 * @brief Gets framebuffer data pointer.
 * @param fb Framebuffer instance.
 * @return Pointer to framebuffer data.
 */
char *wlf_ra_framebuffer_get_data(struct wlf_ra_framebuffer *fb);

/**
 * @brief Gets framebuffer dimensions.
 * @param fb Framebuffer instance.
 * @param width Output width.
 * @param height Output height.
 */
void wlf_ra_framebuffer_get_size(struct wlf_ra_framebuffer *fb, int *width, int *height);

/**
 * @brief Updates framebuffer content.
 * @param fb Framebuffer instance.
 * @param data Source data.
 * @param width Source width.
 * @param height Source height.
 * @param stride Source stride.
 */
void wlf_ra_framebuffer_update(struct wlf_ra_framebuffer *fb, const char *data,
		int width, int height, int stride);

#endif /* RA_WLF_RA_FRAMEBUFFER_H */
