/**
 * @file        cursor.h
 * @brief       Xresource-aware cursor implementation for the X11 backend.
 * @details     Uses libXcursor for theme files and XCB Render for native cursors.
 *              The implementation owns its shape cache and reload listener.
 * @author      YaoBing Xiao
 * @date        2026-11-13
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-11-13, initial version\n
 */
#ifndef PLATFORM_X11_CURSOR_H
#define PLATFORM_X11_CURSOR_H

#include "wlf/types/wlf_cursor.h"
#include <xcb/xcb.h>

struct wlf_x11_backend;
struct wlf_x11_cursor;

/**
 * @brief Creates a themed cursor and subscribes to Xcursor resource changes.
 * @param backend Borrowed X11 backend with an initialized resource manager.
 * @return New cursor, or NULL on allocation or core cursor font failure.
 * @note Destroy through wlf_cursor_destroy() before the resource manager and
 * connection. Missing themes or Render support fall back to core cursor glyphs.
 */
struct wlf_cursor *wlf_x11_cursor_create(struct wlf_x11_backend *backend);

/**
 * @brief Sets the window targeted by generic cursor shape requests.
 * @param cursor X11 cursor returned by wlf_x11_cursor_create().
 * @param window Native window ID, or XCB_WINDOW_NONE when no window is focused.
 * @note Internal backend hook; the window remains borrowed.
 */
WLF_PRIVATE void wlf_x11_cursor_set_window(struct wlf_cursor *cursor,
	xcb_window_t window);

/**
 * @brief Forgets a window when it is hidden, destroyed, or left by the pointer.
 * @param cursor X11 cursor instance.
 * @param window Window to clear only if it is the current target.
 * @note Internal backend hook; other windows keep their cursor target.
 */
WLF_PRIVATE void wlf_x11_cursor_forget_window(struct wlf_cursor *cursor,
	xcb_window_t window);

#endif // PLATFORM_X11_CURSOR_H
