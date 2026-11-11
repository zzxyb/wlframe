/**
 * @file        swapchain.h
 * @brief       Wayland shared-memory swapchain.
 * @details     Declares the double-buffered wl_shm swapchain used by the
 *              Pixman renderer.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WL_SHM_SWAPCHAIN_H
#define WL_SHM_SWAPCHAIN_H

#include "wlf/swapchain/wlf_swapchain.h"

#include <stdbool.h>

/**
 * @brief Swapchain containing front and back CPU-renderable buffers.
 *
 * The front buffer is owned by the compositor after commit; the back buffer
 * is returned for the next rendering operation.
 */
struct wlf_wl_shm_swapchain {
	struct wlf_swapchain base; /**< Generic swapchain interface. */
	struct wlf_buffer *front;   /**< Buffer currently committed to compositor */
	struct wlf_buffer *back;    /**< Buffer available for rendering */
};

/**
 * @brief Creates a Wayland shared-memory swapchain for a window.
 * @param window Window receiving the swapchain.
 * @param width Initial buffer width in pixels.
 * @param height Initial buffer height in pixels.
 * @param format Requested wl_shm render format.
 * @return Newly allocated generic swapchain, or NULL on failure.
 */
struct wlf_wl_shm_swapchain *wlf_wl_shm_swapchain_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);

/**
 * @brief Checks whether a swapchain is backed by wl_shm.
 * @param swapchain Generic swapchain to inspect.
 * @return true when @p swapchain is wl_shm-backed, false otherwise.
 */
bool wlf_swapchain_is_wl_shm(const struct wlf_swapchain *swapchain);

/**
 * @brief Casts a generic swapchain to a Wayland shared-memory swapchain.
 * @param swapchain Swapchain known to be wl_shm-backed.
 * @return Enclosing wl_shm swapchain, or NULL when the type does not match.
 */
struct wlf_wl_shm_swapchain *wlf_wl_shm_swapchain_from_swapchain(struct wlf_swapchain *swapchain);

#endif // WL_SHM_SWAPCHAIN_H
