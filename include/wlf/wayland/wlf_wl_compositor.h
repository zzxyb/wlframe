/**
 * @file        wlf_wl_compositor.h
 * @brief       Wayland compositor utility for wlframe.
 * @details     This file provides structures and functions for managing the Wayland compositor,
 *              including creation, destruction, surface and region creation, and event signaling.
 *              The compositor can be directly bound from a Wayland registry without requiring
 *              a full wlf_wl_display context, making it more lightweight and flexible.
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
#include <stdint.h>

struct wl_compositor;
struct wl_surface;
struct wl_region;
struct wl_registry;

/**
 * @brief Structure representing a Wayland compositor and its events.
 *
 * This structure wraps a Wayland compositor interface and provides
 * event signaling capabilities. It is designed to be lightweight
 * and can be created directly from a Wayland registry.
 */
struct wlf_wl_compositor {
	struct wl_compositor *base;    /**< Wayland compositor pointer */

	struct {
		struct wlf_signal destroy;       /**< Signal emitted when compositor is destroyed */
	} events;
};

/**
 * @brief Create a new wlf_wl_compositor object directly from Wayland registry.
 *
 * This function binds directly to the Wayland compositor interface using the
 * provided registry, name, and version. This approach is more lightweight
 * than the previous implementation that required a full wlf_wl_display context.
 *
 * @param wl_registry Wayland registry to bind from
 * @param name Global name (ID) of the compositor interface in the registry
 * @param version Version of the compositor interface to bind to
 * @return Pointer to the newly allocated wlf_wl_compositor, or NULL on failure
 */
struct wlf_wl_compositor *wlf_wl_compositor_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy a wlf_wl_compositor object and free its resources.
 *
 * This function properly cleans up the compositor by emitting the destroy signal,
 * destroying the underlying Wayland compositor interface, and freeing allocated memory.
 * It is safe to call this function with a NULL pointer.
 *
 * @param compositor Pointer to the wlf_wl_compositor to destroy
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
