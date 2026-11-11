/**
 * @file        buffer.h
 * @brief       Wayland shared memory buffer implementation.
 * @details     This file provides a buffer implementation backed by POSIX shared memory.
 *              Wayland SHM buffers use memory-mapped files for CPU-accessible buffer storage
 *              suitable for software rendering and IPC.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-22, initial version\n
 */

#ifndef BUFFER_WLF_WL_SHM_BUFFER_H
#define BUFFER_WLF_WL_SHM_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

struct wl_buffer;
struct wlf_shm_allocator;

/**
 * @brief Wayland shared memory buffer attributes.
 *
 * Describes the layout and properties of a shared memory buffer.
 */
struct wlf_wl_shm_attributes {
	int fd;              /**< File descriptor of the shared memory */
	uint32_t format;     /**< FourCC pixel format code (see DRM_FORMAT_*) */
	uint32_t width;      /**< Buffer width in pixels */
	uint32_t height;     /**< Buffer height in pixels */
	uint32_t stride;     /**< Number of bytes between consecutive pixel lines */
	off_t offset;        /**< Offset in bytes of the first pixel in FD */
};

/**
 * @brief A Wayland buffer backed by shared memory.
 *
 * This buffer implementation uses POSIX shared memory (shm_open/mmap)
 * to provide CPU-accessible buffer storage suitable for software rendering.
 */
struct wlf_wl_shm_buffer {
	struct wlf_buffer base;              /**< Base buffer structure */
	struct wlf_wl_shm_attributes shm;       /**< Shared memory attributes */
	struct wl_buffer *wl_buffer;         /**< Wayland buffer object */
	void *data;                          /**< Mapped memory pointer */
	size_t size;                         /**< Total size of mapped memory in bytes */
};

/**
 * @brief Creates a Wayland SHM buffer with the specified format.
 *
 * This function is called by the SHM allocator to create buffers.
 *
 * @param alloc SHM allocator.
 * @param width Buffer width in pixels.
 * @param height Buffer height in pixels.
 * @param format Pixel format (DRM FourCC code).
 * @return Pointer to struct wlf_wl_shm_buffer on success, or NULL on failure.
 */
struct wlf_wl_shm_buffer *wlf_wl_shm_buffer_create(struct wlf_shm_allocator *alloc,
	int width, int height, uint32_t format);

/**
 * @brief Checks if a buffer is a Wayland SHM buffer.
 *
 * @param buffer Buffer to check.
 * @return true if the buffer is a Wayland SHM buffer, false otherwise.
 */
bool wlf_buffer_is_wl_shm(struct wlf_buffer *buffer);

/**
 * @brief Gets the Wayland SHM buffer from a generic buffer.
 *
 * @param buffer Generic buffer pointer.
 * @return Wayland SHM buffer pointer, or NULL if the buffer is not a Wayland SHM buffer.
 */
struct wlf_wl_shm_buffer *wlf_wl_shm_buffer_from_buffer(struct wlf_buffer *buffer);

/**
 * @brief Gets the Wayland SHM attributes of a Wayland SHM buffer.
 *
 * @param buffer Wayland SHM buffer.
 * @param attribs Output Wayland SHM attributes.
 * @return true on success, false on failure.
 */
bool wlf_wl_shm_buffer_get_shm(struct wlf_wl_shm_buffer *buffer,
	struct wlf_wl_shm_attributes *attribs);

/**
 * @brief Allocates a POSIX shared memory file descriptor.
 *
 * Creates an anonymous shared memory file of the specified size using
 * memfd_create or shm_open as a fallback.
 *
 * @param size Size of the shared memory region in bytes.
 * @return File descriptor on success, -1 on failure.
 */
int wlf_allocate_shm_file(size_t size);

/**
 * @brief Allocates read-write and read-only descriptors for the same SHM file.
 *
 * Creates a POSIX shared memory file of the specified size, unlinks its name,
 * and clears its permission bits. Both descriptors have FD_CLOEXEC set.
 *
 * @param size Size of the shared memory region in bytes.
 * @param rw_fd_ptr Non-NULL output pointer for the read-write descriptor.
 * @param ro_fd_ptr Non-NULL output pointer for the read-only descriptor;
 * must be distinct from @p rw_fd_ptr.
 * @return true on success, false on failure with both outputs unchanged.
 * @note The caller owns both returned descriptors and must close them.
 */
bool wlf_allocate_shm_file_pair(size_t size, int *rw_fd_ptr, int *ro_fd_ptr);

#endif // BUFFER_WLF_WL_SHM_BUFFER_H
