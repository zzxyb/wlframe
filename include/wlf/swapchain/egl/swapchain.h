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
#include "wlf/config.h"

#include <stdbool.h>

#if WLF_HAS_LINUX_PLATFORM
#include <wayland-egl-core.h>
#endif

#include <EGL/egl.h>

/**
 * @brief Swapchain backed by an EGL window surface.
 *
 * On Linux, @p egl_window wraps the native Wayland surface used to create the
 * EGL surface. The EGL configuration and surface are owned by this object.
 */
struct wlf_egl_swapchain {
	struct wlf_swapchain base; /**< Generic swapchain interface. */
	struct wlf_buffer buffer;  /**< Generic handle for the EGL surface. */

#if WLF_HAS_LINUX_PLATFORM
	struct wl_egl_window *egl_window; /**< Native Wayland EGL window. */
#endif
	EGLConfig config; /**< EGL configuration selected for the window. */
	EGLSurface surface; /**< EGL presentation surface. */
};

/**
 * @brief Creates an EGL swapchain for a window.
 * @param window Window receiving the swapchain.
 * @param width Initial buffer width in pixels.
 * @param height Initial buffer height in pixels.
 * @param format Requested EGL render format.
 * @return Newly allocated generic swapchain, or NULL on failure.
 */
struct wlf_swapchain *wlf_egl_swapchain_create(struct wlf_window *window,
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

/**
 * @brief Checks whether a generic buffer represents an EGL surface.
 */
bool wlf_buffer_is_egl(struct wlf_buffer *buffer);

/**
 * @brief Gets the EGL swapchain represented by a generic buffer.
 */
struct wlf_egl_swapchain *wlf_egl_swapchain_from_buffer(
	struct wlf_buffer *buffer);

#endif // EGL_SWAPCHAIN_H
