/**
 * @file        wlf_xpm_image.h
 * @brief       XPM image handling and utility functions for wlframe.
 * @details     This file defines XPM-specific image structures derived from
 *              @ref wlf_image and provides helper functions for creating,
 *              detecting, and converting XPM image objects.
 *
 *              Typical usage:
 *                  - Create an XPM image container via wlf_xpm_image_create().
 *                  - Check whether a generic image instance is an XPM image.
 *                  - Convert a generic @ref wlf_image pointer into a
 *                    @ref wlf_xpm_image pointer when appropriate.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-10
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-10, initial version\n
 */

#ifndef IMAGE_WLF_XPM_IMAGE_H
#define IMAGE_WLF_XPM_IMAGE_H

#include "wlf/image/wlf_image.h"

/**
 * @brief XPM image structure extending @ref wlf_image.
 */
struct wlf_xpm_image {
	struct wlf_image base; /**< Base image structure */
};

/**
 * @brief Create a new XPM image object.
 * @return Pointer to allocated XPM image, or NULL on failure.
 */
struct wlf_xpm_image *wlf_xpm_image_create(void);

/**
 * @brief Check if a generic image is an XPM image.
 * @param image Generic image pointer.
 * @return true if image is XPM, false otherwise.
 */
bool wlf_image_is_xpm(const struct wlf_image *image);

/**
 * @brief Cast generic image pointer to XPM image pointer.
 * @param wlf_image Generic image pointer.
 * @return XPM image pointer.
 */
struct wlf_xpm_image *wlf_xpm_image_from_image(struct wlf_image *wlf_image);

#endif // IMAGE_WLF_XPM_IMAGE_H
