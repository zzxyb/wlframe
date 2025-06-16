/**
 * @file        wlf_xpm_image.h
 * @brief       XPM image handling and utility functions for wlframe.
 * @details     This file defines the wlf_xpm_image structure and related functions,
 *              providing a unified interface for creating, converting, and processing
 *              XPM images within wlframe. XPM (X PixMap) is a color image format
 *              that stores images as C language header files, commonly used in
 *              X Window System applications for icons and small graphics.
 *
 *              Typical usage:
 *                  - Create an XPM image object.
 *                  - Convert between wlf_image and wlf_xpm_image.
 *                  - Load and save XPM image files.
 *
 * @author      YaoBing Xiao
 * @date        2025-06-17
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-17, initial version\n
 */

#ifndef IMAGE_WLF_XPM_IMAGE_H
#define IMAGE_WLF_XPM_IMAGE_H

#include "wlf/image/wlf_image.h"

/**
 * @brief XPM color entry structure.
 */
struct wlf_xpm_color {
	char key;           /**< Character key for this color */
	uint8_t r, g, b;    /**< RGB color values */
	char *name;         /**< Color name (optional) */
};

/**
 * @brief XPM hotspot information for cursor images.
 */
struct wlf_xpm_hotspot {
	int32_t x; /**< X coordinate of hotspot */
	int32_t y; /**< Y coordinate of hotspot */
};

/**
 * @brief XPM image structure, extending wlf_image with XPM-specific information.
 */
struct wlf_xpm_image {
	struct wlf_image base;              /**< Base image structure */
	char *name;                         /**< XPM variable name (without suffix) */
	struct wlf_xpm_hotspot hotspot;     /**< Hotspot coordinates for cursor images */
	bool has_hotspot;                   /**< True if hotspot is defined */
	uint32_t colors_per_pixel;          /**< Characters per pixel (usually 1) */
	uint32_t num_colors;                /**< Number of colors in palette */
	struct wlf_xpm_color *colors;       /**< Color palette */
};

/**
 * @brief Create a new wlf_xpm_image object.
 * @return Pointer to a newly allocated wlf_xpm_image structure, or NULL on failure.
 */
struct wlf_xpm_image *wlf_xpm_image_create(void);

/**
 * @brief Check if a wlf_image is an XPM image.
 * @param image Pointer to the wlf_image structure to check.
 * @return true if the image is an XPM image, false otherwise.
 * @note This function checks the image type and implementation to determine
 *       if the image is an XPM. It's useful for type checking before
 *       calling XPM-specific functions.
 */
bool wlf_image_is_xpm(struct wlf_image *image);

/**
 * @brief Convert a wlf_image pointer to a wlf_xpm_image pointer.
 * @param wlf_image Pointer to the base wlf_image structure.
 * @return Pointer to the corresponding wlf_xpm_image structure.
 */
struct wlf_xpm_image *wlf_xpm_image_from_image(struct wlf_image *wlf_image);

/**
 * @brief Set the XPM variable name.
 * @param image Pointer to the wlf_xpm_image structure.
 * @param name Variable name (will be copied).
 */
void wlf_xpm_image_set_name(struct wlf_xpm_image *image, const char *name);

/**
 * @brief Set the hotspot coordinates for cursor images.
 * @param image Pointer to the wlf_xpm_image structure.
 * @param x X coordinate of hotspot.
 * @param y Y coordinate of hotspot.
 */
void wlf_xpm_image_set_hotspot(struct wlf_xpm_image *image, int32_t x, int32_t y);

/**
 * @brief Clear the hotspot information.
 * @param image Pointer to the wlf_xpm_image structure.
 */
void wlf_xpm_image_clear_hotspot(struct wlf_xpm_image *image);

/**
 * @brief Set the characters per pixel value.
 * @param image Pointer to the wlf_xpm_image structure.
 * @param cpp Characters per pixel (typically 1 or 2).
 */
void wlf_xpm_image_set_colors_per_pixel(struct wlf_xpm_image *image, uint32_t cpp);

#endif // IMAGE_WLF_XPM_IMAGE_H
