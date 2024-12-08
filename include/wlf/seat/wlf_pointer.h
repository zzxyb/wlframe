#ifndef WLF_POINTER_H
#define WLF_POINTER_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/seat/wlf_input_device.h"

#include <stdint.h>

#define WLF_POINTER_BUTTONS_CAP 16
#define WLF_POINTER_AXIS_DISCRETE_STEP 120

struct wlf_pointer_impl;

enum wlf_button_state {
	WLF_BUTTON_RELEASED, /**< Button is released */
	WLF_BUTTON_PRESSED,  /**< Button is pressed */
};

enum wlf_pointer_axis_source {
	/**
	 * a physical wheel rotation
	 */
	WLF_POINTER_AXIS_SOURCE_WHEEL = 0,
	/**
	 * finger on a touch surface
	 */
	WLF_POINTER_AXIS_SOURCE_FINGER = 1,
	/**
	 * continuous coordinate space
	 */
	WLF_POINTER_AXIS_SOURCE_CONTINUOUS = 2,
	/**
	 * a physical wheel tilt
	 * @since 6
	 */
	WLF_POINTER_AXIS_SOURCE_WHEEL_TILT = 3,
};

enum wlf_pointer_axis {
	/**
	 * vertical axis
	 */
	WLF_POINTER_AXIS_VERTICAL_SCROLL = 0,
	/**
	 * horizontal axis
	 */
	WLF_POINTER_AXIS_HORIZONTAL_SCROLL = 1,
};

enum wlf_pointer_axis_relative_direction {
	/**
	 * physical motion matches axis direction
	 */
	WLF_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL = 0,
	/**
	 * physical motion is the inverse of the axis direction
	 */
	WLF_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED = 1,
};

/**
 * @brief A structure representing a pointer input device.
 */
struct wlf_pointer {
	struct wlf_input_device base; /**< Base input device structure */

	const struct wlf_pointer_impl *impl; /**< Pointer implementation details */

	char *output_name; /**< Name of the output device */

	uint32_t buttons[WLF_POINTER_BUTTONS_CAP]; /**< Array of button states */
	size_t button_count; /**< Number of buttons currently pressed */

	struct {
		struct wlf_signal motion; /**< Signal for pointer motion events */
		struct wlf_signal motion_absolute; /**< Signal for absolute pointer motion events */
		struct wlf_signal button; /**< Signal for button events */
		struct wlf_signal axis; /**< Signal for axis events */
		struct wlf_signal frame; /**< Signal for frame events */

		struct wlf_signal swipe_begin; /**< Signal for swipe begin events */
		struct wlf_signal swipe_update; /**< Signal for swipe update events */
		struct wlf_signal swipe_end; /**< Signal for swipe end events */

		struct wlf_signal pinch_begin; /**< Signal for pinch begin events */
		struct wlf_signal pinch_update; /**< Signal for pinch update events */
		struct wlf_signal pinch_end; /**< Signal for pinch end events */

		struct wlf_signal hold_begin; /**< Signal for hold begin events */
		struct wlf_signal hold_end; /**< Signal for hold end events */
	} events; /**< Events related to pointer actions */

	void *data; /**< Pointer to device-specific data */
};

/**
 * @brief Represents a motion event for a pointer.
 */
struct wlf_pointer_motion_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	double delta_x, delta_y; /**< Change in position */
	double unaccel_dx, unaccel_dy; /**< Unaccelerated change in position */
};

/**
 * @brief Represents an absolute motion event for a pointer.
 */
struct wlf_pointer_motion_absolute_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	double x, y; /**< Absolute position from 0 to 1 */
};

/**
 * @brief Represents a button event for a pointer.
 */
struct wlf_pointer_button_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	uint32_t button; /**< Button identifier */
	enum wlf_button_state state; /**< State of the button (pressed/released) */
};

/**
 * @brief Represents an axis event for a pointer.
 */
struct wlf_pointer_axis_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	enum wlf_pointer_axis_source source; /**< Source of the axis event */
	enum wlf_pointer_axis orientation; /**< Orientation of the axis */
	enum wlf_pointer_axis_relative_direction relative_direction; /**< Relative direction of the axis */
	double delta; /**< Change in axis position */
	int32_t delta_discrete; /**< Discrete change in axis position */
};

/**
 * @brief Represents a swipe begin event for a pointer.
 */
struct wlf_pointer_swipe_begin_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	uint32_t fingers; /**< Number of fingers involved in the swipe */
};

/**
 * @brief Represents a swipe update event for a pointer.
 */
struct wlf_pointer_swipe_update_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	uint32_t fingers; /**< Number of fingers involved in the swipe */
	double dx, dy; /**< Relative coordinates of the logical center of the gesture */
};

/**
 * @brief Represents a swipe end event for a pointer.
 */
struct wlf_pointer_swipe_end_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	bool cancelled; /**< Indicates if the swipe was cancelled */
};

/**
 * @brief Represents a pinch begin event for a pointer.
 */
struct wlf_pointer_pinch_begin_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	uint32_t fingers; /**< Number of fingers involved in the pinch */
};

/**
 * @brief Represents a pinch update event for a pointer.
 */
struct wlf_pointer_pinch_update_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	uint32_t fingers; /**< Number of fingers involved in the pinch */
	double dx, dy; /**< Relative coordinates of the logical center of the gesture */
	double scale; /**< Absolute scale compared to the begin event */
	double rotation; /**< Relative angle in degrees clockwise compared to the previous event */
};

/**
 * @brief Represents a pinch end event for a pointer.
 */
struct wlf_pointer_pinch_end_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	bool cancelled; /**< Indicates if the pinch was cancelled */
};

/**
 * @brief Represents a hold begin event for a pointer.
 */
struct wlf_pointer_hold_begin_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	uint32_t fingers; /**< Number of fingers involved in the hold */
};

/**
 * @brief Represents a hold end event for a pointer.
 */
struct wlf_pointer_hold_end_event {
	struct wlf_pointer *pointer; /**< Pointer to the associated wlf_pointer */
	uint32_t time_msec; /**< Time of the event in milliseconds */
	bool cancelled; /**< Indicates if the hold was cancelled */
};

/**
 * @brief Get a struct wlf_pointer from a struct wlf_input_device.
 *
 * Asserts that the input device is a pointer.
 * @param input_device The input device to convert
 * @return A pointer to the corresponding wlf_pointer structure
 */
struct wlf_pointer *wlf_pointer_from_input_device(
	struct wlf_input_device *input_device);

void wlf_pointer_init(struct wlf_pointer *pointer,
	const struct wlf_pointer_impl *impl, const char *name, enum wlf_input_device_type type);

void wlf_pointer_finish(struct wlf_pointer *pointer);

void wlf_pointer_notify_button(struct wlf_pointer *pointer,
	struct wlf_pointer_button_event *event);

#endif
