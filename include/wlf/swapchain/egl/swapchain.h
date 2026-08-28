/**
 * @file        swapchain.h
 * @brief       EGL swapchain.
 * @details     Declares the EGL-backed swapchain used by the GLES renderer
 *              to present frames for a wlframe window.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef EGL_SWAPCHAIN_H
#define EGL_SWAPCHAIN_H

#include "wlf/swapchain/wlf_swapchain.h"

#include <stdbool.h>

/**
 * @brief Swapchain backed by an EGL window surface.
 *
 * The EGL-backed buffer is created by the swapchain's EGL allocator. The
 * swapchain retains the buffer handle and owns presentation policy, while the
 * buffer owns the EGL surface resources.
 */
struct wlf_egl_swapchain {
	struct wlf_swapchain base; /**< Generic swapchain interface. */
	struct wlf_buffer *back; /**< EGL-backed buffer available for rendering. */
};

/**
 * @brief Creates an EGL swapchain for a window.
 * @param window Window receiving the swapchain.
 * @param width Initial buffer width in pixels.
 * @param height Initial buffer height in pixels.
 * @param format Requested EGL render format.
 * @return Newly allocated generic swapchain, or NULL on failure.
 */
struct wlf_egl_swapchain *wlf_egl_swapchain_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);

/**
 * @brief Checks whether a swapchain is EGL-backed.
 * @param swapchain Generic swapchain to inspect.
 * @return true when @p swapchain is EGL-backed, false otherwise.
 */
bool wlf_swapchain_is_egl(const struct wlf_swapchain *swapchain);

/**
 * @brief Casts a generic swapchain to an EGL swapchain.
 * @param swapchain Swapchain known to be EGL-backed.
 * @return Enclosing EGL swapchain, or NULL when the type does not match.
 */
struct wlf_egl_swapchain *wlf_egl_swapchain_from_swapchain(struct wlf_swapchain *swapchain);

#endif // EGL_SWAPCHAIN_H
