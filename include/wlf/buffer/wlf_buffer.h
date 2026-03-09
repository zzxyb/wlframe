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

/** @brief Forward declaration of wlf_buffer. */
struct wlf_buffer;

/**
 * @brief Flags for controlling raw pixel data pointer access.
 */
enum wlf_buffer_data_ptr_access_flag {
	WLF_BUFFER_DATA_PTR_ACCESS_READ  = 1 << 0,  /**< Request read access to pixel data */
	WLF_BUFFER_DATA_PTR_ACCESS_WRITE = 1 << 1,  /**< Request write access to pixel data */
};

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

	bool dropped;                /**< Whether the buffer has been dropped (pending destruction) */
	size_t n_locks;              /**< Lock counter for reference tracking */
	uint32_t width, height;      /**< Buffer dimensions in pixels */
	bool accessing_data_ptr;     /**< Flag indicating if raw data is currently being accessed */
};

/**
 * @brief A read-only buffer backed by a user-provided data pointer.
 *
 * Wraps a raw pixel data pointer as a wlf_buffer. The data is not copied;
 * the caller must ensure the pointer remains valid for the buffer's lifetime.
 */
struct wlf_readonly_data_buffer {
	struct wlf_buffer base;   /**< Base buffer object */

	const void *data;         /**< Pointer to the read-only pixel data */
	uint32_t format;          /**< Pixel format of the data */
	size_t stride;            /**< Row stride of the data in bytes */

	void *saved_data;         /**< Internal copy used during data pointer access */
};

/**
 * @brief Creates a read-only buffer from a raw pixel data pointer.
 *
 * @param format Pixel format of the data.
 * @param stride Row stride in bytes.
 * @param width Buffer width in pixels.
 * @param height Buffer height in pixels.
 * @param data Pointer to the pixel data (must remain valid for the buffer's lifetime).
 * @return Pointer to the new buffer, or NULL on failure.
 */
struct wlf_readonly_data_buffer *wlf_readonly_data_buffer_create(uint32_t format,
	size_t stride, uint32_t width, uint32_t height, const void *data);

/**
 * @brief Drops a read-only data buffer, releasing it when no longer locked.
 *
 * @param buffer Buffer to drop.
 * @return true if the buffer was immediately destroyed, false if still locked.
 */
bool wlf_readonly_data_buffer_drop(struct wlf_readonly_data_buffer *buffer);

/**
 * @brief Checks whether a buffer is a read-only data buffer.
 *
 * @param wlf_buffer Buffer to check.
 * @return true if the buffer was created with wlf_readonly_data_buffer_create().
 */
bool wlf_buffer_is_readonly_data(const struct wlf_buffer *wlf_buffer);

/**
 * @brief Downcasts a generic buffer to a read-only data buffer.
 *
 * The caller must ensure the buffer is actually a wlf_readonly_data_buffer
 * (e.g. by checking wlf_buffer_is_readonly_data() first).
 *
 * @param wlf_buffer Buffer to downcast.
 * @return Pointer to the wlf_readonly_data_buffer.
 */
struct wlf_readonly_data_buffer *wlf_readonly_data_buffer_from_buffer(
	struct wlf_buffer *wlf_buffer);

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
 * @brief Drops a buffer, destroying it immediately if there are no active locks.
 *
 * After calling this function the caller must not use the buffer pointer unless
 * it holds an active lock obtained via wlf_buffer_lock().
 *
 * @param buffer Buffer to drop.
 */
void wlf_buffer_drop(struct wlf_buffer *buffer);

/**
 * @brief Locks a buffer to prevent modifications.
 *
 * Buffers use reference counting for locks. Multiple locks are allowed,
 * and each lock must be paired with an unlock.
 *
 * @param buffer Buffer to lock.
 */
struct wlf_buffer *wlf_buffer_lock(struct wlf_buffer *buffer);

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
