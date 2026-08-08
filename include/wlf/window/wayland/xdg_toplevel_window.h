/**
 * @file        xdg_toplevel_window.h
 * @brief       Wayland xdg_toplevel window implementation for wlframe.
 * @details     This file exposes the wlframe window object backed by the
 *              Wayland xdg_toplevel role. It combines the generic wlf_window
 *              abstraction with the Wayland wl_surface, xdg_wm_base,
 *              xdg_surface, and xdg_toplevel wrapper objects.
 * @author      YaoBing Xiao
 * @date        2026-06-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-20, initial version\n
 */

#ifndef WAYLAND_XDG_TOPLEVEL_WINDOW_H
#define WAYLAND_XDG_TOPLEVEL_WINDOW_H

#include "wlf/window/wlf_window.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_backend;
struct wlf_wl_surface;
struct wlf_xdg_surface;
struct wlf_xdg_toplevel;
struct wlf_xdg_wm_base;

/**
 * @brief Wayland xdg_toplevel window object.
 *
 * This structure is fully exposed so platform-specific users can access the
 * underlying Wayland wrapper objects directly when they need protocol-level
 * control beyond the generic wlf_window API.
 */
struct wlf_xdg_toplevel_window {
	struct wlf_window base;                  /**< Generic wlframe window base */
	struct wlf_backend *backend;             /**< Backend that owns this window */
	struct wlf_wl_surface *surface;          /**< Wayland wl_surface wrapper */
	struct wlf_xdg_wm_base *wm_base;         /**< xdg_wm_base wrapper */
	struct wlf_xdg_surface *xdg_surface;     /**< xdg_surface role wrapper */
	struct wlf_xdg_toplevel *xdg_toplevel;   /**< xdg_toplevel role wrapper */

	struct wlf_listener xdg_surface_configure;   /**< xdg_surface configure listener */
	struct wlf_listener xdg_toplevel_configure;  /**< xdg_toplevel configure listener */
	struct wlf_listener xdg_toplevel_close;      /**< xdg_toplevel close listener */
	bool has_xdg_surface_configure_listener;     /**< Whether xdg_surface listener is registered */
	bool has_xdg_toplevel_configure_listener;    /**< Whether configure listener is registered */
	bool has_xdg_toplevel_close_listener;        /**< Whether close listener is registered */
};

/**
 * @brief Creates a Wayland xdg_toplevel window from a backend.
 *
 * @param backend Wayland backend used to create protocol objects.
 * @param width Initial window width.
 * @param height Initial window height.
 * @return Generic wlf_window pointer or NULL on failure.
 */
struct wlf_window *wlf_xdg_toplevel_window_create_from_backend(
	struct wlf_backend *backend, uint32_t width, uint32_t height);

/**
 * @brief Checks whether a generic window is an xdg_toplevel window.
 *
 * @param window Window to check.
 * @return true if the window is backed by wlf_xdg_toplevel_window.
 * @return false otherwise.
 */
bool wlf_window_is_xdg_toplevel(const struct wlf_window *window);

/**
 * @brief Gets the xdg_toplevel window from a generic window.
 *
 * @param window Generic window to convert.
 * @return xdg_toplevel window object or NULL if the window is not an xdg_toplevel.
 */
struct wlf_xdg_toplevel_window *wlf_xdg_toplevel_window_from_window(
	struct wlf_window *window);

#endif /* WAYLAND_XDG_TOPLEVEL_WINDOW_H */
