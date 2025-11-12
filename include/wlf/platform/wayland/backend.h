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
#include "wlf/wayland/wlf_wl_display.h"

#include <stdbool.h>

/**
 * @brief Wayland backend specific data
 */
struct wlf_backend_wayland {
	struct wlf_backend base;              /**< Base backend structure */
	struct wlf_wl_display *display;       /**< Wayland display connection */
	struct wlf_wl_compositor *compositor;  /**< Wayland compositor interface */

	struct {
		struct wlf_listener display_destroy;  /**< Display destroy listener */
	} listeners;

	bool started;                         /**< Whether backend is started */
};

/**
 * @brief Register the Wayland backend
 * @return true on success, false on failure
 */
bool wlf_backend_wayland_register(void);

/**
 * @brief Check if a backend is a Wayland backend
 * @param backend Pointer to the backend to check
 * @return true if the backend is a Wayland backend, false otherwise
 */
bool wlf_backend_is_wayland(struct wlf_backend *backend);

/**
 * @brief Cast a generic backend to a Wayland backend
 * @param backend Pointer to the generic backend
 * @return Pointer to the Wayland backend, or NULL if the backend is not a Wayland backend
 */
struct wlf_backend_wayland *wlf_backend_wayland_from_backend(struct wlf_backend *backend);

#endif // PLATFORM_WLF_BACKEND_WAYLAND_H
