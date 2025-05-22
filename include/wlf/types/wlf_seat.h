/**
 * @file        wlf_seat.h
 * @brief       Seat type definition for wlframe.
 * @details     This file provides the structure definition for a Wayland seat,
 *              which represents a group of input devices (keyboard, pointer, etc.).
 *              The seat structure can be extended with additional data as needed.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef TYPES_WLF_SEAT_H
#define TYPES_WLF_SEAT_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

/**
 * @brief Structure representing a Wayland seat (input device group).
 */
struct wlf_seat {
	char *name;    /**< Name of the seat */
	void *data;    /**< User data or backend-specific data */

	struct {
		struct wlf_signal destroy;       /**< Signal emitted when seat is destroyed */
	} events;
};

/**
 * @brief Create a new seat object.
 * @param name Name of the seat.
 * @return Pointer to the newly created seat.
 */
struct wlf_seat *wlf_seat_create(const char *name);

/**
 * @brief Destroy a seat object and free its resources.
 * @param seat Pointer to the seat object.
 */
void wlf_seat_destroy(struct wlf_seat *seat);

#endif // TYPES_WLF_SEAT_H
