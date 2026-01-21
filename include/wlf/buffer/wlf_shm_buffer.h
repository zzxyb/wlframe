/**
 * @file        wlf_shm_buffer.h
 * @brief       Shared memory buffer implementation.
 * @details     This file provides a buffer implementation backed by POSIX shared memory.
 *              SHM buffers use memory-mapped files for CPU-accessible buffer storage
 *              suitable for software rendering and IPC.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-22, initial version\n
 */

#ifndef BUFFER_WLF_SHM_BUFFER_H
#define BUFFER_WLF_SHM_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/allocator/shm/shm.h"

#include <stdbool.h>
#include <stddef.h>

struct wlf_shm_allocator;

/**
 * @brief A buffer backed by shared memory.
 *
 * This buffer implementation uses POSIX shared memory (shm_open/mmap)
 * to provide CPU-accessible buffer storage suitable for software rendering.
 */
struct wlf_shm_buffer {
	struct wlf_buffer base;              /**< Base buffer structure */
	struct wlf_shm_attributes shm;       /**< Shared memory attributes */
	void *data;                          /**< Mapped memory pointer */
	size_t size;                         /**< Total size of mapped memory in bytes */
};

/**
 * @brief Creates a SHM buffer with the specified format.
 *
 * This function is called by the SHM allocator to create buffers.
 *
 * @param alloc SHM allocator.
 * @param width Buffer width in pixels.
 * @param height Buffer height in pixels.
 * @param format Pixel format (DRM FourCC code).
 * @return Pointer to the created buffer, or NULL on failure.
 */
struct wlf_shm_buffer *wlf_shm_buffer_create(struct wlf_shm_allocator *alloc,
	int width, int height, uint32_t format);

/**
 * @brief Gets the SHM buffer from a generic buffer.
 *
 * @param buffer Generic buffer pointer.
 * @return SHM buffer pointer, or NULL if the buffer is not a SHM buffer.
 */
struct wlf_shm_buffer *wlf_shm_buffer_from_buffer(struct wlf_buffer *buffer);

/**
 * @brief Checks if a buffer is a SHM buffer.
 *
 * @param buffer Buffer to check.
 * @return true if the buffer is a SHM buffer, false otherwise.
 */
bool wlf_buffer_is_shm(struct wlf_buffer *buffer);

/**
 * @brief Gets the SHM attributes of a SHM buffer.
 *
 * @param buffer SHM buffer.
 * @param attribs Output SHM attributes.
 * @return true on success, false on failure.
 */
bool wlf_shm_buffer_get_shm(struct wlf_shm_buffer *buffer,
	struct wlf_shm_attributes *attribs);

#endif // BUFFER_WLF_SHM_BUFFER_H
