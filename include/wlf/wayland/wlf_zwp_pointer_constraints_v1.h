/**
 * @file        wlf_zwp_pointer_constraints_v1.h
 * @brief       Wayland pointer-constraints-unstable-v1 wrapper for wlframe.
 * @details     Provides confined and locked pointer modes for a surface.
 *
 *              Interfaces wrapped:
 *              - wlf_zwp_pointer_constraints_v1 — global manager.
 *              - wlf_zwp_locked_pointer_v1      — locked pointer; cursor
 *                hidden, motion events replaced by lock status events.
 *              - wlf_zwp_confined_pointer_v1    — confined pointer; cursor
 *                stays within the surface (or a sub-region of it).
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_ZWP_POINTER_CONSTRAINTS_V1_H
#define WAYLAND_WLF_ZWP_POINTER_CONSTRAINTS_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_surface;
struct wl_pointer;
struct wl_region;
struct zwp_pointer_constraints_v1;
struct zwp_locked_pointer_v1;
struct zwp_confined_pointer_v1;

/**
 * @brief Pointer constraint lifetime values.
 *
 * Mirrors zwp_pointer_constraints_v1_lifetime without pulling in the
 * generated protocol header in the public API.
 */
enum wlf_pointer_constraint_lifetime {
	WLF_POINTER_CONSTRAINT_LIFETIME_ONESHOT   = 1,
	WLF_POINTER_CONSTRAINT_LIFETIME_PERSISTENT = 2,
};

/**
 * @brief Wrapper around the zwp_pointer_constraints_v1 global.
 */
struct wlf_zwp_pointer_constraints_v1 {
	struct zwp_pointer_constraints_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Locked pointer object.  While locked, the cursor is hidden and no
 *        motion events are sent; instead locked/unlocked status events fire.
 */
struct wlf_zwp_locked_pointer_v1 {
	struct zwp_locked_pointer_v1 *base;

	struct {
		/** Data: self — lock is granted by compositor */
		struct wlf_signal locked;
		/** Data: self — lock is released by compositor */
		struct wlf_signal unlocked;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Confined pointer object.  The pointer is kept within the surface
 *        (or a sub-region) until the compositor revokes the confinement.
 */
struct wlf_zwp_confined_pointer_v1 {
	struct zwp_confined_pointer_v1 *base;

	struct {
		/** Data: self — confinement is granted */
		struct wlf_signal confined;
		/** Data: self — confinement is released */
		struct wlf_signal unconfined;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the zwp_pointer_constraints_v1 global.
 */
struct wlf_zwp_pointer_constraints_v1 *
wlf_zwp_pointer_constraints_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager.
 */
void wlf_zwp_pointer_constraints_v1_destroy(
	struct wlf_zwp_pointer_constraints_v1 *constraints);

/**
 * @brief Lock the pointer to a position within @p surface.
 *
 * @param constraints Bound manager.
 * @param surface     Surface to lock the pointer to.
 * @param pointer     The pointer to lock.
 * @param region      Optional region (NULL means entire surface).
 * @param lifetime    One of wlf_pointer_constraint_lifetime.
 * @return New locked pointer wrapper, or NULL on failure.
 */
struct wlf_zwp_locked_pointer_v1 *
wlf_zwp_pointer_constraints_v1_lock_pointer(
	struct wlf_zwp_pointer_constraints_v1 *constraints,
	struct wl_surface *surface, struct wl_pointer *pointer,
	struct wl_region *region, uint32_t lifetime);

/**
 * @brief Confine the pointer within @p surface.
 *
 * @param constraints Bound manager.
 * @param surface     Surface to confine the pointer in.
 * @param pointer     The pointer to confine.
 * @param region      Optional confinement region (NULL means entire surface).
 * @param lifetime    One of wlf_pointer_constraint_lifetime.
 * @return New confined pointer wrapper, or NULL on failure.
 */
struct wlf_zwp_confined_pointer_v1 *
wlf_zwp_pointer_constraints_v1_confine_pointer(
	struct wlf_zwp_pointer_constraints_v1 *constraints,
	struct wl_surface *surface, struct wl_pointer *pointer,
	struct wl_region *region, uint32_t lifetime);

/**
 * @brief Set a hint for the cursor position when the lock is released.
 *
 * @param locked      Locked pointer object.
 * @param surface_x   X position hint (surface-local coordinates).
 * @param surface_y   Y position hint (surface-local coordinates).
 */
void wlf_zwp_locked_pointer_v1_set_cursor_position_hint(
	struct wlf_zwp_locked_pointer_v1 *locked,
	double surface_x, double surface_y);

/**
 * @brief Update the locked region.
 *
 * @param locked  Locked pointer object.
 * @param region  New lock region (NULL means entire surface).
 */
void wlf_zwp_locked_pointer_v1_set_region(
	struct wlf_zwp_locked_pointer_v1 *locked, struct wl_region *region);

/**
 * @brief Destroy a locked pointer object.
 */
void wlf_zwp_locked_pointer_v1_destroy(
	struct wlf_zwp_locked_pointer_v1 *locked);

/**
 * @brief Update the confinement region.
 *
 * @param confined  Confined pointer object.
 * @param region    New confinement region (NULL means entire surface).
 */
void wlf_zwp_confined_pointer_v1_set_region(
	struct wlf_zwp_confined_pointer_v1 *confined, struct wl_region *region);

/**
 * @brief Destroy a confined pointer object.
 */
void wlf_zwp_confined_pointer_v1_destroy(
	struct wlf_zwp_confined_pointer_v1 *confined);

#endif /* WAYLAND_WLF_ZWP_POINTER_CONSTRAINTS_V1_H */
