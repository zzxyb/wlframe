/**
 * @file        wlf_wl_pointer.h
 * @brief       Wayland backend implementation for wlf_pointer.
 * @details     This file provides the Wayland-specific bindings for pointer handling
 *              in wlframe. It connects the generic wlf_pointer abstraction with
 *              the Wayland wl_pointer protocol object obtained from a wl_seat.
 *
 *              Pointer axis events are accumulated per logical frame before emission.
 *              Surface-local coordinates from wl_pointer enter/motion are reported
 *              via the motion_absolute signal in surface-local pixel units.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-01
 * @version     v1.0
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-01, initial version\n
 */

#ifndef WAYLAND_WLF_WL_POINTER_H
#define WAYLAND_WLF_WL_POINTER_H

#include "wlf/types/wlf_pointer.h"

#include <stdint.h>
#include <stdbool.h>

struct wl_seat;
struct wl_pointer;
struct wl_surface;

/**
 * @brief Per-axis accumulated state for a single wl_pointer frame.
 */
struct wlf_wl_pointer_axis_frame {
	bool valid;                                            /**< True if axis data is pending. */
	bool stop;                                             /**< True if axis was stopped. */
	bool has_value120;                                     /**< True if value120 data arrived. */
	bool has_direction;                                    /**< True if direction data arrived. */
	uint32_t time_msec;                                    /**< Timestamp from axis event. */
	double value;                                          /**< Continuous axis delta. */
	int32_t value120;                                      /**< High-resolution discrete (120 = 1 step). */
	enum wlf_pointer_axis_relative_direction direction;    /**< Relative direction. */
};

/**
 * @brief Wayland-backed wlf_pointer implementation.
 *
 * Wraps a wl_pointer object obtained from a wl_seat. Axis events are
 * accumulated between wl_pointer.frame events and emitted as a single
 * wlf_pointer_axis_event per axis per frame.
 *
 * The `base` field must be first to allow safe casting from wlf_pointer.
 */
struct wlf_wl_pointer {
	struct wlf_pointer base;          /**< Generic pointer base (must be first). */
	struct wl_pointer *pointer;       /**< Underlying Wayland pointer object. */
	struct wl_surface *focus_surface; /**< Currently focused Wayland surface. */
	uint32_t enter_serial;            /**< Serial from the last enter event. */

	/** Accumulated axis frame state, indexed by wlf_pointer_axis. */
	struct wlf_wl_pointer_axis_frame frame_axes[2];
	bool frame_has_source;                            /**< True if axis_source arrived this frame. */
	enum wlf_pointer_axis_source frame_source;        /**< Accumulated axis source. */
};

/**
 * @brief Creates a wlf_pointer backed by a Wayland wl_seat.
 * @param seat The Wayland seat from which to obtain the pointer.
 * @return Pointer to the newly created wlf_pointer, or NULL on failure.
 */
struct wlf_pointer *wlf_wl_pointer_create(struct wl_seat *seat);

/**
 * @brief Checks whether a wlf_pointer is backed by the Wayland backend.
 * @param pointer The pointer instance.
 * @return true if Wayland-backed, false otherwise.
 */
bool wlf_pointer_is_wayland(const struct wlf_pointer *pointer);

/**
 * @brief Converts a generic wlf_pointer to a wlf_wl_pointer.
 * @param pointer The generic pointer.
 * @return Wayland-specific struct, or NULL if not Wayland-backed.
 */
struct wlf_wl_pointer *wlf_wl_pointer_from_pointer(struct wlf_pointer *pointer);

#endif // WAYLAND_WLF_WL_POINTER_H
