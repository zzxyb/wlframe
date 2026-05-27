/**
 * @file        wlf_wl_touch.h
 * @brief       Wayland backend implementation for wlf_touch.
 * @details     This file provides the Wayland-specific bindings for touch handling
 *              in wlframe. It connects the generic wlf_touch abstraction with
 *              the Wayland wl_touch protocol object obtained from a wl_seat.
 *
 *              Touch coordinates from wl_touch are surface-local pixel values.
 *              The `surface` pointer in touch events is set to the wl_surface
 *              reported by the compositor on down events.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-01
 * @version     v1.0
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-01, initial version\n
 */

#ifndef WAYLAND_WLF_WL_TOUCH_H
#define WAYLAND_WLF_WL_TOUCH_H

#include "wlf/types/wlf_touch.h"

#include <stdbool.h>

struct wl_seat;
struct wl_touch;

/**
 * @brief Wayland-backed wlf_touch implementation.
 *
 * Wraps a wl_touch object obtained from a wl_seat.
 *
 * The `base` field must be first to allow safe casting from wlf_touch.
 */
struct wlf_wl_touch {
	struct wlf_touch base;   /**< Generic touch base (must be first). */
	struct wl_touch *touch;  /**< Underlying Wayland touch object. */
};

/**
 * @brief Creates a wlf_touch backed by a Wayland wl_seat.
 * @param seat The Wayland seat from which to obtain the touch device.
 * @return Pointer to the newly created wlf_touch, or NULL on failure.
 */
struct wlf_touch *wlf_wl_touch_create(struct wl_seat *seat);

/**
 * @brief Checks whether a wlf_touch is backed by the Wayland backend.
 * @param touch The touch instance.
 * @return true if Wayland-backed, false otherwise.
 */
bool wlf_touch_is_wayland(const struct wlf_touch *touch);

/**
 * @brief Converts a generic wlf_touch to a wlf_wl_touch.
 * @param touch The generic touch.
 * @return Wayland-specific struct, or NULL if not Wayland-backed.
 */
struct wlf_wl_touch *wlf_wl_touch_from_touch(struct wlf_touch *touch);

#endif // WAYLAND_WLF_WL_TOUCH_H
