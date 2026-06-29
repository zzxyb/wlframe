/**
 * @file        swapchain.h
 * @brief       EGL window-surface swapchain.
 * @details     Declares the swapchain implementation used by the GLES
 *              renderer to acquire and present Wayland window buffers.
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
#include "wlf/config.h"

#include <stdbool.h>

#if WLF_HAS_LINUX_PLATFORM
#include <wayland-egl-core.h>
#endif

#include <EGL/egl.h>

/**
 * @brief Swapchain backed by an EGL window surface.
 *
 * The generic swapchain base is embedded so the object can be used through
 * the renderer-independent swapchain interface.
 */
struct wlf_egl_swapchain {
	struct wlf_swapchain base; /**< Generic swapchain interface. */

#if WLF_HAS_LINUX_PLATFORM
	struct wl_egl_window *egl_window;
#endif
	EGLConfig config; /**< EGL configuration used for the surface. */
	EGLSurface surface; /**< EGL window surface. */
	bool swap_interval_configured; /**< Whether the swap interval was set. */
};

/**
 * @brief Creates an EGL swapchain for a window and render format.
 * @param window Window receiving the swapchain.
 * @param width Initial buffer width in pixels.
 * @param height Initial buffer height in pixels.
 * @param format Requested render format.
 * @return Newly allocated generic swapchain, or NULL on failure.
 */
struct wlf_swapchain *wlf_egl_swapchain_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);

/**
 * @brief Checks whether a swapchain is backed by EGL.
 * @param swapchain Generic swapchain to inspect.
 * @return true when @p swapchain is an EGL swapchain, false otherwise.
 */
bool wlf_swapchain_is_egl(const struct wlf_swapchain *swapchain);

/**
 * @brief Casts a generic swapchain to an EGL swapchain.
 * @param swapchain Swapchain known to be EGL-backed.
 * @return Enclosing EGL swapchain, or NULL when the type does not match.
 */
struct wlf_egl_swapchain *wlf_egl_swapchain_from_swapchain(struct wlf_swapchain *swapchain);

#endif // EGL_SWAPCHAIN_H
