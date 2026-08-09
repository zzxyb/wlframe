/**
 * @file        wlf_wl_shm_pool.h
 * @brief       Wayland wl_shm_pool wrapper for wlframe.
 * @details     Wraps a wl_shm_pool, providing buffer creation and pool resizing.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_SHM_POOL_H
#define WAYLAND_WLF_WL_SHM_POOL_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_buffer;
struct wl_shm_pool;
struct wlf_wl_buffer;

/**
 * @brief Wayland shared memory pool.
 */
struct wlf_wl_shm_pool {
	struct wl_shm_pool *wl_shm_pool; /**< Underlying Wayland object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before destruction. */
	} events;
};

/**
 * @brief Destroys a wlf_wl_shm_pool. Passing NULL is a no-op.
 */
void wlf_wl_shm_pool_destroy(struct wlf_wl_shm_pool *pool);

/**
 * @brief Creates a wl_buffer from this pool.
 *
 * @param pool   Shared memory pool.
 * @param offset Byte offset within the pool.
 * @param width  Buffer width in pixels.
 * @param height Buffer height in pixels.
 * @param stride Bytes per row.
 * @param format Pixel format (wl_shm_format).
 * @return Newly allocated wlf_wl_buffer, or NULL on failure.
 */
struct wlf_wl_buffer *wlf_wl_shm_pool_create_buffer(struct wlf_wl_shm_pool *pool,
	int32_t offset, int32_t width, int32_t height,
	int32_t stride, uint32_t format);

/**
 * @brief Resizes the pool.
 *
 * @param pool Pool to resize.
 * @param size New size in bytes (must be larger than current size).
 */
void wlf_wl_shm_pool_resize(struct wlf_wl_shm_pool *pool, int32_t size);

#endif /* WAYLAND_WLF_WL_SHM_POOL_H */
