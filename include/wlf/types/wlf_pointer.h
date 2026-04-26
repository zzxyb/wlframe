/**
 * @file        wlf_pointer.h
 * @brief       Pointer input interface and event definitions in wlframe.
 * @details     This file defines the generic pointer object, its virtual interface,
 *              button and axis enums, and all pointer-related event payload
 *              structures used with wlf_signal.
 * @author      YaoBing Xiao
 * @date        2026-04-26
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-26, initial version\n
 */

#ifndef TYPES_WLF_POINTER_H
#define TYPES_WLF_POINTER_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define WLF_POINTER_BUTTONS_CAP 16

struct wlf_pointer;

/**
 * @brief Pointer button state.
 */
enum wlf_pointer_button_state {
	WLF_POINTER_BUTTON_STATE_RELEASED = 0,  /**< Button is released. */
	WLF_POINTER_BUTTON_STATE_PRESSED,       /**< Button is pressed. */
};

/**
 * @brief Pointer axis orientation.
 */
enum wlf_pointer_axis {
	WLF_POINTER_AXIS_VERTICAL_SCROLL = 0,   /**< Vertical scroll axis. */
	WLF_POINTER_AXIS_HORIZONTAL_SCROLL,     /**< Horizontal scroll axis. */
};

/**
 * @brief Source of an axis event.
 */
enum wlf_pointer_axis_source {
	WLF_POINTER_AXIS_SOURCE_WHEEL = 0,      /**< Traditional wheel movement. */
	WLF_POINTER_AXIS_SOURCE_FINGER,         /**< Finger-based scrolling (touchpad). */
	WLF_POINTER_AXIS_SOURCE_CONTINUOUS,     /**< Continuous axis source. */
	WLF_POINTER_AXIS_SOURCE_WHEEL_TILT,     /**< Wheel tilt movement. */
};

/**
 * @brief Relative direction of axis movement.
 */
enum wlf_pointer_axis_relative_direction {
	WLF_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL = 0, /**< Direction is unchanged. */
	WLF_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED,      /**< Direction is inverted. */
};

/**
 * @brief Virtual methods for pointer operations.
 */
struct wlf_pointer_impl {
	const char *name; /**< Implementation name. */

	/**
	 * @brief Destroys backend-specific pointer resources.
	 * @param pointer Pointer instance.
	 */
	void (*destroy)(struct wlf_pointer *pointer);
};

/**
 * @brief Base pointer object.
 *
 * Backend-specific pointer types should embed this struct.
 */
struct wlf_pointer {
	const struct wlf_pointer_impl *impl; /**< Virtual method table. */

	uint32_t buttons[WLF_POINTER_BUTTONS_CAP]; /**< Currently pressed button codes. */
	size_t button_count; /**< Number of valid entries in buttons. */

	struct {
		struct wlf_signal destroy; /**< Emitted before pointer is destroyed. */

		struct wlf_signal motion; /**< Payload: wlf_pointer_motion_event. */
		struct wlf_signal motion_absolute; /**< Payload: wlf_pointer_motion_absolute_event. */
		struct wlf_signal button; /**< Payload: wlf_pointer_button_event. */
		struct wlf_signal axis; /**< Payload: wlf_pointer_axis_event. */
		struct wlf_signal frame; /**< Emitted to delimit a logical event frame. */

		struct wlf_signal swipe_begin; /**< Payload: wlf_pointer_swipe_begin_event. */
		struct wlf_signal swipe_update; /**< Payload: wlf_pointer_swipe_update_event. */
		struct wlf_signal swipe_end; /**< Payload: wlf_pointer_swipe_end_event. */

		struct wlf_signal pinch_begin; /**< Payload: wlf_pointer_pinch_begin_event. */
		struct wlf_signal pinch_update; /**< Payload: wlf_pointer_pinch_update_event. */
		struct wlf_signal pinch_end; /**< Payload: wlf_pointer_pinch_end_event. */

		struct wlf_signal hold_begin; /**< Payload: wlf_pointer_hold_begin_event. */
		struct wlf_signal hold_end; /**< Payload: wlf_pointer_hold_end_event. */
	} events;

	void *data; /**< User-defined data pointer. */
};

/**
 * @brief Relative motion event payload.
 */
struct wlf_pointer_motion_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	double delta_x, delta_y; /**< Accelerated relative motion delta. */
	double unaccel_dx, unaccel_dy; /**< Unaccelerated relative motion delta. */
};

/**
 * @brief Absolute motion event payload.
 */
struct wlf_pointer_motion_absolute_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	double x, y; /**< Normalized absolute position in [0, 1]. */
};

/**
 * @brief Button event payload.
 */
struct wlf_pointer_button_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	uint32_t button; /**< Button code. */
	enum wlf_pointer_button_state state; /**< Pressed or released state. */
};

/**
 * @brief Axis event payload.
 */
struct wlf_pointer_axis_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	enum wlf_pointer_axis_source source; /**< Axis source type. */
	enum wlf_pointer_axis orientation; /**< Axis orientation. */
	enum wlf_pointer_axis_relative_direction relative_direction; /**< Logical axis direction relation. */
	double delta; /**< Continuous axis delta. */
	int32_t delta_discrete; /**< Discrete axis steps (120 units per step). */
};

/**
 * @brief Swipe begin event payload.
 */
struct wlf_pointer_swipe_begin_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	uint32_t fingers; /**< Number of fingers in gesture. */
};

/**
 * @brief Swipe update event payload.
 */
struct wlf_pointer_swipe_update_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	uint32_t fingers; /**< Number of fingers in gesture. */
	double dx, dy; /**< Relative gesture center movement. */
};

/**
 * @brief Swipe end event payload.
 */
struct wlf_pointer_swipe_end_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	bool cancelled; /**< True if gesture was cancelled. */
};

/**
 * @brief Pinch begin event payload.
 */
struct wlf_pointer_pinch_begin_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	uint32_t fingers; /**< Number of fingers in gesture. */
};

/**
 * @brief Pinch update event payload.
 */
struct wlf_pointer_pinch_update_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	uint32_t fingers; /**< Number of fingers in gesture. */
	double dx, dy; /**< Relative gesture center movement. */
	double scale; /**< Absolute scale factor from begin event. */
	double rotation; /**< Relative clockwise rotation in degrees. */
};

/**
 * @brief Pinch end event payload.
 */
struct wlf_pointer_pinch_end_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	bool cancelled; /**< True if gesture was cancelled. */
};

/**
 * @brief Hold begin event payload.
 */
struct wlf_pointer_hold_begin_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	uint32_t fingers; /**< Number of fingers in gesture. */
};

/**
 * @brief Hold end event payload.
 */
struct wlf_pointer_hold_end_event {
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	bool cancelled; /**< True if gesture was cancelled. */
};

/**
 * @brief Initializes a pointer object.
 *
 * @param pointer Pointer object to initialize.
 * @param impl Implementation virtual methods.
 */
void wlf_pointer_init(struct wlf_pointer *pointer, const struct wlf_pointer_impl *impl);

/**
 * @brief Destroys a pointer object.
 *
 * @param pointer Pointer object to destroy.
 */
void wlf_pointer_destroy(struct wlf_pointer *pointer);

#endif // TYPES_WLF_POINTER_H
