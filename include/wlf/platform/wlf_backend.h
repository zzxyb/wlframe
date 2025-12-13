/**
 * @file        wlf_backend.h
 * @brief       Backend abstraction for wlframe.
 * @details     This file provides a unified interface for the Wayland backend.
 *              The backend implements the wlf_backend_impl interface and can be
 *              loaded either statically or dynamically.
 *
 *              Typical usage:
 *                  - Auto-create backend: wlf_backend_autocreate()
 *                  - Manual backend selection: wlf_backend_create()
 *                  - Start/stop backend: wlf_backend_start()/wlf_backend_stop()
 *
 * @author      YaoBing Xiao
 * @date        2025-06-25
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-25, initial version\n
 */

#ifndef PLATFORM_WLF_BACKEND_H
#define PLATFORM_WLF_BACKEND_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/types/wlf_output.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_backend;

/**
 * @brief Backend implementation interface
 * This structure defines the function pointers that each backend must implement
 */
struct wlf_backend_impl {
	const char *name;
	/**
	 * @brief Destroy the backend and free resources
	 * @param backend Pointer to the backend
	 */
	void (*destroy)(struct wlf_backend *backend);
};

/**
 * @brief Main backend structure
 */
struct wlf_backend {
	const struct wlf_backend_impl *impl;  /**< Backend implementation */

	struct {
		struct wlf_signal destroy;        /**< Emitted when backend is destroyed */
		struct wlf_signal output_added;    /**< Output added */
		struct wlf_signal output_removed;  /**< Output removed */
	} events;

	/**
	 * Backend-specific data pointer.
	 * Each backend implementation can store its private data here.
	 */
	void *data;

	struct wlf_linked_list outputs;
};

void wlf_backend_init(struct wlf_backend *backend,
	const struct wlf_backend_impl *impl);

/**
 * @brief Auto-create the best available backend for the current environment
 * @return Pointer to created backend, or NULL on failure
 */
struct wlf_backend *wlf_backend_autocreate(void);

/**
 * @brief Destroy a backend and free all resources
 * @param backend Pointer to the backend
 */
void wlf_backend_destroy(struct wlf_backend *backend);

#endif // PLATFORM_WLF_BACKEND_H
