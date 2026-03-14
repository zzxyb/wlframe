/**
 * @file        wlf_shm_allocator.h
 * @brief       Shared memory buffer allocator.
 * @details     This file provides a buffer allocator implementation using POSIX shared memory.
 *              The allocator creates CPU-accessible buffers suitable for software rendering.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-22, initial version\n
 */

#ifndef ALLOCATOR_WLF_SHM_ALLOCATOR_H
#define ALLOCATOR_WLF_SHM_ALLOCATOR_H

#include "wlf/allocator/wlf_allocator.h"

#include <stdbool.h>

/**
 * @brief A shared memory-based buffer allocator.
 *
 * This allocator creates buffers backed by POSIX shared memory,
 * providing CPU-accessible buffer storage suitable for software rendering.
 */
struct wlf_shm_allocator {
	struct wlf_allocator base;  /**< Base allocator structure */
};

/**
 * @brief Creates a new SHM allocator.
 *
 * Creates an allocator that uses POSIX shared memory to allocate
 * CPU-accessible buffers.
 *
 * @return Pointer to the created allocator, or NULL on failure.
 */
struct wlf_allocator *wlf_shm_allocator_create(void);

/**
 * @brief Gets the SHM allocator from a generic allocator.
 *
 * @param allocator Generic allocator pointer.
 * @return SHM allocator pointer, or NULL if the allocator is not a SHM allocator.
 */
struct wlf_shm_allocator *wlf_shm_allocator_from_allocator(
	struct wlf_allocator *allocator);

/**
 * @brief Checks if an allocator is a SHM allocator.
 *
 * @param allocator Allocator to check.
 * @return true if the allocator is a SHM allocator, false otherwise.
 */
bool wlf_allocator_is_shm(struct wlf_allocator *allocator);

#endif // ALLOCATOR_WLF_SHM_ALLOCATOR_H
