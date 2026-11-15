/**
 * @file        backend.h
 * @brief       XCB backend for native X11 applications.
 * @details     This file declares the XCB backend connection, window tracking,
 *              window-manager atoms, and backend creation helpers.
 * @author      YaoBing Xiao
 * @date        2026-11-10
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-11-10, initial version\n
 */

#ifndef PLATFORM_XCB_BACKEND_H
#define PLATFORM_XCB_BACKEND_H

#include "wlf/platform/wlf_backend.h"
#include "wlf/types/wlf_pointer.h"
#include "wlf/types/wlf_keyboard.h"
#include <xcb/xcb.h>

/**
 * @brief Indices of window-manager atoms interned by the XCB backend.
 */
enum wlf_xcb_atom_name {
	WLF_XCB_ATOM_WM_PROTOCOLS, /**< Supported window-manager protocols. */
	WLF_XCB_ATOM_WM_DELETE_WINDOW, /**< Window close request protocol. */
	WLF_XCB_ATOM_UTF8_STRING, /**< UTF-8 string property type. */
	WLF_XCB_ATOM_NET_WM_NAME, /**< EWMH UTF-8 window title. */
	WLF_XCB_ATOM_NET_WM_STATE, /**< EWMH window state property. */
	WLF_XCB_ATOM_NET_WM_STATE_FULLSCREEN, /**< EWMH fullscreen state. */
	WLF_XCB_ATOM_NET_WM_STATE_MAXIMIZED_VERT, /**< EWMH vertical maximization state. */
	WLF_XCB_ATOM_NET_WM_STATE_MAXIMIZED_HORZ, /**< EWMH horizontal maximization state. */
	WLF_XCB_ATOM_MOTIF_WM_HINTS, /**< Motif window decoration hints. */
	WLF_XCB_ATOM_LAST, /**< Number of atoms; keep this entry last. */
};

/**
 * @brief XCB backend and its owned XCB connection.
 *
 * @note Windows must be destroyed before the backend. The screen is borrowed
 * from the connection and remains valid until backend destruction.
 */
struct wlf_xcb_backend {
	struct wlf_backend base; /**< Generic backend interface. */
	struct wlf_pointer pointer; /**< Embedded core pointer owned by the backend. */
	struct wlf_keyboard keyboard; /**< Embedded core keyboard owned by the backend. */
	xcb_connection_t *xcb; /**< Owned connection to the X server. */
	xcb_screen_t *screen; /**< Borrowed screen selected during connection. */
	int screen_number; /**< Screen index used by the XCB EGL platform. */
	struct wlf_linked_list windows; /**< Managed wlf_xcb_window list linked by link. */
	xcb_atom_t atoms[WLF_XCB_ATOM_LAST]; /**< Atoms indexed by enum wlf_xcb_atom_name. */
};

/**
 * @brief Connects to DISPLAY and initializes the XCB backend.
 *
 * @return New backend, or NULL on connection or initialization failure.
 * @note Destroy the returned base with wlf_backend_destroy().
 */
struct wlf_xcb_backend *wlf_xcb_backend_create(void);

/**
 * @brief Checks whether a backend uses the XCB implementation.
 *
 * @param backend Backend to inspect; may be NULL.
 * @return true for an XCB backend, false otherwise.
 */
bool wlf_backend_is_xcb(const struct wlf_backend *backend);

/**
 * @brief Gets the XCB backend containing a generic backend.
 *
 * @param backend Non-NULL backend known to use the XCB implementation.
 * @return Enclosing XCB backend, borrowed.
 * @note Asserts that backend is an XCB backend.
 */
struct wlf_xcb_backend *wlf_xcb_backend_from_backend(struct wlf_backend *backend);

#endif // PLATFORM_XCB_BACKEND_H
