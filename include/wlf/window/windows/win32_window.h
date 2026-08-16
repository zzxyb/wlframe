/**
 * @file win32_window.h
 * @brief Native Win32 toplevel window implementation.
 */

#ifndef WLF_WINDOW_WINDOWS_WIN32_WINDOW_H
#define WLF_WINDOW_WINDOWS_WIN32_WINDOW_H

#include "wlf/window/wlf_window.h"
#include "wlf/types/wlf_cursor.h"

#include <stdbool.h>
#include <stdint.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct wlf_win32_window {
	struct wlf_window base;
	struct wlf_pointer pointer;
	struct wlf_keyboard keyboard;
	struct wlf_touch touch;
	struct wlf_cursor cursor;
	HWND hwnd;
	HCURSOR cursor_handle;
	WINDOWPLACEMENT restore_placement;
	DWORD restore_style;
	uint32_t input_serial;
	wchar_t pending_high_surrogate;
	bool fullscreen;
	bool counted;
	bool pointer_inside;
};

/** Creates a native Windows toplevel with the requested client size. */
struct wlf_window *wlf_win32_window_create_from_backend(
	struct wlf_backend *backend, uint32_t width, uint32_t height);

bool wlf_window_is_win32(const struct wlf_window *window);
struct wlf_win32_window *wlf_win32_window_from_window(
	struct wlf_window *window);

#endif /* WLF_WINDOW_WINDOWS_WIN32_WINDOW_H */
