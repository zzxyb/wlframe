/**
 * @file        swapchain.h
 * @brief       XCB EGL swapchain.
 * @details     Declares the XCB EGL-backed swapchain used by the GLES renderer
 *              to present frames for an XCB window.
 * @author      YaoBing Xiao
 * @date        2026-11-15
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-11-15, initial version\n
 */

#ifndef XCB_EGL_SWAPCHAIN_H
#define XCB_EGL_SWAPCHAIN_H

#include "wlf/swapchain/wlf_swapchain.h"

#include <stdbool.h>

/**
 * @brief Swapchain backed by an XCB EGL window surface.
 *
 * The EGL-backed buffer is created by the swapchain's EGL allocator. The
 * swapchain retains the buffer handle and owns presentation policy, while the
 * buffer owns the EGL surface resources.
 */
struct wlf_xcb_egl_swapchain {
	struct wlf_swapchain base; /**< Generic swapchain interface. */
	struct wlf_buffer *back; /**< EGL-backed buffer available for rendering. */
};

/**
 * @brief Creates an EGL swapchain for an XCB window.
 * @param window XCB window using a GLES renderer.
 * @param width Initial buffer width in pixels.
 * @param height Initial buffer height in pixels.
 * @param format Requested EGL render format.
 * @return Newly allocated XCB EGL swapchain, or NULL on failure.
 */
struct wlf_xcb_egl_swapchain *wlf_xcb_egl_swapchain_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);

/**
 * @brief Checks whether a swapchain uses the XCB EGL implementation.
 * @param swapchain Generic swapchain to inspect.
 * @return true when @p swapchain is an XCB EGL swapchain, false otherwise.
 */
bool wlf_swapchain_is_xcb_egl(const struct wlf_swapchain *swapchain);

/**
 * @brief Casts a generic swapchain to an XCB EGL swapchain.
 * @param swapchain Swapchain known to use the XCB EGL implementation.
 * @return Enclosing XCB EGL swapchain.
 * @note Asserts that the swapchain uses the XCB EGL implementation.
 */
struct wlf_xcb_egl_swapchain *wlf_xcb_egl_swapchain_from_swapchain(struct wlf_swapchain *swapchain);

#endif // XCB_EGL_SWAPCHAIN_H
