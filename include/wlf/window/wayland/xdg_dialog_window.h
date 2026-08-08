/**
 * @file        xdg_dialog_window.h
 * @brief       Wayland xdg_dialog window implementation for wlframe.
 * @details     This file exposes the wlframe window object backed by an
 *              xdg_toplevel role and extended with the xdg_dialog_v1 protocol.
 *              It is used as the Wayland implementation for dialog windows,
 *              including modal dialogs when supported by the compositor.
 * @author      YaoBing Xiao
 * @date        2026-06-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-20, initial version\n
 */

#ifndef WAYLAND_XDG_DIALOG_WINDOW_H
#define WAYLAND_XDG_DIALOG_WINDOW_H

#include "wlf/window/wlf_window.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_backend;
struct wlf_wl_surface;
struct wlf_xdg_dialog_v1;
struct wlf_xdg_surface;
struct wlf_xdg_toplevel;
struct wlf_xdg_wm_base;
struct wlf_xdg_wm_dialog_v1;

/**
 * @brief Wayland xdg_dialog window object.
 *
 * A dialog window is represented as an xdg_toplevel plus an xdg_dialog_v1 role
 * object. The structure is fully exposed so callers can access the wrapped
 * Wayland protocol objects directly when necessary.
 */
struct wlf_xdg_dialog_window {
	struct wlf_window base;                         /**< Generic wlframe window base */
	struct wlf_backend *backend;                    /**< Backend that owns this window */
	struct wlf_wl_surface *surface;                 /**< Wayland wl_surface wrapper */
	struct wlf_xdg_wm_base *wm_base;                /**< xdg_wm_base wrapper */
	struct wlf_xdg_surface *xdg_surface;            /**< xdg_surface role wrapper */
	struct wlf_xdg_toplevel *xdg_toplevel;          /**< xdg_toplevel role wrapper */
	struct wlf_xdg_wm_dialog_v1 *dialog_manager;    /**< xdg_wm_dialog_v1 manager wrapper */
	struct wlf_xdg_dialog_v1 *xdg_dialog;           /**< xdg_dialog_v1 role wrapper */

	struct wlf_listener xdg_surface_configure;      /**< xdg_surface configure listener */
	struct wlf_listener xdg_toplevel_configure;     /**< xdg_toplevel configure listener */
	struct wlf_listener xdg_toplevel_close;         /**< xdg_toplevel close listener */
	bool has_xdg_surface_configure_listener;        /**< Whether xdg_surface listener is registered */
	bool has_xdg_toplevel_configure_listener;       /**< Whether configure listener is registered */
	bool has_xdg_toplevel_close_listener;           /**< Whether close listener is registered */
};

/**
 * @brief Creates a Wayland xdg_dialog window from a backend.
 *
 * @param backend Wayland backend used to create protocol objects.
 * @param parent Parent xdg_toplevel or xdg_dialog window, or NULL.
 * @param width Initial dialog width.
 * @param height Initial dialog height.
 * @param modal Whether the dialog should be created as modal.
 * @return Generic wlf_window pointer or NULL on failure.
 */
struct wlf_window *wlf_xdg_dialog_window_create_from_backend(
	struct wlf_backend *backend, struct wlf_window *parent,
	uint32_t width, uint32_t height, bool modal);

/**
 * @brief Checks whether a generic window is an xdg_dialog window.
 *
 * @param window Window to check.
 * @return true if the window is backed by wlf_xdg_dialog_window.
 * @return false otherwise.
 */
bool wlf_window_is_xdg_dialog(const struct wlf_window *window);

/**
 * @brief Gets the xdg_dialog window from a generic window.
 *
 * @param window Generic window to convert.
 * @return xdg_dialog window object or NULL if the window is not an xdg_dialog.
 */
struct wlf_xdg_dialog_window *wlf_xdg_dialog_window_from_window(
	struct wlf_window *window);

#endif /* WAYLAND_XDG_DIALOG_WINDOW_H */
