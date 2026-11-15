/**
 * @file        swapchain.h
 * @brief       Wayland EGL swapchain.
 * @details     Declares the Wayland EGL-backed swapchain used by the GLES renderer
 *              to present frames for a Wayland window.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WL_EGL_SWAPCHAIN_H
#define WL_EGL_SWAPCHAIN_H

#include "wlf/swapchain/wlf_swapchain.h"

#include <stdbool.h>

/**
 * @brief Swapchain backed by a Wayland EGL window surface.
 *
 * The EGL-backed buffer is created by the swapchain's EGL allocator. The
 * swapchain retains the buffer handle and owns presentation policy, while the
 * buffer owns the EGL surface resources.
 */
struct wlf_wl_egl_swapchain {
	struct wlf_swapchain base; /**< Generic swapchain interface. */
	struct wlf_buffer *back; /**< EGL-backed buffer available for rendering. */
};

/**
 * @brief Creates an EGL swapchain for a Wayland window.
 * @param window Wayland window using a GLES renderer.
 * @param width Initial buffer width in pixels.
 * @param height Initial buffer height in pixels.
 * @param format Requested EGL render format.
 * @return Newly allocated Wayland EGL swapchain, or NULL on failure.
 */
struct wlf_wl_egl_swapchain *wlf_wl_egl_swapchain_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);

/**
 * @brief Checks whether a swapchain uses the Wayland EGL implementation.
 * @param swapchain Generic swapchain to inspect.
 * @return true when @p swapchain is a Wayland EGL swapchain, false otherwise.
 */
bool wlf_swapchain_is_wl_egl(const struct wlf_swapchain *swapchain);

/**
 * @brief Casts a generic swapchain to a Wayland EGL swapchain.
 * @param swapchain Swapchain known to use the Wayland EGL implementation.
 * @return Enclosing Wayland EGL swapchain.
 * @note Asserts that the swapchain uses the Wayland EGL implementation.
 */
struct wlf_wl_egl_swapchain *wlf_wl_egl_swapchain_from_swapchain(struct wlf_swapchain *swapchain);

#endif // WL_EGL_SWAPCHAIN_H
