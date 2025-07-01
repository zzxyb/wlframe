/**
 * @file        wlf_backend_wayland.h
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

#endif // PLATFORM_WLF_BACKEND_WAYLAND_H
