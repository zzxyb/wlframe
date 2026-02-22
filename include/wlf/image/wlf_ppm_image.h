/**
 * @file        wlf_ppm_image.h
 * @brief       PPM image handling and utility functions for wlframe.
 * @details     This file defines the wlf_ppm_image structure and related functions,
 *              providing a unified interface for creating, converting, and processing
 *              PPM images within wlframe. PPM (Portable Pixmap Format) is a simple
 *              uncompressed bitmap format that supports RGB color data.
 *
 *              Typical usage:
 *                  - Create a PPM image object.
 *                  - Convert between wlf_image and wlf_ppm_image.
 *                  - Load and save PPM image files.
 *
 * @author      YaoBing Xiao
 * @date        2025-06-17
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-17, initial version\n
 */

#ifndef IMAGE_WLF_PPM_IMAGE_H
#define IMAGE_WLF_PPM_IMAGE_H

#include "wlf/image/wlf_image.h"

/**
 * @brief Supported PPM format variants.
 */
enum wlf_ppm_format {
	WLF_PPM_FORMAT_P3 = 3, /**< ASCII PPM format (P3) */
	WLF_PPM_FORMAT_P6 = 6, /**< Binary PPM format (P6) */
};

/**
 * @brief PPM image structure, extending wlf_image with PPM-specific information.
 */
struct wlf_ppm_image {
	struct wlf_image base;        /**< Base image structure */
	enum wlf_ppm_format format;   /**< PPM format variant (P3 or P6) */
	uint32_t max_val;             /**< Maximum color value (typically 255) */
};

/**
 * @brief Create a new wlf_ppm_image object.
 * @return Pointer to a newly allocated wlf_ppm_image structure, or NULL on failure.
 */
struct wlf_ppm_image *wlf_ppm_image_create(void);

/**
 * @brief Check if a wlf_image is a PPM image.
 * @param image Pointer to the wlf_image structure to check.
 * @return true if the image is a PPM image, false otherwise.
 * @note This function checks the image type and implementation to determine
 *       if the image is a PPM. It's useful for type checking before
 *       calling PPM-specific functions.
 */
bool wlf_image_is_ppm(const struct wlf_image *image);

/**
 * @brief Convert a wlf_image pointer to a wlf_ppm_image pointer.
 * @param wlf_image Pointer to the base wlf_image structure.
 * @return Pointer to the corresponding wlf_ppm_image structure.
 */
struct wlf_ppm_image *wlf_ppm_image_from_image(struct wlf_image *wlf_image);

/**
 * @brief Set the PPM format for the image.
 * @param image Pointer to the wlf_ppm_image structure.
 * @param format PPM format variant to set.
 */
void wlf_ppm_image_set_format(struct wlf_ppm_image *image, enum wlf_ppm_format format);

/**
 * @brief Set the maximum color value for the PPM image.
 * @param image Pointer to the wlf_ppm_image structure.
 * @param max_val Maximum color value (typically 255 for 8-bit images).
 */
void wlf_ppm_image_set_max_val(struct wlf_ppm_image *image, uint32_t max_val);

#endif // IMAGE_WLF_PPM_IMAGE_H
