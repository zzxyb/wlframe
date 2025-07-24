/**
 * @file        wlf_bmp_image.h
 * @brief       BMP image handling and utility functions for wlframe.
 * @details     This file defines the wlf_bmp_image structure and related functions,
 *              providing a unified interface for creating, converting, and processing
 *              BMP images within wlframe. BMP (Bitmap) is a raster graphics image
 *              format developed by Microsoft for storing bitmap digital images.
 *
 *              Typical usage:
 *                  - Create a BMP image object.
 *                  - Convert between wlf_image and wlf_bmp_image.
 *                  - Load and save BMP image files.
 *
 * @author      YaoBing Xiao
 * @date        2025-06-17
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-17, initial version\n
 */

#ifndef IMAGE_WLF_BMP_IMAGE_H
#define IMAGE_WLF_BMP_IMAGE_H

#include "wlf/image/wlf_image.h"

/**
 * @brief Supported BMP compression types.
 */
enum wlf_bmp_compression {
	WLF_BMP_COMPRESSION_RGB = 0,       /**< No compression (RGB) */
	WLF_BMP_COMPRESSION_RLE8 = 1,      /**< 8-bit RLE compression */
	WLF_BMP_COMPRESSION_RLE4 = 2,      /**< 4-bit RLE compression */
	WLF_BMP_COMPRESSION_BITFIELDS = 3, /**< Bitfields compression */
};

/**
 * @brief BMP image structure, extending wlf_image with BMP-specific information.
 */
struct wlf_bmp_image {
	struct wlf_image base;                /**< Base image structure */
	enum wlf_bmp_compression compression; /**< BMP compression type */
	uint32_t bits_per_pixel;              /**< Bits per pixel (1, 4, 8, 16, 24, 32) */
	uint32_t colors_used;                 /**< Number of colors in palette (0 = maximum) */
	uint32_t important_colors;            /**< Number of important colors (0 = all) */
	bool top_down;                        /**< True if image is stored top-down */
};

/**
 * @brief Create a new wlf_bmp_image object.
 * @return Pointer to a newly allocated wlf_bmp_image structure, or NULL on failure.
 */
struct wlf_bmp_image *wlf_bmp_image_create(void);

/**
 * @brief Check if a wlf_image is a BMP image.
 * @param image Pointer to the wlf_image structure to check.
 * @return true if the image is a BMP image, false otherwise.
 * @note This function checks the image type and implementation to determine
 *       if the image is a BMP. It's useful for type checking before
 *       calling BMP-specific functions.
 */
bool wlf_image_is_bmp(struct wlf_image *image);

/**
 * @brief Convert a wlf_image pointer to a wlf_bmp_image pointer.
 * @param wlf_image Pointer to the base wlf_image structure.
 * @return Pointer to the corresponding wlf_bmp_image structure.
 */
struct wlf_bmp_image *wlf_bmp_image_from_image(struct wlf_image *wlf_image);

/**
 * @brief Set the BMP compression type for the image.
 * @param image Pointer to the wlf_bmp_image structure.
 * @param compression BMP compression type to set.
 */
void wlf_bmp_image_set_compression(struct wlf_bmp_image *image, enum wlf_bmp_compression compression);

/**
 * @brief Set the bits per pixel for the BMP image.
 * @param image Pointer to the wlf_bmp_image structure.
 * @param bits_per_pixel Bits per pixel (typically 1, 4, 8, 16, 24, or 32).
 */
void wlf_bmp_image_set_bits_per_pixel(struct wlf_bmp_image *image, uint32_t bits_per_pixel);

#endif // IMAGE_WLF_BMP_IMAGE_H
