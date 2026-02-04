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

/**
 * @brief Backend plugin entry point signature
 * Every backend plugin must export a function with this signature named "wlf_backend_plugin_init"
 */
typedef bool (*wlf_backend_plugin_init_func_t)(void);

/**
 * @brief Backend plugin cleanup signature
 * Every backend plugin should export a function with this signature named "wlf_backend_plugin_cleanup"
 */
typedef void (*wlf_backend_plugin_cleanup_func_t)(void);

/**
 * @brief Backend types enumeration
 */
enum wlf_backend_type {
	WLF_BACKEND_AUTOCREATE = 0,  /**< Auto-detect and create the best backend */
	WLF_BACKEND_WAYLAND,         /**< Wayland client backend */
	WLF_BACKEND_MACOS,           /**< macOS backend */
};

struct wlf_backend;

/**
 * @brief Backend implementation interface
 * This structure defines the function pointers that each backend must implement
 */
struct wlf_backend_impl {
	/**
	 * @brief Start the backend
	 * @param backend Pointer to the backend
	 * @return true on success, false on failure
	 */
	bool (*start)(struct wlf_backend *backend);

	/**
	 * @brief Stop the backend
	 * @param backend Pointer to the backend
	 */
	void (*stop)(struct wlf_backend *backend);

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
	enum wlf_backend_type type;           /**< Backend type */

	struct {
		struct wlf_signal destroy;        /**< Emitted when backend is destroyed */
	} events;

	/**
	 * Backend-specific data pointer.
	 * Each backend implementation can store its private data here.
	 */
	void *data;

	/**
	 * Output manager associated with this backend.
	 * Handles creation, removal, and tracking of all outputs provided
	 * by the backend (e.g., Wayland).
	 */
	struct wlf_output_manager *output_manager;
};

/**
 * @brief Backend registry entry for plugin system
 */
struct wlf_backend_registry_entry {
	struct wlf_linked_list link;          /**< Linked list node */
	enum wlf_backend_type type;           /**< Backend type */
	const char *name;                     /**< Backend name */
	int priority;                         /**< Backend priority (higher = preferred) */

	/**
	 * @brief Backend factory function
	 * @param args Backend-specific arguments (can be NULL)
	 * @return Pointer to created backend, or NULL on failure
	 */
	struct wlf_backend *(*create)(void *args);

	/**
	 * @brief Check if backend is available on current system
	 * @return true if backend is available, false otherwise
	 */
	bool (*is_available)(void);

	void *handle;  /**< Dynamic library handle (for plugins) */
};

/**
 * @brief Backend creation arguments for different backend types
 */
struct wlf_backend_create_args {
	enum wlf_backend_type type;

	union {
		struct {
			struct wlf_wl_display *display;  /**< Wayland display (optional) */
		} wayland;
	};
};

/**
 * @brief Initialize the backend subsystem
 * This should be called once at program startup
 */
void wlf_backend_init(void);

/**
 * @brief Cleanup the backend subsystem
 * This should be called once at program shutdown
 */
void wlf_backend_finish(void);

/**
 * @brief Register a backend implementation
 * @param entry Backend registry entry to register
 * @return true on success, false on failure
 */
bool wlf_backend_register(struct wlf_backend_registry_entry *entry);

/**
 * @brief Unregister a backend implementation
 * @param type Backend type to unregister
 */
void wlf_backend_unregister(enum wlf_backend_type type);

/**
 * @brief Load a backend plugin from shared library
 * @param plugin_path Path to the plugin shared library
 * @return true on success, false on failure
 */
bool wlf_backend_load_plugin(const char *plugin_path);

/**
 * @brief Unload a backend plugin
 * @param plugin_path Path to the plugin that was loaded
 */
void wlf_backend_unload_plugin(const char *plugin_path);

/**
 * @brief Auto-create the best available backend for the current environment
 * @return Pointer to created backend, or NULL on failure
 */
struct wlf_backend *wlf_backend_autocreate(void);

/**
 * @brief Create a backend of specific type
 * @param args Backend creation arguments
 * @return Pointer to created backend, or NULL on failure
 */
struct wlf_backend *wlf_backend_create(const struct wlf_backend_create_args *args);

/**
 * @brief Start a backend
 * @param backend Pointer to the backend
 * @return true on success, false on failure
 */
bool wlf_backend_start(struct wlf_backend *backend);

/**
 * @brief Stop a backend
 * @param backend Pointer to the backend
 */
void wlf_backend_stop(struct wlf_backend *backend);

/**
 * @brief Destroy a backend and free all resources
 * @param backend Pointer to the backend
 */
void wlf_backend_destroy(struct wlf_backend *backend);

/**
 * @brief Get backend type
 * @param backend Pointer to the backend
 * @return Backend type enumeration value
 */
enum wlf_backend_type wlf_backend_get_type(struct wlf_backend *backend);

/**
 * @brief Check if backend is active (started)
 * @param backend Pointer to the backend
 * @return true if backend is active, false otherwise
 */
bool wlf_backend_is_active(struct wlf_backend *backend);

/**
 * @brief Get the name string for a backend type
 * @param type Backend type
 * @return Backend name string
 */
const char *wlf_backend_type_name(enum wlf_backend_type type);

#endif // PLATFORM_WLF_BACKEND_H
