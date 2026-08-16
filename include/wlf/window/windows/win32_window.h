/**
 * @file win32_window.h
 * @brief Native Win32 toplevel window implementation.
 */

#ifndef WLF_WINDOW_WINDOWS_WIN32_WINDOW_H
#define WLF_WINDOW_WINDOWS_WIN32_WINDOW_H

#include "wlf/window/wlf_window.h"

#include <stdbool.h>
#include <stdint.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct wlf_win32_window {
	struct wlf_window base;
	HWND hwnd;
	WINDOWPLACEMENT restore_placement;
	DWORD restore_style;
	bool fullscreen;
	bool counted;
};

/** Creates a native Windows toplevel with the requested client size. */
struct wlf_window *wlf_win32_window_create_from_backend(
	struct wlf_backend *backend, uint32_t width, uint32_t height);

bool wlf_window_is_win32(const struct wlf_window *window);
struct wlf_win32_window *wlf_win32_window_from_window(
	struct wlf_window *window);

#endif /* WLF_WINDOW_WINDOWS_WIN32_WINDOW_H */
