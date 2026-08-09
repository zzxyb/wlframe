/**
 * @file        wlf_zwp_keyboard_shortcuts_inhibit_manager_v1.h
 * @brief       Wayland keyboard-shortcuts-inhibit-unstable-v1 wrapper.
 * @details     Allows a client surface to inhibit compositor keyboard-shortcut
 *              handling so that all key events are forwarded to the client.
 *
 *              Interfaces wrapped:
 *              - wlf_zwp_keyboard_shortcuts_inhibit_manager_v1  — global.
 *              - wlf_zwp_keyboard_shortcuts_inhibitor_v1        — one
 *                inhibitor per (surface, seat) pair.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_ZWP_KEYBOARD_SHORTCUTS_INHIBIT_MANAGER_V1_H
#define WAYLAND_WLF_ZWP_KEYBOARD_SHORTCUTS_INHIBIT_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_surface;
struct wl_seat;
struct zwp_keyboard_shortcuts_inhibit_manager_v1;
struct zwp_keyboard_shortcuts_inhibitor_v1;

/**
 * @brief Wrapper around the zwp_keyboard_shortcuts_inhibit_manager_v1 global.
 */
struct wlf_zwp_keyboard_shortcuts_inhibit_manager_v1 {
	struct zwp_keyboard_shortcuts_inhibit_manager_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Represents an active keyboard-shortcuts inhibitor.
 *
 * Events:
 *  - active:   compositor grants the inhibitor (all shortcuts suppressed).
 *  - inactive: compositor revokes the inhibitor (shortcuts restored).
 */
struct wlf_zwp_keyboard_shortcuts_inhibitor_v1 {
	struct zwp_keyboard_shortcuts_inhibitor_v1 *base;

	struct {
		/** Data: self */
		struct wlf_signal active;
		/** Data: self */
		struct wlf_signal inactive;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the zwp_keyboard_shortcuts_inhibit_manager_v1 global.
 */
struct wlf_zwp_keyboard_shortcuts_inhibit_manager_v1 *
wlf_zwp_keyboard_shortcuts_inhibit_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 */
void wlf_zwp_keyboard_shortcuts_inhibit_manager_v1_destroy(
	struct wlf_zwp_keyboard_shortcuts_inhibit_manager_v1 *manager);

/**
 * @brief Create a keyboard-shortcuts inhibitor for a surface/seat pair.
 *
 * @param manager  Bound manager.
 * @param surface  The surface that will receive all key events.
 * @param seat     The seat whose shortcuts are to be inhibited.
 * @return A new inhibitor wrapper, or NULL on failure.
 */
struct wlf_zwp_keyboard_shortcuts_inhibitor_v1 *
wlf_zwp_keyboard_shortcuts_inhibit_manager_v1_inhibit_shortcuts(
	struct wlf_zwp_keyboard_shortcuts_inhibit_manager_v1 *manager,
	struct wl_surface *surface, struct wl_seat *seat);

/**
 * @brief Destroy an inhibitor object.
 */
void wlf_zwp_keyboard_shortcuts_inhibitor_v1_destroy(
	struct wlf_zwp_keyboard_shortcuts_inhibitor_v1 *inhibitor);

#endif /* WAYLAND_WLF_ZWP_KEYBOARD_SHORTCUTS_INHIBIT_MANAGER_V1_H */
