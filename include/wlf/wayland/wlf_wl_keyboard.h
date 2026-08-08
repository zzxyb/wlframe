/**
 * @file        wlf_wl_keyboard.h
 * @brief       Wayland wl_keyboard implementation of wlf_keyboard.
 * @details     Wraps a wl_keyboard Wayland protocol object and translates its
 *              events into the wlframe wlf_keyboard signal system.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_KEYBOARD_H
#define WAYLAND_WLF_WL_KEYBOARD_H

#include "wlf/types/wlf_keyboard.h"

struct wl_keyboard;
struct wl_seat;

/**
 * @brief Wayland wl_keyboard backed implementation of wlf_keyboard.
 *
 * Embeds wlf_keyboard as the first member so that a pointer to
 * wlf_wl_keyboard and wlf_keyboard are interchangeable.
 */
struct wlf_wl_keyboard {
	struct wlf_keyboard base; /**< Generic keyboard base; must be first. */

	struct wl_keyboard *wl_keyboard; /**< Underlying Wayland object. */
};

/**
 * @brief Creates a wlf_wl_keyboard by obtaining wl_keyboard from a wl_seat.
 *
 * Calls wl_seat_get_keyboard() and registers all Wayland listeners.
 *
 * @param seat  Wayland seat that has keyboard capability.
 * @return Newly allocated wlf_wl_keyboard, or NULL on failure.
 */
struct wlf_wl_keyboard *wlf_wl_keyboard_create(struct wl_seat *seat);

/**
 * @brief Destroys and frees a wlf_wl_keyboard.
 *
 * Emits wlf_keyboard.events.destroy, releases the wl_keyboard, and frees
 * the allocation. Passing NULL is a no-op.
 */
void wlf_wl_keyboard_destroy(struct wlf_wl_keyboard *keyboard);

/**
 * @brief Downcasts a wlf_keyboard to wlf_wl_keyboard.
 *
 * Asserts that keyboard was created by this implementation.
 */
struct wlf_wl_keyboard *wlf_wl_keyboard_from_base(struct wlf_keyboard *base);

#endif /* WAYLAND_WLF_WL_KEYBOARD_H */
