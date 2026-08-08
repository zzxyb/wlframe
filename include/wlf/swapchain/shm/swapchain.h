/**
 * @file        swapchain.h
 * @brief       Shared-memory swapchain.
 * @details     Declares the double-buffered wl_shm swapchain used by the
 *              Pixman renderer.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef SHM_SWAPCHAIN_H
#define SHM_SWAPCHAIN_H

#include "wlf/swapchain/wlf_swapchain.h"

#include <stdbool.h>

/**
 * @brief Swapchain containing front and back CPU-renderable buffers.
 *
 * The front buffer is owned by the compositor after commit; the back buffer
 * is returned for the next rendering operation.
 */
struct wlf_shm_swapchain {
	struct wlf_swapchain base; /**< Generic swapchain interface. */
	struct wlf_buffer *front;   /**< Buffer currently committed to compositor */
	struct wlf_buffer *back;    /**< Buffer available for rendering */
};

/**
 * @brief Creates a shared-memory swapchain for a window.
 * @param window Window receiving the swapchain.
 * @param width Initial buffer width in pixels.
 * @param height Initial buffer height in pixels.
 * @param format Requested wl_shm render format.
 * @return Newly allocated generic swapchain, or NULL on failure.
 */
struct wlf_swapchain *wlf_shm_swapchain_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);

/**
 * @brief Checks whether a swapchain is shared-memory backed.
 * @param swapchain Generic swapchain to inspect.
 * @return true when @p swapchain is SHM-backed, false otherwise.
 */
bool wlf_swapchain_is_shm(const struct wlf_swapchain *swapchain);

/**
 * @brief Casts a generic swapchain to a shared-memory swapchain.
 * @param swapchain Swapchain known to be SHM-backed.
 * @return Enclosing SHM swapchain, or NULL when the type does not match.
 */
struct wlf_shm_swapchain *wlf_shm_swapchain_from_swapchain(struct wlf_swapchain *swapchain);

#endif // SHM_SWAPCHAIN_H
