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
struct wlf_pointer;
struct wlf_keyboard;
struct wlf_touch;
struct wlf_window;
struct wl_compositor;
struct wl_shm;
struct wp_cursor_shape_manager_v1;

/** @brief Seat capabilities bitmask (mirrors wl_seat_capability). */
enum wlf_wl_seat_capability {
	WLF_WL_SEAT_CAPABILITY_POINTER  = 1, /**< Seat provides a pointer device. */
	WLF_WL_SEAT_CAPABILITY_KEYBOARD = 2, /**< Seat provides a keyboard device. */
	WLF_WL_SEAT_CAPABILITY_TOUCH    = 4, /**< Seat provides a touch device. */
};

/**
 * @brief Wayland input seat global.
 */
struct wlf_wl_seat {
	struct wl_seat *wl_seat;  /**< Underlying Wayland object. */
	uint32_t version; /**< Bound Wayland protocol version. */
	uint32_t capabilities;    /**< Current capability bitmask. */
	char *name;               /**< Seat identifier (heap-allocated). */
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	struct wlf_keyboard *keyboard; /**< Keyboard associated with the seat. */
	struct wlf_touch *touch; /**< Touch device associated with the seat. */
	struct wlf_window *pointer_window; /**< Window with pointer focus. */
	struct wlf_window *keyboard_window; /**< Window with keyboard focus. */
	struct wlf_window *touch_window; /**< Window receiving the active touch sequence. */
	struct wp_cursor_shape_manager_v1 *cursor_shape_manager; /**< Borrowed cursor-shape manager, when available. */
	struct wl_compositor *cursor_compositor; /**< Borrowed compositor used to create cursor surfaces. */
	struct wl_shm *cursor_shm; /**< Borrowed shared memory global used for themed cursors. */
	struct {
		struct wlf_listener pointer_enter, /**< Listener for pointer enter events. */ pointer_leave, /**< Listener for pointer leave events. */ pointer_motion; /**< Listener for pointer motion events. */
		struct wlf_listener pointer_button, /**< Listener for pointer button events. */ pointer_axis, /**< Listener for pointer axis events. */ pointer_frame; /**< Listener for pointer frame events. */
		struct wlf_listener keyboard_keymap, /**< Listener for keyboard keymap events. */ keyboard_enter, /**< Listener for keyboard enter events. */ keyboard_leave; /**< Listener for keyboard leave events. */
		struct wlf_listener keyboard_key, /**< Listener for keyboard key events. */ keyboard_modifiers; /**< Listener for keyboard modifiers events. */
		struct wlf_listener keyboard_repeat_info; /**< Listener for keyboard repeat info events. */
		struct wlf_listener touch_down, /**< Listener for touch down events. */ touch_up, /**< Listener for touch up events. */ touch_motion, /**< Listener for touch motion events. */ touch_cancel; /**< Listener for touch cancel events. */
		struct wlf_listener touch_frame, /**< Listener for touch frame events. */ touch_shape, /**< Listener for touch shape events. */ touch_orientation; /**< Listener for touch orientation events. */
		struct wlf_listener pointer_window_destroy; /**< Listener for pointer window destroy events. */
		struct wlf_listener keyboard_window_destroy; /**< Listener for keyboard window destroy events. */
		struct wlf_listener touch_window_destroy; /**< Listener for touch window destroy events. */
	} listeners; /**< Listeners attached to related objects. */

	struct {
		struct wlf_signal destroy;       /**< Emitted before destruction. */
		struct wlf_signal capabilities;  /**< Emitted on capability change. */
		struct wlf_signal name;          /**< Emitted on name change. */
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
 *
 * @param seat Wayland seat to configure or destroy.
 */
void wlf_wl_seat_destroy(struct wlf_wl_seat *seat);

/**
 * @brief Updates the globals used for cursor-shape or themed cursor rendering.
 *
 * @param seat Wayland seat to configure or destroy.
 * @param shape_manager Borrowed cursor-shape manager, or NULL to use themed cursors.
 * @param compositor Borrowed compositor used to create cursor surfaces.
 * @param shm Borrowed shared memory global used to load themed cursors.
 */
void wlf_wl_seat_configure_cursor(struct wlf_wl_seat *seat,
	struct wp_cursor_shape_manager_v1 *shape_manager,
	struct wl_compositor *compositor, struct wl_shm *shm);

#endif /* WAYLAND_WLF_WL_SEAT_H */
