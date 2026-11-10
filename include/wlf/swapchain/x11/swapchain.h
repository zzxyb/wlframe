/**
 * @file        swapchain.h
 * @brief       X11 software swapchain for the Pixman renderer.
 * @details     This file declares the XRGB8888 software swapchain, which
 *              uploads damaged pixels to an XCB window.
 * @author      YaoBing Xiao
 * @date        2026-11-10
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-11-10, initial version\n
 */

#ifndef SWAPCHAIN_X11_SWAPCHAIN_H
#define SWAPCHAIN_X11_SWAPCHAIN_H

#include "wlf/swapchain/wlf_swapchain.h"
struct wlf_window;
/**
 * @brief Creates a software swapchain for an X11 window.
 *
 * @param window X11 window borrowing the backend connection.
 * @param width Initial buffer width in pixels, from 1 to INT16_MAX.
 * @param height Initial buffer height in pixels, from 1 to INT16_MAX.
 * @param format Requested render format; must be WLF_FORMAT_XRGB8888.
 * @return New swapchain, or NULL for unsupported formats or allocation failure.
 * @note Requires a 24-bit TrueColor root visual and 32-bit pixels with
 * matching byte order. Destroy with wlf_swapchain_destroy() before the window.
 */
struct wlf_swapchain *wlf_x11_swapchain_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);

#endif // SWAPCHAIN_X11_SWAPCHAIN_H
