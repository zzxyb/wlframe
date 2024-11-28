#ifndef WLF_INPUT_DEVICE_H
#define WLF_INPUT_DEVICE_H

#include "wlf/util/wlf_double_list.h"
#include "wlf/util/wlf_signal.h"

enum wlf_button_state {
	WLF_BUTTON_RELEASED, /**< Button is released */
	WLF_BUTTON_PRESSED,  /**< Button is pressed */
};

/**
 * @brief Type of an input device.
 */
enum wlf_input_device_type {
	WLF_INPUT_DEVICE_UNKNOWN, /**< Unknown pointer type */
	WLF_INPUT_DEVICE_KEYBOARD, /**< Keyboard input device */
	WLF_INPUT_DEVICE_MOUSE, /**< Mouse pointer type */
	WLF_INPUT_DEVICE_TOUCHPAD, /**< Touchpad pointer type */
	WLF_INPUT_DEVICE_TRACKBALL, /**< Trackball pointer type */
	WLF_INPUT_DEVICE_JOYSTICK, /**< Joystick pointer type */
	WLF_INPUT_DEVICE_POINTING_STICK, /**< Pointing stick pointer type */
	WLF_INPUT_DEVICE_TOUCH, /**< Touch input device */
	WLF_INPUT_DEVICE_TABLET, /**< Tablet input device */
	WLF_INPUT_DEVICE_TABLET_PAD, /**< Tablet pad input device */
	WLF_INPUT_DEVICE_SWITCH, /**< Switch input device */
};

/**
 * @brief An input device.
 *
 * Depending on its type, the input device can be converted to a more specific
 * type. See the various wlf_*_from_input_device() functions.
 *
 * Input devices are typically advertised by the new_input event in
 * struct wlf_backend.
 */
struct wlf_input_device {
	enum wlf_input_device_type type; /**< The type of the input device */
	char *name; /**< The name of the input device, may be NULL */

	struct {
		struct wlf_signal destroy; /**< Signal emitted when the device is destroyed */
	} events;

	void *data; /**< Pointer to device-specific data */
};

#endif
