/**
 * @file        wlf_wl_shm.h
 * @brief       Wayland wl_shm global wrapper for wlframe.
 * @details     Wraps wl_shm (shared memory) bound from the Wayland registry.
 *              Tracks available pixel formats and provides wl_shm_pool creation.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_SHM_H
#define WAYLAND_WLF_WL_SHM_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_shm;
struct wl_shm_pool;
struct wlf_wl_shm_pool;

/**
 * @brief Wayland shared memory global.
 */
struct wlf_wl_shm {
	struct wl_shm *wl_shm; /**< Underlying Wayland object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before destruction. */
		struct wlf_signal format;  /**< Payload: wlf_wl_shm (wl_shm->format). */
	} events;
};

/**
 * @brief Binds wl_shm from the Wayland registry.
 *
 * @param wl_registry Wayland registry.
 * @param name        Global name of wl_shm.
 * @param version     Advertised version.
 * @return Newly allocated wlf_wl_shm, or NULL on failure.
 */
struct wlf_wl_shm *wlf_wl_shm_create(struct wl_registry *wl_registry,
	uint32_t name, uint32_t version);

/**
 * @brief Destroys a wlf_wl_shm. Passing NULL is a no-op.
 */
void wlf_wl_shm_destroy(struct wlf_wl_shm *shm);

/**
 * @brief Creates a wl_shm_pool from this shm global.
 *
 * @param shm  Shared memory global.
 * @param fd   File descriptor for the shared memory.
 * @param size Pool size in bytes.
 * @return Newly allocated wlf_wl_shm_pool, or NULL on failure.
 */
struct wlf_wl_shm_pool *wlf_wl_shm_create_pool(struct wlf_wl_shm *shm,
	int fd, int32_t size);

#endif /* WAYLAND_WLF_WL_SHM_H */
