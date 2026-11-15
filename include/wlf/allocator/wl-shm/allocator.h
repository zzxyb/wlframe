/**
 * @file        allocator.h
 * @brief       Wayland shared memory buffer allocator.
 * @details     This file provides a buffer allocator implementation using Wayland shared memory
 *              (wl_shm). The allocator creates CPU-accessible wl_buffer objects suitable for
 *              compositing via a Wayland compositor.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-22, initial version\n
 */

#ifndef WL_SHM_ALLOCATOR_H
#define WL_SHM_ALLOCATOR_H

#include "wlf/allocator/wlf_allocator.h"

#include <stdbool.h>

struct wl_shm;

/**
 * @brief A Wayland shared memory buffer allocator.
 *
 * This allocator creates wl_buffer objects via wl_shm_pool, backed by
 * memory-mapped POSIX shared memory. Buffers are CPU-accessible and
 * usable directly by a Wayland compositor.
 */
struct wlf_wl_shm_allocator {
	struct wlf_allocator base;  /**< Base allocator structure */
	struct wl_shm *wl_shm;      /**< Wayland shared memory global */
};

/**
 * @brief Creates a new Wayland SHM allocator.
 *
 * Creates an allocator that uses wl_shm to allocate CPU-accessible
 * wl_buffer objects backed by POSIX shared memory.
 *
 * @param wl_shm Wayland shared memory global to use for buffer creation.
 * @return Pointer to the created allocator, or NULL on failure.
 */
struct wlf_wl_shm_allocator *wlf_wl_shm_allocator_create(struct wl_shm *wl_shm);

/**
 * @brief Checks if an allocator is a Wayland SHM allocator.
 *
 * @param allocator Allocator to check.
 * @return true if the allocator is a Wayland SHM allocator, false otherwise.
 */
bool wlf_allocator_is_wl_shm(const struct wlf_allocator *allocator);

/**
 * @brief Gets the Wayland SHM allocator from a generic allocator.
 *
 * @param allocator Generic allocator pointer.
 * @return Wayland SHM allocator pointer.
 * @note Asserts that the allocator uses the Wayland SHM implementation.
 */
struct wlf_wl_shm_allocator *wlf_wl_shm_allocator_from_allocator(
	struct wlf_allocator *allocator);

#endif // WL_SHM_ALLOCATOR_H
