/**
 * @file        wlf_seat.h
 * @brief       Seat type definition for wlframe.
 * @details     This file provides the structure definition for a Wayland seat,
 *              which represents a group of input devices (keyboard, pointer, etc.).
 *              The seat structure can be extended with additional data as needed.
 * @author      xyb
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, xyb, 2024-05-20, initial version\n
 */

#ifndef TYPES_WLF_SEAT_H
#define TYPES_WLF_SEAT_H

#include <stdint.h>

/**
 * @brief Structure representing a Wayland seat (input device group).
 */
struct wlf_seat {
	char *name;    /**< Name of the seat */
	void *data;    /**< User data or backend-specific data */
};

#endif // TYPES_WLF_SEAT_H
