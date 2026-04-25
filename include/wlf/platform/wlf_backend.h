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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct wlf_backend;

typedef void (*wlf_backend_event_source_dispatch_t)(
	struct wlf_backend *backend, int fd, uint32_t revents, void *data);

/**
 * @brief Backend implementation interface
 * This structure defines the function pointers that each backend must implement
 */
struct wlf_backend_impl {
	const char *name;  /**< Backend implementation name */
	/**
	 * @brief Destroy the backend and free resources
	 * @param backend Pointer to the backend
	 */
	void (*destroy)(struct wlf_backend *backend);
	/**
	 * @brief Run the backend main event loop
	 * @param backend Pointer to the backend
	 */
	void (*exe)(struct wlf_backend *backend);
};

/**
 * @brief Main backend structure
 */
struct wlf_backend {
	const struct wlf_backend_impl *impl;  /**< Backend implementation */
	bool running;  /**< True while backend event loop is running */

	struct {
		int fd;  /**< File descriptor to monitor */
		short events;  /**< poll(2) event mask */
		wlf_backend_event_source_dispatch_t dispatch;  /**< Event callback */
		void *data;  /**< User data passed to callback */
	} *event_sources;  /**< Dynamic event source array */
	size_t event_source_count;  /**< Number of active event sources */
	size_t event_source_capacity;  /**< Allocated event source capacity */

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

	struct wlf_linked_list outputs;  /**< Managed output list */
};

/**
 * @brief Initialize a backend object
 * @param backend Pointer to backend object to initialize
 * @param impl Backend implementation vtable
 */
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

/**
 * @brief Execute backend event loop
 * @param backend Pointer to backend
 */
void wlf_backend_exe(struct wlf_backend *backend);

/**
 * @brief Register an external file descriptor event source
 * @param backend Pointer to backend
 * @param fd File descriptor to poll
 * @param events poll(2) event mask
 * @param dispatch Callback invoked when events occur
 * @param data User data passed to dispatch callback
 * @return true on success, false on failure
 */
bool wlf_backend_add_event_source(struct wlf_backend *backend,
	int fd, short events,
	wlf_backend_event_source_dispatch_t dispatch,
	void *data);

/**
 * @brief Unregister a previously added event source
 * @param backend Pointer to backend
 * @param fd File descriptor used during registration
 * @param data User data used during registration
 * @return true if removed, false if not found
 */
bool wlf_backend_remove_event_source(struct wlf_backend *backend,
	int fd, void *data);

/**
 * @brief Request backend event loop termination
 * @param backend Pointer to backend
 */
void wlf_backend_quit(struct wlf_backend *backend);

#endif // PLATFORM_WLF_BACKEND_H
