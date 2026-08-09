/**
 * @file        wlf_xdg_toplevel_icon_manager_v1.h
 * @brief       Wayland xdg_toplevel_icon_manager_v1 protocol wrapper for
 *              wlframe.
 * @details     Implements the staging xdg-toplevel-icon-v1 protocol, which
 *              lets clients set a pixmap or named icon on an xdg_toplevel.
 *              The compositor advertises its preferred icon sizes via
 *              icon_size events followed by a done event.
 *
 *              Usage:
 *                1. Bind wlf_xdg_toplevel_icon_manager_v1 from the registry.
 *                2. Listen to events.done to know when the compositor has
 *                   finished advertising preferred sizes (available in
 *                   manager->preferred_sizes[0..n_preferred_sizes-1]).
 *                3. Call wlf_xdg_toplevel_icon_manager_v1_create_icon() to
 *                   create an icon object, then configure it with
 *                   wlf_xdg_toplevel_icon_v1_set_name() and/or
 *                   wlf_xdg_toplevel_icon_v1_add_buffer().
 *                4. Call wlf_xdg_toplevel_icon_manager_v1_set_icon() to
 *                   attach the icon to a toplevel.
 *                5. Destroy the icon object after set_icon (it may be reused).
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_XDG_TOPLEVEL_ICON_MANAGER_V1_H
#define WAYLAND_WLF_XDG_TOPLEVEL_ICON_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stddef.h>
#include <stdint.h>

struct wl_buffer;
struct wl_registry;
struct xdg_toplevel;
struct xdg_toplevel_icon_manager_v1;
struct xdg_toplevel_icon_v1;

/**
 * @brief Wrapper around a bound xdg_toplevel_icon_manager_v1 global.
 *
 * The compositor emits icon_size events followed by a done event to
 * advertise its preferred icon sizes.  The sizes are accumulated in
 * @c preferred_sizes (heap-allocated) before the @c events.done signal
 * fires.  The caller must not free this array; it is owned by the manager.
 */
struct wlf_xdg_toplevel_icon_manager_v1 {
	struct xdg_toplevel_icon_manager_v1 *base;

	/** Compositor-preferred icon edge sizes, in surface-local coordinates. */
	int32_t *preferred_sizes;
	/** Number of entries in @c preferred_sizes. */
	size_t n_preferred_sizes;

	struct {
		/** Emitted after all icon_size events have been received.
		 *  Data: the manager itself. */
		struct wlf_signal done;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Per-toplevel icon object.
 *
 * Created by wlf_xdg_toplevel_icon_manager_v1_create_icon().  Configure
 * with set_name and/or add_buffer, then pass to set_icon.  The caller
 * owns and must destroy this object.
 */
struct wlf_xdg_toplevel_icon_v1 {
	struct xdg_toplevel_icon_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the xdg_toplevel_icon_manager_v1 global from the registry.
 */
struct wlf_xdg_toplevel_icon_manager_v1 *
wlf_xdg_toplevel_icon_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 */
void wlf_xdg_toplevel_icon_manager_v1_destroy(
	struct wlf_xdg_toplevel_icon_manager_v1 *manager);

/**
 * @brief Create a new icon object.
 *
 * @param manager  Bound manager.
 * @return A new wlf_xdg_toplevel_icon_v1, or NULL on failure.
 */
struct wlf_xdg_toplevel_icon_v1 *
wlf_xdg_toplevel_icon_manager_v1_create_icon(
	struct wlf_xdg_toplevel_icon_manager_v1 *manager);

/**
 * @brief Attach an icon to an xdg_toplevel.
 *
 * Pass @p icon as NULL to unset the icon.
 *
 * @param manager   Bound manager.
 * @param toplevel  The toplevel to assign the icon to.
 * @param icon      Icon to set, or NULL to unset.
 */
void wlf_xdg_toplevel_icon_manager_v1_set_icon(
	struct wlf_xdg_toplevel_icon_manager_v1 *manager,
	struct xdg_toplevel *toplevel,
	struct wlf_xdg_toplevel_icon_v1 *icon);

/**
 * @brief Set the icon name (for named/themed icons).
 *
 * @param icon       Icon object.
 * @param icon_name  Icon name string (XDG icon theme name).
 */
void wlf_xdg_toplevel_icon_v1_set_name(
	struct wlf_xdg_toplevel_icon_v1 *icon, const char *icon_name);

/**
 * @brief Add a pixel buffer to the icon at the given scale.
 *
 * The buffer must be backed by wl_shm.
 *
 * @param icon    Icon object.
 * @param buffer  Pixel buffer.
 * @param scale   Buffer scale factor (e.g. 1 for 1:1, 2 for HiDPI).
 */
void wlf_xdg_toplevel_icon_v1_add_buffer(
	struct wlf_xdg_toplevel_icon_v1 *icon,
	struct wl_buffer *buffer, int32_t scale);

/**
 * @brief Destroy the icon object and free its resources.
 */
void wlf_xdg_toplevel_icon_v1_destroy(struct wlf_xdg_toplevel_icon_v1 *icon);

#endif /* WAYLAND_WLF_XDG_TOPLEVEL_ICON_MANAGER_V1_H */
