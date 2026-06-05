/**
 * @file        wlf_wl_seat.h
 * @brief       Wayland wl_seat global wrapper for wlframe.
 * @details     Wraps wl_seat bound from the Wayland registry.  Tracks seat
 *              capability changes and name, and provides factory helpers to
 *              obtain wlf_wl_pointer, wlf_wl_keyboard, and wlf_wl_touch.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_SEAT_H
#define WAYLAND_WLF_WL_SEAT_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_seat;
struct wlf_wl_keyboard;
struct wlf_wl_pointer;
struct wlf_wl_touch;

/** @brief Seat capabilities bitmask (mirrors wl_seat_capability). */
enum wlf_wl_seat_capability {
	WLF_WL_SEAT_CAPABILITY_POINTER  = 1,
	WLF_WL_SEAT_CAPABILITY_KEYBOARD = 2,
	WLF_WL_SEAT_CAPABILITY_TOUCH    = 4,
};

/**
 * @brief Wayland input seat global.
 */
struct wlf_wl_seat {
	struct wl_seat *wl_seat;  /**< Underlying Wayland object. */
	uint32_t version;
	uint32_t capabilities;    /**< Current capability bitmask. */
	char *name;               /**< Seat identifier (heap-allocated). */

	struct {
		struct wlf_signal destroy;       /**< Emitted before destruction. */
		struct wlf_signal capabilities;  /**< Emitted on capability change. Payload: wlf_wl_seat. */
		struct wlf_signal name;          /**< Emitted on name change. Payload: wlf_wl_seat. */
	} events;
};

/**
 * @brief Binds wl_seat from the Wayland registry.
 *
 * @param wl_registry Wayland registry.
 * @param name        Global name of wl_seat.
 * @param version     Advertised version.
 * @return Newly allocated wlf_wl_seat, or NULL on failure.
 */
struct wlf_wl_seat *wlf_wl_seat_create(struct wl_registry *wl_registry,
	uint32_t name, uint32_t version);

/**
 * @brief Destroys a wlf_wl_seat. Passing NULL is a no-op.
 */
void wlf_wl_seat_destroy(struct wlf_wl_seat *seat);

#endif /* WAYLAND_WLF_WL_SEAT_H */
