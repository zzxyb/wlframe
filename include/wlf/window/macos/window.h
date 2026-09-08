/**
 * @file window.h
 * @brief Native macOS implementation of wlf_window.
 */

#ifndef WLF_WINDOW_MACOS_WINDOW_H
#define WLF_WINDOW_MACOS_WINDOW_H

#include "wlf/window/wlf_window.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_macos_window {
	struct wlf_window base;
	struct wlf_backend *backend;
	void *ns_window; /**< Retained NSWindow. */
	void *view; /**< Borrowed content NSView. */
	void *delegate; /**< Retained NSWindowDelegate. */
	struct wlf_macos_pointer *pointer;
	struct wlf_macos_keyboard *keyboard;
	bool frame_pending;
	bool pointer_inside;
};

struct wlf_macos_window *wlf_macos_window_create_from_backend(
	struct wlf_backend *backend, uint32_t width, uint32_t height);

bool wlf_window_is_macos(const struct wlf_window *window);

struct wlf_macos_window *wlf_macos_window_from_window(
	struct wlf_window *window);

void *wlf_macos_window_get_nswindow(const struct wlf_macos_window *window);

void *wlf_macos_window_get_view(const struct wlf_macos_window *window);

struct wlf_pointer *wlf_macos_window_get_pointer(
	const struct wlf_macos_window *window);

struct wlf_keyboard *wlf_macos_window_get_keyboard(
	const struct wlf_macos_window *window);

#endif /* WLF_WINDOW_MACOS_WINDOW_H */
