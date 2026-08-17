/**
 * @file        allocator.h
 * @brief       EGL window-surface allocator.
 * @details     Declares the allocator that creates EGL-backed wlframe
 *              buffers for a native Wayland surface.
 * @author      YaoBing Xiao
 * @date        2026-08-17
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-17, initial version\n
 */

#ifndef ALLOCATOR_EGL_ALLOCATOR_H
#define ALLOCATOR_EGL_ALLOCATOR_H

#include "wlf/allocator/wlf_allocator.h"

#include <stdbool.h>

struct wl_surface;
struct wlf_egl;

/**
 * @brief Allocator for buffers backed by EGL window surfaces.
 *
 * The EGL context and native Wayland surface are borrowed. Buffers created by
 * this allocator own their individual EGL surface resources.
 */
struct wlf_egl_allocator {
	struct wlf_allocator base; /**< Generic allocator interface. */
	struct wlf_egl *egl; /**< EGL context owner, borrowed. */
	struct wl_surface *surface; /**< Native Wayland surface, borrowed. */
};

/**
 * @brief Creates an EGL window-surface allocator.
 * @param egl EGL context used for surface creation.
 * @param surface Native Wayland surface receiving the EGL window.
 * @return Newly allocated generic allocator, or NULL on failure.
 */
struct wlf_allocator *wlf_egl_allocator_create(struct wlf_egl *egl,
	struct wl_surface *surface);

/**
 * @brief Checks whether an allocator is EGL-backed.
 * @param allocator Allocator to inspect.
 * @return true when @p allocator is an EGL allocator, false otherwise.
 */
bool wlf_allocator_is_egl(const struct wlf_allocator *allocator);

/**
 * @brief Casts a generic allocator to an EGL allocator.
 * @param allocator Allocator known to be EGL-backed.
 * @return Enclosing EGL allocator, or NULL when the type does not match.
 */
struct wlf_egl_allocator *wlf_egl_allocator_from_allocator(
	struct wlf_allocator *allocator);

#endif // ALLOCATOR_EGL_ALLOCATOR_H
