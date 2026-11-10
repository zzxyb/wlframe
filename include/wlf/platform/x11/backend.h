/**
 * @file        backend.h
 * @brief       XCB backend for native X11 applications.
 * @details     This file declares the X11 backend connection, window tracking,
 *              window-manager atoms, and backend creation helpers.
 * @author      YaoBing Xiao
 * @date        2026-11-10
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-11-10, initial version\n
 */

#ifndef PLATFORM_X11_BACKEND_H
#define PLATFORM_X11_BACKEND_H

#include "wlf/platform/wlf_backend.h"
#include <xcb/xcb.h>

/**
 * @brief X11 backend and its owned XCB connection.
 *
 * @note Windows must be destroyed before the backend. The screen is borrowed
 * from the connection and remains valid until backend destruction.
 */
struct wlf_x11_backend {
	struct wlf_backend base; /**< Generic backend interface. */
	xcb_connection_t *xcb; /**< Owned connection to the X server. */
	xcb_screen_t *screen; /**< Borrowed screen selected during connection. */
	int screen_number; /**< Screen index used by the XCB EGL platform. */
	struct wlf_linked_list windows; /**< Managed wlf_x11_window list linked by link. */
	/** Atoms used for window titles, state, decorations, and close requests. */
	struct {
		xcb_atom_t wm_protocols, wm_delete_window; /**< ICCCM protocol and close-request atoms. */
		xcb_atom_t utf8_string, net_wm_name; /**< UTF-8 title property type and name. */
		xcb_atom_t net_wm_state, net_wm_state_fullscreen; /**< EWMH state property and fullscreen atom. */
		xcb_atom_t net_wm_state_maximized_vert, net_wm_state_maximized_horz; /**< EWMH vertical and horizontal maximize atoms. */
		xcb_atom_t motif_wm_hints; /**< Motif decoration hints property. */
	} atoms; /**< Window-manager atoms interned on this connection. */
};

/**
 * @brief Connects to DISPLAY and initializes the X11 backend.
 *
 * @return New backend, or NULL on connection or initialization failure.
 * @note Destroy the returned base with wlf_backend_destroy().
 */
struct wlf_x11_backend *wlf_x11_backend_create(void);
/**
 * @brief Checks whether a backend uses the X11 implementation.
 *
 * @param backend Backend to inspect; may be NULL.
 * @return true for an X11 backend, false otherwise.
 */
bool wlf_backend_is_x11(const struct wlf_backend *backend);
/**
 * @brief Gets the X11 backend containing a generic backend.
 *
 * @param backend Non-NULL backend known to use the X11 implementation.
 * @return Enclosing X11 backend, borrowed.
 * @note Asserts that backend is an X11 backend.
 */
struct wlf_x11_backend *wlf_x11_backend_from_backend(struct wlf_backend *backend);

#endif // PLATFORM_X11_BACKEND_H
