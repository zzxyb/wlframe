/**
 * @file        wlf_swapchain.h
 * @brief       Swapchain management for buffer rotation and presentation.
 * @details     This file provides structures and functions for managing a swapchain
 *              of buffers used in window rendering. The swapchain rotates between
 *              multiple buffers to enable smooth double/triple buffering without
 *              tearing or blocking.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-22, initial version\n
 */

#ifndef SWAPCHAIN_WLF_SWAPCHAIN_H
#define SWAPCHAIN_WLF_SWAPCHAIN_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/allocator/wlf_allocator.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

#include <pixman.h>

struct wlf_swapchain;
struct wlf_window;

/**
 * @brief Virtual methods for swapchain operations.
 *
 * This structure defines the interface that swapchain implementations
 * (Vulkan, Pixman, etc.) must provide.
 */
struct wlf_swapchain_impl {
	/**
	 * @brief Destroys the swapchain implementation.
	 * @param swapchain Swapchain to destroy.
	 */
	void (*destroy)(struct wlf_swapchain *swapchain);

	/**
	 * @brief Resizes the swapchain buffers.
	 * @param swapchain Swapchain to resize.
	 * @param width New width in pixels.
	 * @param height New height in pixels.
	 * @return true on success, false on failure.
	 */
	bool (*resize)(struct wlf_swapchain *swapchain, int width, int height);

	/**
	 * @brief Presents the current back buffer to the display.
	 *
	 * Commits the buffer returned by the last acquire() call to the
	 * compositor (attach + damage + commit) and rotates the swapchain.
	 * Must be called after rendering into the acquired buffer.
	 *
	 * @param swapchain Swapchain to present.
	 */
	void (*present)(struct wlf_swapchain *swapchain, const pixman_region32_t *damage);
};

/**
 * @brief A swapchain for managing multiple buffers.
 *
 * The swapchain manages a ring of buffers of the same size and format,
 * allowing applications to acquire free buffers for rendering and
 * automatically recycle them when released.
 *
 * This is an abstract base structure. Specific implementations (Vulkan, Pixman)
 * should embed this structure and provide their own implementation methods.
 */
struct wlf_swapchain {
	const struct wlf_swapchain_impl *impl; /**< Virtual method table */
	struct wlf_allocator *allocator;       /**< Allocator used to create buffers (NULL if destroyed) */
	struct wlf_window *window;
	struct wlf_buffer *back;                /**< Generic buffer used for rendering */

	int width, height;                     /**< Size of all buffers in pixels */
	struct wlf_render_format format;

	struct {
		struct wlf_signal destroy;         /**< Signal emitted when swapchain is destroyed */
	} events;
};

/**
 * @brief Initializes the base swapchain structure.
 *
 * This function should be called by implementation-specific creation functions
 * (e.g., wlf_vulkan_swapchain_create, wlf_pixman_swapchain_create) to initialize
 * the common swapchain fields.
 *
 * @param swapchain Swapchain to initialize.
 * @param impl Implementation methods for this swapchain.
 * @param alloc Allocator to use for creating buffers.
 * @param width Width of buffers in pixels.
 * @param height Height of buffers in pixels.
 * @param format Format and attributes for the buffers.
 * @param slot_count Number of buffer slots (typically 2-4 depending on backend capabilities).
 */
void wlf_swapchain_init(struct wlf_swapchain *swapchain, struct wlf_allocator *allocator,
	const struct wlf_swapchain_impl *impl, int width, int height);

struct wlf_swapchain *wlf_swapchain_auto_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);

/**
 * @brief Returns the buffer currently available for rendering.
 *
 * @param swapchain Swapchain to query.
 * @return Back buffer, or NULL if the swapchain has no generic buffer.
 */
struct wlf_buffer *wlf_swapchain_get_back_buffer(
	struct wlf_swapchain *swapchain);

/**
 * @brief Destroys a swapchain.
 *
 * Frees all resources associated with the swapchain, including any
 * buffers that are not currently acquired. Acquired buffers will be
 * destroyed when they are released.
 *
 * @param swapchain Swapchain to destroy.
 */
void wlf_swapchain_destroy(struct wlf_swapchain *swapchain);

/**
 * @brief Resizes all buffers in the swapchain.
 *
 * Updates the dimensions of the swapchain and recreates all buffers
 * with the new size. Currently acquired buffers will be destroyed when released.
 *
 * @param swapchain Swapchain to resize.
 * @param width New width in pixels.
 * @param height New height in pixels.
 * @return true on success, false on failure.
 */
bool wlf_swapchain_resize(struct wlf_swapchain *swapchain, int width, int height);

/**
 * @brief Presents the current back buffer to the display.
 *
 * Commits the buffer returned by the last wlf_swapchain_acquire() to the
 * compositor with the accumulated damage region, then swaps front and back.
 * Must be called after rendering into the acquired buffer.
 *
 * @param swapchain Swapchain to present.
 * @param damage Swapchain damage region to present.
 */
void wlf_swapchain_present(struct wlf_swapchain *swapchain, const pixman_region32_t *damage);

#endif // SWAPCHAIN_WLF_SWAPCHAIN_H
