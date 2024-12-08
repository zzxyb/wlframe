#ifndef WLF_SWAPCHAIN_H
#define WLF_SWAPCHAIN_H

#include "wlf/utils/wlf_double_list.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

#define WLF_SWAPCHAIN_CAP 4

/**
 * @brief A structure representing a slot in the swapchain
 */
struct wlf_swapchain_slot {
	struct wlf_buffer *buffer;  /**< Pointer to the buffer in the slot */
	bool acquired;              /**< Indicates if the buffer is currently acquired (waiting for release) */

	struct wlf_listener release;  /**< Listener for buffer release events */
};

/**
 * @brief A structure representing a swapchain
 */
struct wlf_swapchain {
	struct wlf_allocator *allocator;  /**< Pointer to the allocator, NULL if destroyed */
	int width;                        /**< Width of the swapchain */
	int height;                       /**< Height of the swapchain */
	struct wlf_drm_format format;     /**< Format of the swapchain buffers */
	struct wlf_swapchain_slot slots[WLF_SWAPCHAIN_CAP];  /**< Array of swapchain slots */

	struct wlf_listener allocator_destroy;  /**< Listener for allocator destroy events */
};

/**
 * @brief Creates a new swapchain
 * @param alloc Pointer to the allocator to use for the swapchain
 * @param width Width of the swapchain
 * @param height Height of the swapchain
 * @param format Pointer to the DRM format for the swapchain buffers
 * @return Pointer to the newly created wlf_swapchain structure
 */
struct wlf_swapchain *wlf_swapchain_create(
	struct wlf_allocator *alloc, int width, int height,
	const struct wlf_drm_format *format);

/**
 * @brief Destroys a swapchain and frees associated resources
 * @param swapchain Pointer to the swapchain to destroy
 */
void wlf_swapchain_destroy(struct wlf_swapchain *swapchain);

/**
 * @brief Acquires a buffer from the swapchain
 * @param swapchain Pointer to the swapchain to acquire from
 * @return Pointer to the acquired buffer, which is locked
 *
 * The caller must unlock the buffer by calling wlf_buffer_unlock when done.
 */
struct wlf_buffer *wlf_swapchain_acquire(struct wlf_swapchain *swapchain);

/**
 * @brief Checks if a buffer has been created by this swapchain
 * @param swapchain Pointer to the swapchain to check
 * @param buffer Pointer to the buffer to check
 * @return true if the buffer was created by this swapchain, false otherwise
 */
bool wlf_swapchain_has_buffer(struct wlf_swapchain *swapchain,
	struct wlf_buffer *buffer);

#endif
