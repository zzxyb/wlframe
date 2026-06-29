/**
 * @file        wlf_zwp_relative_pointer_manager_v1.h
 * @brief       Wayland relative-pointer-unstable-v1 wrapper for wlframe.
 * @details     Provides relative (delta-based) pointer motion events
 *              independently of the on-screen cursor position.  Useful for
 *              first-person camera control, game input, etc.
 *
 *              Interfaces wrapped:
 *              - wlf_zwp_relative_pointer_manager_v1 — global manager.
 *              - wlf_zwp_relative_pointer_v1         — relative-motion
 *                event source attached to a specific wl_pointer.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_ZWP_RELATIVE_POINTER_MANAGER_V1_H
#define WAYLAND_WLF_ZWP_RELATIVE_POINTER_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_pointer;
struct zwp_relative_pointer_manager_v1;
struct zwp_relative_pointer_v1;

/**
 * @brief State populated before emitting the @c relative_motion signal.
 */
struct wlf_zwp_relative_pointer_v1_state {
	/** High 32 bits of the timestamp in microseconds (64-bit split). */
	uint32_t utime_hi;
	/** Low 32 bits of the timestamp in microseconds (64-bit split). */
	uint32_t utime_lo;

	double dx;          /**< Compositor-accelerated X delta (surface units) */
	double dy;          /**< Compositor-accelerated Y delta (surface units) */
	double dx_unaccel;  /**< Unaccelerated X delta (raw hardware units)     */
	double dy_unaccel;  /**< Unaccelerated Y delta (raw hardware units)     */
};

/**
 * @brief Relative-pointer event source.
 *
 * Attach this to a @c wl_pointer to receive delta-motion events.
 */
struct wlf_zwp_relative_pointer_v1 {
	struct zwp_relative_pointer_v1 *base;

	struct wlf_zwp_relative_pointer_v1_state current;

	struct {
		/** Data: self — emitted on every relative-motion event */
		struct wlf_signal relative_motion;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Wrapper around the zwp_relative_pointer_manager_v1 global.
 */
struct wlf_zwp_relative_pointer_manager_v1 {
	struct zwp_relative_pointer_manager_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the zwp_relative_pointer_manager_v1 global.
 */
struct wlf_zwp_relative_pointer_manager_v1 *
wlf_zwp_relative_pointer_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager.
 */
void wlf_zwp_relative_pointer_manager_v1_destroy(
	struct wlf_zwp_relative_pointer_manager_v1 *manager);

/**
 * @brief Create a relative pointer for @p pointer.
 *
 * @param manager  Bound manager.
 * @param pointer  The pointer to attach to.
 * @return New relative-pointer wrapper, or NULL on failure.
 */
struct wlf_zwp_relative_pointer_v1 *
wlf_zwp_relative_pointer_manager_v1_get_relative_pointer(
	struct wlf_zwp_relative_pointer_manager_v1 *manager,
	struct wl_pointer *pointer);

/**
 * @brief Destroy a relative-pointer wrapper.
 */
void wlf_zwp_relative_pointer_v1_destroy(
	struct wlf_zwp_relative_pointer_v1 *relative_pointer);

#endif /* WAYLAND_WLF_ZWP_RELATIVE_POINTER_MANAGER_V1_H */
