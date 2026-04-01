/**
 * @file        wlf_xpm_image.h
 * @brief       XPM image handling and utility functions for wlframe.
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
