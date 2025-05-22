/**
 * @file        wlf_wl_compositor.h
 * @brief       Wayland compositor utility for wlframe.
 * @details     This file provides structures and functions for managing the Wayland compositor,
 *              including creation, destruction, surface and region creation, and event signaling.
 *              It also provides listeners for global add/remove events.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef WAYLAND_WLF_WL_COMPOSITOR_H
#define WAYLAND_WLF_WL_COMPOSITOR_H

#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_wl_display;
struct wl_compositor;
struct wl_surface;
struct wl_region;

/**
 * @brief Structure representing a Wayland compositor and its events.
 */
struct wlf_wl_compositor {
	struct wlf_wl_display *display;      /**< Associated Wayland display */
	struct wl_compositor *compositor;    /**< Wayland compositor pointer */

	struct {
		struct wlf_signal destroy;       /**< Signal emitted when compositor is destroyed */
	} events;

	struct wlf_listener global_add;      /**< Listener for global add event */
	struct wlf_listener global_remove;   /**< Listener for global remove event */
};

/**
 * @brief Create a new wlf_wl_compositor object.
 * @param display Pointer to the associated wlf_wl_display.
 * @return Pointer to the newly allocated wlf_wl_compositor.
 */
struct wlf_wl_compositor *wlf_wl_compositor_create(struct wlf_wl_display *display);

/**
 * @brief Check if the compositor is nil (invalid).
 * @param compositor Pointer to the wlf_wl_compositor.
 * @return true if compositor is nil, false otherwise.
 */
bool wlf_wl_compositor_is_nil(struct wlf_wl_compositor *compositor);

/**
 * @brief Destroy a wlf_wl_compositor object and free its resources.
 * @param compositor Pointer to the wlf_wl_compositor.
 */
void wlf_wl_compositor_destroy(struct wlf_wl_compositor *compositor);

/**
 * @brief Create a new Wayland surface from the compositor.
 * @param compositor Pointer to the wlf_wl_compositor.
 * @return Pointer to the newly created wl_surface.
 */
struct wl_surface *wlf_wl_compositor_create_surface(struct wlf_wl_compositor *compositor);

/**
 * @brief Create a new Wayland region from the compositor.
 * @param compositor Pointer to the wlf_wl_compositor.
 * @return Pointer to the newly created wl_region.
 */
struct wl_region *wlf_wl_compositor_create_region(struct wlf_wl_compositor *compositor);

#endif // WAYLAND_WLF_WL_COMPOSITOR_H
