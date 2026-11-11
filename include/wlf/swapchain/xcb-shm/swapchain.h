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

#ifndef XCB_SHM_SWAPCHAIN_H
#define XCB_SHM_SWAPCHAIN_H

#include "wlf/swapchain/wlf_swapchain.h"

#include <stdbool.h>
#include <xcb/xcb.h>

struct wlf_window;

/**
 * @brief Software swapchain uploading pixels to an XCB window.
 */
struct wlf_xcb_shm_swapchain {
	struct wlf_swapchain base; /**< Generic swapchain interface. */
	xcb_gcontext_t gc;        /**< Graphics context used for pixel uploads. */
};

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
struct wlf_swapchain *wlf_xcb_shm_swapchain_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);

/**
 * @brief Checks whether a swapchain is backed by XCB shared memory.
 * @param swapchain Generic swapchain to inspect.
 * @return true when @p swapchain is xcb-shm-backed, false otherwise.
 */
bool wlf_swapchain_is_xcb_shm(const struct wlf_swapchain *swapchain);

/**
 * @brief Casts a generic swapchain to an XCB shared-memory swapchain.
 * @param swapchain Swapchain known to be xcb-shm-backed.
 * @return Enclosing xcb-shm swapchain.
 * @note Asserts that the swapchain has the expected implementation.
 */
struct wlf_xcb_shm_swapchain *wlf_xcb_shm_swapchain_from_swapchain(
	struct wlf_swapchain *swapchain);

#endif // XCB_SHM_SWAPCHAIN_H
