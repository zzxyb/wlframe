#ifndef WLF_INPUT_DEVICE_H
#define WLF_INPUT_DEVICE_H

#include "wlf/utils/wlf_signal.h"

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
	WLF_INPUT_DEVICE_TOUCHSCREEN, /**< Touchscreen input device */
	WLF_INPUT_DEVICE_TABLET, /**< Tablet input device */
	WLF_INPUT_DEVICE_TABLET_PAD, /**< Tablet pad input device */
	WLF_INPUT_DEVICE_SWITCH, /**< Switch input device */
};

/**
 * @brief Capabilities of an input device.
 */
enum wlf_input_device_capability {
	WLF_INPUT_DEVICE_CAP_KEYBOARD, /**< Device has keyboard input capability */
	WLF_INPUT_DEVICE_CAP_POINTER, /**< Device has pointer input capability */
	WLF_INPUT_DEVICE_CAP_TOUCH, /**< Device has touch input capability */
	WLF_INPUT_DEVICE_CAP_TABLET_TOOL, /**< Device has tablet tool capability */
	WLF_INPUT_DEVICE_CAP_TABLET_PAD, /**< Device has tablet pad capability */
	WLF_INPUT_DEVICE_CAP_GESTURE, /**< Device has gesture input capability */
	WLF_INPUT_DEVICE_CAP_SWITCH, /**< Device has switch input capability */
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
	enum wlf_input_device_capability capabilities; /**< Capabilities of the input device */
	enum wlf_input_device_type type; /**< The type of the input device */
	char *name; /**< The name of the input device, may be NULL */

	struct {
		struct wlf_signal destroy; /**< Signal emitted when the device is destroyed */
	} events;

	void *data; /**< Pointer to device-specific data */
};

void wlf_input_device_init(struct wlf_input_device *device, enum wlf_input_device_type type,
		const char *name, enum wlf_input_device_capability capabilities);

void wlf_input_device_fnish(struct wlf_input_device *device);

#endif
