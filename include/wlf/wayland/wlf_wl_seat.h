/**
 * @file        wlf_wl_seat.h
 * @brief       Wayland seat management utility for wlframe.
 * @details     This file provides structures and functions for managing Wayland seats,
 *              including seat manager, seat creation/destruction, capability tracking,
 *              and event signaling for seat lifecycle. Supports multiple seats, pointer
 *              and keyboard lists, and default seat management.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef WAYLAND_WLF_WL_SEAT_H
#define WAYLAND_WLF_WL_SEAT_H

#include "wlf/utils/wlf_linked_list.h"

#include <stdint.h>

struct wl_seat;

struct wlf_seat;
struct wlf_wl_display;
struct wlf_wl_pointer;
struct wlf_keyboard;

/**
 * @brief Structure for managing all Wayland seats in a display.
 */
struct wlf_wl_seat_manager {
	struct wlf_wl_display *display;      /**< Associated Wayland display */
	struct wlf_wl_seat *default_seat;    /**< Default seat pointer */
	struct wlf_linked_list seats;        /**< List of all seats */
};

/**
 * @brief Structure representing a Wayland seat and its state.
 */
struct wlf_wl_seat {
	struct wlf_seat *base;               /**< Backend-independent seat base */
	struct wl_seat *seat;                /**< Wayland seat pointer */

	struct wlf_wl_pointer *active_pointer; /**< Currently active pointer */

	struct wlf_linked_list link;         /**< Linked list node for seat manager */
	struct wlf_linked_list pointers;     /**< List of pointers for this seat */
	struct wlf_linked_list keyboards;    /**< List of keyboards for this seat */

	uint32_t capabilities;               /**< Current seat capabilities */
	uint32_t accumulated_capabilities;   /**< Accumulated capabilities */
};

/**
 * @brief Create a new seat manager for a Wayland display.
 * @param display Pointer to the associated wlf_wl_display.
 * @return Pointer to the newly created seat manager.
 */
struct wlf_wl_seat_manager *wlf_wl_seat_manager_create(
	struct wlf_wl_display *display);

/**
 * @brief Destroy a seat manager and free its resources.
 * @param manager Pointer to the seat manager.
 */
void wlf_wl_seat_manager_destroy(
	struct wlf_wl_seat_manager *manager);

/**
 * @brief Get a seat by name from the seat manager.
 * @param manager Pointer to the seat manager.
 * @param name Name of the seat to search for.
 * @return Pointer to the found seat, or NULL if not found.
 */
struct wlf_wl_seat *wlf_wl_seat_manager_get_seat(
	struct wlf_wl_seat_manager *manager, const char *name);

/**
 * @brief Get the default seat from the seat manager.
 * @param manager Pointer to the seat manager.
 * @return Pointer to the default seat, or NULL if not set.
 */
struct wlf_wl_seat *wlf_wl_seat_manager_get_default_seat(
	struct wlf_wl_seat_manager *manager);

/**
 * @brief Create a new Wayland seat object.
 * @param seat Pointer to the wl_seat.
 * @return Pointer to the newly created wlf_wl_seat.
 */
struct wlf_wl_seat *wlf_wl_seat_create(struct wl_seat *seat);

/**
 * @brief Destroy a Wayland seat object and free its resources.
 * @param seat Pointer to the wlf_wl_seat.
 */
void wlf_wl_seat_destroy(struct wlf_wl_seat *seat);

#endif // WAYLAND_WLF_WL_SEAT_H
