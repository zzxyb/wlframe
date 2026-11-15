/**
 * @file        window.h
 * @brief       XCB toplevel windows for wlframe.
 * @details     This file provides XCB window creation and access through the
 *              generic window interface, plus internal event dispatch.
 * @author      YaoBing Xiao
 * @date        2026-11-10
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-11-10, initial version\n
 */

#ifndef WINDOW_XCB_WINDOW_H
#define WINDOW_XCB_WINDOW_H

#include "wlf/window/wlf_window.h"
#include "wlf/platform/xcb/backend.h"

/**
 * @brief Toplevel window backed by an XCB window.
 *
 * @note The backend is borrowed. Destroy the window before its renderer
 * and backend; the generic window owns its scene and swapchain.
 */
struct wlf_xcb_window {
	struct wlf_window base; /**< Generic window interface and state. */
	struct wlf_xcb_backend *backend; /**< Borrowed owner of the XCB connection. */
	xcb_window_t window; /**< Owned XCB window ID, or XCB_WINDOW_NONE. */
	xcb_colormap_t colormap; /**< Owned colormap, or XCB_COLORMAP_NONE. */
	uint8_t depth; /**< Native window depth used for image uploads. */
	bool frame_pending; /**< Whether a software-clock frame is requested. */
	struct wlf_linked_list link; /**< Link in the backend windows list. */
};

/**
 * @brief Creates an initially hidden XCB toplevel window.
 *
 * @param backend XCB backend that owns the connection.
 * @param width Initial width in pixels, from 1 to INT16_MAX.
 * @param height Initial height in pixels, from 1 to INT16_MAX.
 * @return New window, or NULL for invalid arguments or creation failure.
 * @note Destroy the returned base with wlf_window_destroy().
 */
struct wlf_xcb_window *wlf_xcb_window_create_from_backend(
	struct wlf_backend *backend, uint32_t width, uint32_t height);
/**
 * @brief Checks whether a window uses the XCB implementation.
 *
 * @param window Window to inspect; may be NULL.
 * @return true for an XCB window, false otherwise.
 */
bool wlf_window_is_xcb(const struct wlf_window *window);
/**
 * @brief Gets the XCB window containing a generic window.
 *
 * @param window Non-NULL window known to use the XCB implementation.
 * @return Enclosing XCB window, borrowed.
 * @note Asserts that window is an XCB window.
 */
struct wlf_xcb_window *wlf_xcb_window_from_window(struct wlf_window *window);
/**
 * @brief Dispatches an XCB event to its managed window.
 *
 * @param backend XCB backend whose connection produced the event.
 * @param event Borrowed event; the caller retains ownership.
 * @note Internal event-loop entry point. Unknown windows are ignored.
 */
WLF_PRIVATE void wlf_xcb_window_handle_event(struct wlf_xcb_backend *backend,
	xcb_generic_event_t *event);

#endif // WINDOW_XCB_WINDOW_H
