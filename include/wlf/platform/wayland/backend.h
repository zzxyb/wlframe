/**
 * @file        backend.h
 * @brief       Wayland client backend implementation
 * @details     This backend allows wlframe to run as a Wayland client,
 *              creating windows on an existing Wayland compositor.
 * @author      YaoBing Xiao
 * @date        2025-06-25
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-25, initial version\n
 */

#ifndef PLATFORM_WLF_BACKEND_WAYLAND_H
#define PLATFORM_WLF_BACKEND_WAYLAND_H

#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

#include <wayland-client-core.h>

/**
 * @brief Wayland backend specific data
 */
struct wlf_backend_wayland {
	struct wlf_backend base;              /**< Base backend structure */

	struct wl_display *display;         /**< Wayland display pointer */
	struct wl_registry *registry;       /**< Wayland registry pointer */
	struct wl_compositor *compositor;  /**< Wayland compositor interface */
	struct wl_shm *shm;

	struct wlf_linked_list interfaces;  /**< List of global interfaces */

	struct {
		struct wlf_signal destroy;      /**< Signal emitted when display is destroyed */
		struct wlf_signal global_add;   /**< Signal emitted when a global is added */
		struct wlf_signal global_remove;/**< Signal emitted when a global is removed */
	} events;

	struct {
		struct wlf_listener compositor_destroy; /**< wl_compositor destroy listener */
		struct wlf_listener output_manager_destroy; /**< zxdg_output_manager_v1 destroy listener */
	} listeners;
};

struct wlf_backend *wayland_backend_create(void);

/**
 * @brief Check if a backend is a Wayland backend
 * @param backend Pointer to the backend to check
 * @return true if the backend is a Wayland backend, false otherwise
 */
bool wlf_backend_is_wayland(const struct wlf_backend *backend);

/**
 * @brief Cast a generic backend to a Wayland backend
 * @param backend Pointer to the generic backend
 * @return Pointer to the Wayland backend, or NULL if the backend is not a Wayland backend
 */
struct wlf_backend_wayland *wlf_backend_wayland_from_backend(struct wlf_backend *backend);

#endif // PLATFORM_WLF_BACKEND_WAYLAND_H
