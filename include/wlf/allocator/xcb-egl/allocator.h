/**
 * @file        allocator.h
 * @brief       XCB EGL window-surface allocator.
 * @details     Declares the allocator that creates EGL-backed wlframe
 *              buffers for a native XCB window.
 * @author      YaoBing Xiao
 * @date        2026-11-15
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-11-15, initial version\n
 */

#ifndef ALLOCATOR_XCB_EGL_ALLOCATOR_H
#define ALLOCATOR_XCB_EGL_ALLOCATOR_H

#include "wlf/allocator/wlf_allocator.h"

#include <xcb/xcb.h>

#include <stdbool.h>

struct wlf_egl;

/**
 * @brief Allocator for buffers backed by XCB EGL window surfaces.
 *
 * The EGL context and native XCB window are borrowed. Buffers
 * created by this allocator own their individual EGL surface resources.
 */
struct wlf_xcb_egl_allocator {
	struct wlf_allocator base; /**< Generic allocator interface. */
	struct wlf_egl *egl; /**< EGL context owner, borrowed. */
	xcb_window_t xcb_window; /**< Borrowed XCB window ID. */
};

/**
 * @brief Creates an EGL allocator for a native XCB window.
 *
 * @param egl Borrowed XCB EGL context used for surface creation.
 * @param window Nonzero borrowed XCB window ID.
 * @return New allocator, or NULL for invalid arguments or allocation failure.
 * @note The EGL context and native window must outlive the allocator
 * and all buffers it creates.
 */
struct wlf_xcb_egl_allocator *wlf_xcb_egl_allocator_create(struct wlf_egl *egl,
	xcb_window_t window);

/**
 * @brief Checks whether an allocator uses the XCB EGL implementation.
 * @param allocator Allocator to inspect.
 * @return true when @p allocator is an XCB EGL allocator, false otherwise.
 */
bool wlf_allocator_is_xcb_egl(const struct wlf_allocator *allocator);

/**
 * @brief Casts a generic allocator to an XCB EGL allocator.
 * @param allocator Allocator known to use the XCB EGL implementation.
 * @return Enclosing XCB EGL allocator.
 * @note Asserts that the allocator uses this implementation.
 */
struct wlf_xcb_egl_allocator *wlf_xcb_egl_allocator_from_allocator(
	struct wlf_allocator *allocator);

#endif // ALLOCATOR_XCB_EGL_ALLOCATOR_H
