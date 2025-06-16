/**
 * @file        wlf_png_image.h
 * @brief       PNG image handling and utility functions for wlframe.
 * @details     This file defines the wlf_png_image structure and related functions,
 *              providing a unified interface for creating, converting, and printing
 *              PNG images within wlframe. It also includes helpers for interlace type
 *              and pixel data output.
 *
 *              Typical usage:
 *                  - Create a PNG image object.
 *                  - Convert between wlf_image and wlf_png_image.
 *                  - Print PNG image pixel data for debugging or export.
 *
 * @author      YaoBing Xiao
 * @date        2025-06-08
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-08, initial version\n
 */

#ifndef IMAGE_WLF_PNG_IMAGE_H
#define IMAGE_WLF_PNG_IMAGE_H

#include "wlf/image/wlf_image.h"

/**
 * @brief Supported PNG interlace types.
 */
enum wlf_image_interlace_type {
	WLF_IMAGE_INTERLACE_NONE = 0, /**< No interlacing */
	WLF_IMAGE_INTERLACE_ADAM7,    /**< Adam7 interlacing: progressive display using 7 passes */
};

/**
 * @brief PNG image structure, extending wlf_image with interlace information.
 */
struct wlf_png_image {
	struct wlf_image base;                        /**< Base image structure */
	enum wlf_image_interlace_type interlace_type; /**< PNG interlace type */
};

/**
 * @brief Create a new wlf_png_image object.
 * @return Pointer to a newly allocated wlf_png_image structure, or NULL on failure.
 */
struct wlf_png_image *wlf_png_image_create(void);

/**
 * @brief Check if a wlf_image is a PNG image.
 * @param image Pointer to the wlf_image structure to check.
 * @return true if the image is a PNG image, false otherwise.
 * @note This function checks the image type and implementation to determine
 *       if the image is a PNG. It's useful for type checking before
 *       calling PNG-specific functions.
 */
bool wlf_image_is_png(struct wlf_image *image);

/**
 * @brief Convert a wlf_image pointer to a wlf_png_image pointer.
 * @param wlf_image Pointer to the base wlf_image structure.
 * @return Pointer to the corresponding wlf_png_image structure.
 */
struct wlf_png_image *wlf_png_image_from_image(struct wlf_image *wlf_image);

/**
 * @brief Convert a wlf_image color type to PNG color type.
 * @param image Pointer to the wlf_image structure.
 * @return Corresponding PNG color type value.
 */
int wlf_color_type_to_png(struct wlf_image *image);

#endif // IMAGE_WLF_PNG_IMAGE_H
