/**
 * @file        allocator.h
 * @brief       Wayland EGL window-surface allocator.
 * @details     Declares the allocator that creates EGL-backed wlframe
 *              buffers for a native Wayland surface.
 * @author      YaoBing Xiao
 * @date        2026-08-17
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-17, initial version\n
 */

#ifndef ALLOCATOR_WL_EGL_ALLOCATOR_H
#define ALLOCATOR_WL_EGL_ALLOCATOR_H

#include "wlf/allocator/wlf_allocator.h"

#include <stdbool.h>

struct wl_surface;
struct wlf_egl;

/**
 * @brief Allocator for buffers backed by Wayland EGL window surfaces.
 *
 * The EGL context and native Wayland surface are borrowed. Buffers
 * created by this allocator own their individual EGL surface resources.
 */
struct wlf_wl_egl_allocator {
	struct wlf_allocator base; /**< Generic allocator interface. */
	struct wlf_egl *egl; /**< EGL context owner, borrowed. */
	struct wl_surface *surface; /**< Native Wayland surface, borrowed. */
};

/**
 * @brief Creates a Wayland EGL window-surface allocator.
 * @param egl EGL context used for surface creation.
 * @param surface Native Wayland surface receiving the EGL window.
 * @return Newly allocated Wayland EGL allocator, or NULL on failure.
 */
struct wlf_wl_egl_allocator *wlf_wl_egl_allocator_create(struct wlf_egl *egl,
	struct wl_surface *surface);

/**
 * @brief Checks whether an allocator uses the Wayland EGL implementation.
 * @param allocator Allocator to inspect.
 * @return true when @p allocator is a Wayland EGL allocator, false otherwise.
 */
bool wlf_allocator_is_wl_egl(const struct wlf_allocator *allocator);

/**
 * @brief Casts a generic allocator to a Wayland EGL allocator.
 * @param allocator Allocator known to use the Wayland EGL implementation.
 * @return Enclosing Wayland EGL allocator.
 * @note Asserts that the allocator uses this implementation.
 */
struct wlf_wl_egl_allocator *wlf_wl_egl_allocator_from_allocator(
	struct wlf_allocator *allocator);

#endif // ALLOCATOR_WL_EGL_ALLOCATOR_H
