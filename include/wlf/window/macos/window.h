/**
 * @file        window.h
 * @brief       macOS native window implementation for wlframe.
 * @details     This file defines the macOS-specific window structure and creation
 *              function. The implementation wraps NSWindow and NSView via AppKit,
 *              bridging native window events into the wlframe signal system.
 * @author      YaoBing Xiao
 * @date        2026-03-17
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-17, initial version\n
 */

#ifndef MACOS_WINDOW_H
#define MACOS_WINDOW_H

#include "wlf/window/wlf_window.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief macOS-specific window object.
 *
 * Extends @ref wlf_window with native AppKit handles. The NSWindow and
 * NSView objects are bridged through opaque void pointers so that this
 * header can be included from plain-C translation units.
 */
struct wlf_macos_window {
	struct wlf_window base;    /**< Base window object (must be first). */

	void *ns_window;           /**< Native NSWindow handle. */
	void *ns_view;             /**< Native NSView (content view) handle. */
	void *ns_delegate;         /**< Native NSWindowDelegate handle. */
};

/**
 * @brief Creates a new macOS window.
 *
 * Allocates a @ref wlf_macos_window, creates the underlying NSWindow and
 * installs a delegate that forwards native events into the wlframe signal
 * system.
 *
 * @param type   Window type controlling style mask selection.
 * @param width  Initial content width in points.
 * @param height Initial content height in points.
 * @return Pointer to the base @ref wlf_window on success, or NULL on failure.
 */
struct wlf_window *wlf_macos_window_create(enum wlf_window_type type,
	uint32_t width, uint32_t height);

/**
 * @brief Checks whether a window is a macOS window.
 *
 * @param window Generic window pointer.
 * @return true if the window was created by the macOS backend.
 */
bool wlf_window_is_macos(const struct wlf_window *window);

/**
 * @brief Downcasts a generic window to a macOS window.
 *
 * The caller must ensure the window is actually a @ref wlf_macos_window
 * (e.g. by checking wlf_window_is_macos() first).
 *
 * @param window Generic window pointer.
 * @return Pointer to the underlying wlf_macos_window, or NULL if mismatch.
 */
struct wlf_macos_window *wlf_macos_window_from_window(struct wlf_window *window);

#endif // MACOS_WINDOW_H
