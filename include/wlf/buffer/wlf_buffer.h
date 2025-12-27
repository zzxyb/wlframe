/**
 * @file        wlf_buffer.h
 * @brief       Buffer management utility for wlframe.
 * @details     This file provides structures and functions for buffer operations,
 *              including buffer creation, destruction, locking, data access, and opacity region queries.
 * @author      YaoBing Xiao
 * @date        2026-01-12
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-12, initial version\n
 */

#ifndef BUFFER_WLF_BUFFER_H
#define BUFFER_WLF_BUFFER_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_region.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_buffer;

/**
 * @brief Virtual methods for buffer operations.
 *
 * This structure defines the interface that buffer implementations must provide.
 */
struct wlf_buffer_impl {
	/**
	 * @brief Destroys the buffer.
	 * @param buffer Buffer to destroy.
	 */
	void (*destroy)(struct wlf_buffer *buffer);

	/**
	 * @brief Begins accessing the buffer's raw pixel data.
	 * @param buffer Buffer to access.
	 * @param flags Access flags (implementation-specific).
	 * @param data Output pointer to pixel data.
	 * @param format Output pixel format.
	 * @param stride Output row stride in bytes.
	 * @return true if access succeeded, false otherwise.
	 */
	bool (*begin_data_ptr_access)(struct wlf_buffer *buffer, uint32_t flags,
		void **data, uint32_t *format, size_t *stride);

	/**
	 * @brief Ends accessing the buffer's raw pixel data.
	 * @param buffer Buffer to stop accessing.
	 */
	void (*end_data_ptr_access)(struct wlf_buffer *buffer);

	/**
	 * @brief Gets the opaque region of the buffer.
	 * @param buffer Buffer to query.
	 * @return Opaque region or NULL if no specific region is opaque.
	 */
	const struct wlf_region *(*opaque_region)(struct wlf_buffer *buffer);
};

/**
 * @brief A buffer object representing image data in memory.
 *
 * Buffers are used to store pixel data and can be locked to prevent
 * concurrent modifications. They also support accessing raw pixel data
 * through a pointer-based interface.
 */
struct wlf_buffer {
	const struct wlf_buffer_impl *impl;  /**< Virtual method table */
	struct {
		struct wlf_signal destroy;       /**< Signal emitted when buffer is destroyed */
	} events;

	size_t n_locks;              /**< Lock counter for reference tracking */
	uint32_t width, height;      /**< Buffer dimensions in pixels */
	bool accessing_data_ptr;     /**< Flag indicating if raw data is currently being accessed */
};

/**
 * @brief Initializes a buffer.
 *
 * @param buffer Buffer to initialize.
 * @param impl Implementation methods for this buffer.
 * @param width Buffer width in pixels.
 * @param height Buffer height in pixels.
 */
void wlf_buffer_init(struct wlf_buffer *buffer,
	const struct wlf_buffer_impl *impl, uint32_t width, uint32_t height);

/**
 * @brief Destroys a buffer.
 *
 * @param buffer Buffer to destroy.
 */
void wlf_buffer_destroy(struct wlf_buffer *buffer);

/**
 * @brief Locks a buffer to prevent modifications.
 *
 * Buffers use reference counting for locks. Multiple locks are allowed,
 * and each lock must be paired with an unlock.
 *
 * @param buffer Buffer to lock.
 */
void wlf_buffer_lock(struct wlf_buffer *buffer);

/**
 * @brief Unlocks a buffer to allow modifications.
 *
 * @param buffer Buffer to unlock.
 */
void wlf_buffer_unlock(struct wlf_buffer *buffer);

/**
 * @brief Checks if the buffer is completely opaque.
 *
 * @param buffer Buffer to check.
 * @return true if buffer is opaque, false otherwise.
 */
bool wlf_buffer_is_opaque(struct wlf_buffer *buffer);

/**
 * @brief Begins accessing the buffer's raw pixel data.
 *
 * This function provides access to the underlying pixel data for reading or writing.
 * Each call must be paired with wlf_buffer_end_data_ptr_access().
 *
 * @param buffer Buffer to access.
 * @param flags Access flags (implementation-specific).
 * @param data Output pointer to pixel data.
 * @param format Output pixel format.
 * @param stride Output row stride in bytes.
 * @return true if access succeeded, false otherwise.
 */
bool wlf_buffer_begin_data_ptr_access(struct wlf_buffer *buffer, uint32_t flags,
	void **data, uint32_t *format, size_t *stride);

/**
 * @brief Ends accessing the buffer's raw pixel data.
 *
 * @param buffer Buffer to stop accessing.
 */
void wlf_buffer_end_data_ptr_access(struct wlf_buffer *buffer);

#endif // BUFFER_WLF_BUFFER_H
