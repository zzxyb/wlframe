/**
 * @file        shm.h
 * @brief       Shared memory buffer attributes and structures.
 * @details     This file provides structures for shared memory buffer management,
 *              including SHM attributes and buffer/allocator definitions.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-22, initial version\n
 */

#ifndef ALLOCATOR_WLF_SHM_H
#define ALLOCATOR_WLF_SHM_H

#include <stdint.h>
#include <sys/types.h>

/**
 * @brief Shared memory buffer attributes.
 *
 * Describes the layout and properties of a shared memory buffer.
 */
struct wlf_shm_attributes {
	int fd;              /**< File descriptor of the shared memory */
	uint32_t format;     /**< FourCC pixel format code (see DRM_FORMAT_*) */
	uint32_t width;      /**< Buffer width in pixels */
	uint32_t height;     /**< Buffer height in pixels */
	uint32_t stride;     /**< Number of bytes between consecutive pixel lines */
	off_t offset;        /**< Offset in bytes of the first pixel in FD */
};

#endif // ALLOCATOR_WLF_SHM_H
