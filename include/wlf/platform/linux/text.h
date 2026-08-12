/**
 * @file        text.h
 * @brief       Linux text implementation for wlframe.
 * @details     Declares the Linux Cairo/Pango/HarfBuzz text implementation used by
 *              the platform-independent text interface.
 * @author      YaoBing Xiao
 * @date        2026-08-12
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-12, initial version\n
 */

#ifndef LINUX_TEXT_H
#define LINUX_TEXT_H

#include "wlf/platform/wlf_text.h"

/**
 * @brief Linux text object.
 */
struct wlf_linux_text {
	struct wlf_text base; /**< Platform-independent text object. */
};

/**
 * @brief Create the Linux Cairo/Pango/HarfBuzz text implementation.
 * @return A newly allocated text object, or NULL on allocation failure.
 */
struct wlf_linux_text *wlf_linux_text_create(void);

#endif // LINUX_TEXT_H
