/**
 * @file        xdg_popup_window.h
 * @brief       Wayland xdg_popup window implementation for wlframe.
 * @details     This file exposes the wlframe window object backed by the
 *              Wayland xdg_popup role. It is used for transient popup-style
 *              surfaces such as menus and platform-specific popup windows.
 * @author      YaoBing Xiao
 * @date        2026-06-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-20, initial version\n
 */

#ifndef WAYLAND_XDG_POPUP_WINDOW_H
#define WAYLAND_XDG_POPUP_WINDOW_H

#include "wlf/window/wlf_window.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_backend;
struct wlf_wl_surface;
struct wlf_xdg_popup;
struct wlf_xdg_surface;
struct wlf_xdg_wm_base;

/**
 * @brief Wayland xdg_popup window object.
 *
 * This structure is fully exposed so callers can access the wrapped Wayland
 * xdg_popup state directly when generic wlf_window operations are not enough.
 */
struct wlf_xdg_popup_window {
	struct wlf_window base;              /**< Generic wlframe window base */
	struct wlf_backend *backend;         /**< Backend that owns this window */
	struct wlf_wl_surface *surface;      /**< Wayland wl_surface wrapper */
	struct wlf_xdg_wm_base *wm_base;     /**< xdg_wm_base wrapper */
	struct wlf_xdg_surface *xdg_surface; /**< xdg_surface role wrapper */
	struct wlf_xdg_popup *xdg_popup;     /**< xdg_popup role wrapper */

	struct wlf_listener xdg_surface_configure; /**< xdg_surface configure listener */
	struct wlf_listener xdg_popup_configure;   /**< xdg_popup configure listener */
	struct wlf_listener xdg_popup_done;        /**< xdg_popup done listener */
	bool has_xdg_surface_configure_listener;   /**< Whether xdg_surface listener is registered */
	bool has_xdg_popup_configure_listener;     /**< Whether configure listener is registered */
	bool has_xdg_popup_done_listener;          /**< Whether done listener is registered */
};

/**
 * @brief Creates a Wayland xdg_popup window from a backend.
 *
 * @param backend Wayland backend used to create protocol objects.
 * @param parent Parent xdg or wlr-layer-shell window, or NULL. A layer parent
 *               is associated through zwlr_layer_surface_v1.get_popup.
 * @param x Popup anchor x position relative to the parent.
 * @param y Popup anchor y position relative to the parent.
 * @param width Initial popup width.
 * @param height Initial popup height.
 * @return Generic wlf_window pointer or NULL on failure.
 */
struct wlf_xdg_popup_window *wlf_xdg_popup_window_create_from_backend(
	struct wlf_backend *backend, struct wlf_window *parent,
	int32_t x, int32_t y, uint32_t width, uint32_t height);

/**
 * @brief Checks whether a generic window is an xdg_popup window.
 *
 * @param window Window to check.
 * @return true if the window is backed by wlf_xdg_popup_window.
 * @return false otherwise.
 */
bool wlf_window_is_xdg_popup(const struct wlf_window *window);

/**
 * @brief Gets the xdg_popup window from a generic window.
 *
 * @param window Generic window to convert.
 * @return xdg_popup window object or NULL if the window is not an xdg_popup.
 */
struct wlf_xdg_popup_window *wlf_xdg_popup_window_from_window(
	struct wlf_window *window);

#endif /* WAYLAND_XDG_POPUP_WINDOW_H */
