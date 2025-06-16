/**
 * @file        wlf_xbm_image.h
 * @brief       XBM image handling and utility functions for wlframe.
 * @details     This file defines the wlf_xbm_image structure and related functions,
 *              providing a unified interface for creating, converting, and processing
 *              XBM images within wlframe. XBM (X BitMap) is a monochrome image
 *              format that stores images as C language header files, commonly
 *              used in X Window System applications.
 *
 *              Typical usage:
 *                  - Create an XBM image object.
 *                  - Convert between wlf_image and wlf_xbm_image.
 *                  - Load and save XBM image files.
 *
 * @author      YaoBing Xiao
 * @date        2025-06-17
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-17, initial version\n
 */

#ifndef IMAGE_WLF_XBM_IMAGE_H
#define IMAGE_WLF_XBM_IMAGE_H

#include "wlf/image/wlf_image.h"

/**
 * @brief XBM hotspot information for cursor images.
 */
struct wlf_xbm_hotspot {
	int32_t x; /**< X coordinate of hotspot */
	int32_t y; /**< Y coordinate of hotspot */
};

/**
 * @brief XBM image structure, extending wlf_image with XBM-specific information.
 */
struct wlf_xbm_image {
	struct wlf_image base;             /**< Base image structure */
	char *name;                        /**< XBM variable name (without suffix) */
	struct wlf_xbm_hotspot hotspot;    /**< Hotspot coordinates for cursor images */
	bool has_hotspot;                  /**< True if hotspot is defined */
};

/**
 * @brief Create a new wlf_xbm_image object.
 * @return Pointer to a newly allocated wlf_xbm_image structure, or NULL on failure.
 */
struct wlf_xbm_image *wlf_xbm_image_create(void);

/**
 * @brief Check if a wlf_image is an XBM image.
 * @param image Pointer to the wlf_image structure to check.
 * @return true if the image is an XBM image, false otherwise.
 * @note This function checks the image type and implementation to determine
 *       if the image is an XBM. It's useful for type checking before
 *       calling XBM-specific functions.
 */
bool wlf_image_is_xbm(struct wlf_image *image);

/**
 * @brief Convert a wlf_image pointer to a wlf_xbm_image pointer.
 * @param wlf_image Pointer to the base wlf_image structure.
 * @return Pointer to the corresponding wlf_xbm_image structure.
 */
struct wlf_xbm_image *wlf_xbm_image_from_image(struct wlf_image *wlf_image);

/**
 * @brief Set the XBM variable name.
 * @param image Pointer to the wlf_xbm_image structure.
 * @param name Variable name (will be copied).
 */
void wlf_xbm_image_set_name(struct wlf_xbm_image *image, const char *name);

/**
 * @brief Set the hotspot coordinates for cursor images.
 * @param image Pointer to the wlf_xbm_image structure.
 * @param x X coordinate of hotspot.
 * @param y Y coordinate of hotspot.
 */
void wlf_xbm_image_set_hotspot(struct wlf_xbm_image *image, int32_t x, int32_t y);

/**
 * @brief Clear the hotspot information.
 * @param image Pointer to the wlf_xbm_image structure.
 */
void wlf_xbm_image_clear_hotspot(struct wlf_xbm_image *image);

#endif // IMAGE_WLF_XBM_IMAGE_H
