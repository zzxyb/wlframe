/**
 * @file        wlr_layer_window.h
 * @brief       Wayland wlr-layer-shell window implementation for wlframe.
 * @details     This file provides a window implementation backed by the
 *              wlr-layer-shell protocol. Layer windows can be placed on the
 *              background, bottom, top, or overlay layer and can be used as
 *              parents of xdg_popup windows.
 *
 *              Typical usage:
 *                  - Create a layer window with
 *                    wlf_wlr_layer_window_create_from_backend()
 *                  - Configure its layer-surface properties through
 *                    wlf_wlr_layer_window::layer_surface
 *                  - Show the window with wlf_window_show()
 *                  - Destroy it with wlf_window_destroy()
 * @author      YaoBing Xiao
 * @date        2026-06-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-22, initial version\n
 */

#ifndef WAYLAND_WLR_LAYER_WINDOW_H
#define WAYLAND_WLR_LAYER_WINDOW_H

#include "wlf/wayland/wlf_zwlr_layer_shell_v1.h"
#include "wlf/window/wlf_window.h"

#include <stdbool.h>
#include <stdint.h>

struct wl_output;
struct wlf_backend;
struct wlf_wl_surface;

/**
 * @brief Wayland window backed by a zwlr_layer_surface_v1 role.
 *
 * This structure combines the generic wlf_window interface with the Wayland
 * wl_surface and wlr-layer-shell protocol objects required by a layer window.
 * Protocol-specific properties can be changed through @ref layer_surface.
 * Double-buffered layer-surface state takes effect on the next wl_surface
 * commit.
 */
struct wlf_wlr_layer_window {
	struct wlf_window base;                         /**< Generic window base. */
	struct wlf_backend *backend;                    /**< Owning backend. */
	struct wlf_wl_surface *surface;                 /**< wl_surface wrapper. */
	struct wlf_zwlr_layer_shell_v1 *layer_shell;    /**< Layer-shell wrapper. */
	struct wlf_zwlr_layer_surface_v1 *layer_surface; /**< Layer-surface role. */

	/** Listener for zwlr_layer_surface_v1 configure events. */
	struct wlf_listener layer_surface_configure;
	/** Listener for zwlr_layer_surface_v1 closed events. */
	struct wlf_listener layer_surface_closed;
	/** Whether the configure listener is currently registered. */
	bool has_layer_surface_configure_listener;
	/** Whether the closed listener is currently registered. */
	bool has_layer_surface_closed_listener;
};

/**
 * @brief Creates a wlr-layer-shell window from a Wayland backend.
 *
 * The returned window owns its wl_surface, layer-shell wrapper, and
 * layer-surface role. The caller must destroy the window with
 * wlf_window_destroy(). The initial role commit is performed when the caller
 * shows the window, which allows layer-surface properties such as anchors and
 * margins to be configured before the initial commit.
 *
 * @param backend Wayland backend used to create protocol objects.
 * @param output Target output, or NULL for compositor selection.
 * @param layer Initial layer.
 * @param namespace Namespace identifying the surface purpose; must not be NULL.
 * @param width Requested width, or zero for compositor-selected width.
 * @param height Requested height, or zero for compositor-selected height.
 * @return A generic wlf_window pointer, or NULL on failure.
 */
struct wlf_wlr_layer_window *wlf_wlr_layer_window_create_from_backend(
	struct wlf_backend *backend, struct wl_output *output,
	enum wlf_zwlr_layer_v1 layer, const char *namespace,
	uint32_t width, uint32_t height);

/**
 * @brief Checks whether a generic window is a wlr-layer-shell window.
 *
 * @param window Window to check.
 * @return true if the window is backed by wlf_wlr_layer_window.
 * @return false otherwise.
 */
bool wlf_window_is_wlr_layer(const struct wlf_window *window);

/**
 * @brief Gets the wlr-layer-shell window from a generic window.
 *
 * @param window Generic window to convert.
 * @return Layer window object, or NULL if the window has another
 *         implementation.
 */
struct wlf_wlr_layer_window *wlf_wlr_layer_window_from_window(
	struct wlf_window *window);

#endif /* WAYLAND_WLR_LAYER_WINDOW_H */
