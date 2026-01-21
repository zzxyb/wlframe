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
#include "wlf/dmabuf/wlf_dmabuf.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_swapchain;

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
	 * @brief Acquires a buffer from the swapchain.
	 * @param swapchain Swapchain to acquire from.
	 * @return Pointer to an acquired buffer, or NULL if none available.
	 */
	struct wlf_buffer *(*acquire)(struct wlf_swapchain *swapchain);

	/**
	 * @brief Checks if a buffer belongs to this swapchain.
	 * @param swapchain Swapchain to check against.
	 * @param buffer Buffer to verify.
	 * @return true if buffer belongs to this swapchain, false otherwise.
	 */
	bool (*has_buffer)(struct wlf_swapchain *swapchain,
		struct wlf_buffer *buffer);

	/**
	 * @brief Resizes the swapchain buffers.
	 * @param swapchain Swapchain to resize.
	 * @param width New width in pixels.
	 * @param height New height in pixels.
	 * @return true on success, false on failure.
	 */
	bool (*resize)(struct wlf_swapchain *swapchain, int width, int height);
};

/**
 * @brief A single slot in the swapchain.
 *
 * Each slot contains a buffer and tracks whether it's currently acquired
 * by the caller (and thus waiting for release).
 */
struct wlf_swapchain_slot {
	struct wlf_buffer *buffer;     /**< Buffer stored in this slot */
	bool acquired;                 /**< True if buffer is acquired and waiting for release */

	struct {
		struct wlf_listener release;  /**< Listener for buffer release events */
	} listener;
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

	int width;                             /**< Width of all buffers in pixels */
	int height;                            /**< Height of all buffers in pixels */
	struct wlf_dmabuf_attributes format;   /**< Format and attributes for all buffers */

	struct wlf_swapchain_slot *slots;      /**< Dynamic array of buffer slots */
	size_t slot_count;                     /**< Number of slots in the swapchain */

	struct {
		struct wlf_signal destroy;         /**< Signal emitted when swapchain is destroyed */
	} events;

	struct {
		struct wlf_listener allocator_destroy; /**< Listener for allocator destruction */
	} listener;
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
 * @return true on success, false on failure.
 */
bool wlf_swapchain_init(struct wlf_swapchain *swapchain,
	const struct wlf_swapchain_impl *impl,
	struct wlf_allocator *alloc, int width, int height,
	const struct wlf_dmabuf_attributes *format, size_t slot_count);

/**
 * @brief Finalizes the base swapchain structure.
 *
 * This function should be called by implementation-specific destroy functions
 * to clean up common swapchain resources.
 *
 * @param swapchain Swapchain to finalize.
 */
void wlf_swapchain_finish(struct wlf_swapchain *swapchain);

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
 * @brief Acquires a buffer from the swapchain.
 *
 * Searches for a free buffer slot and returns its buffer. If no buffer
 * is free, returns NULL. The returned buffer is locked and marked as
 * acquired. When the caller is done with it, they must unlock it by
 * calling wlf_buffer_unlock(), which will trigger the swapchain to
 * mark it as available again.
 *
 * @param swapchain Swapchain to acquire from.
 * @return Pointer to a locked buffer, or NULL if no buffer is available.
 */
struct wlf_buffer *wlf_swapchain_acquire(struct wlf_swapchain *swapchain);

/**
 * @brief Checks if a buffer belongs to this swapchain.
 *
 * Determines whether the given buffer was created by and is managed
 * by this swapchain.
 *
 * @param swapchain Swapchain to check against.
 * @param buffer Buffer to verify.
 * @return true if the buffer belongs to this swapchain, false otherwise.
 */
bool wlf_swapchain_has_buffer(struct wlf_swapchain *swapchain,
	struct wlf_buffer *buffer);

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

#endif // SWAPCHAIN_WLF_SWAPCHAIN_H
