/**
 * @file        wlf_touch.h
 * @brief       Touch input device type definition for wlframe.
 * @details     This file provides the structure definitions and event handling for touch input devices,
 *              including touch down, up, motion, frame, and cancel events. It supports multi-touch
 *              input with touch point tracking by ID, and provides event signals for touch actions.
 *              The implementation is designed to be extensible for device-specific data and can
 *              handle various touch screen and touchpad devices.
 * @author      YaoBing Xiao
 * @date        2024-10-14
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-10-14, initial version\n
 */

#ifndef TYPES_WLF_TOUCH_H
#define TYPES_WLF_TOUCH_H

#include <stdint.h>

#include "wlf/utils/wlf_signal.h"

struct wlf_touch {
	struct {
		struct wlf_signal down;            /**< Signal for touch down events */
		struct wlf_signal up;              /**< Signal for touch up events */
		struct wlf_signal motion;          /**< Signal for touch motion events */
		struct wlf_signal frame;           /**< Signal for touch frame events */
		struct wlf_signal cancel;          /**< Signal for touch cancel events */
		struct wlf_signal destroy;         /**< Signal emitted when touch is destroyed */
	} events; /**< Events related to touch actions */

	void *data; /**< Pointer to device-specific data */
};

struct wlf_touch_down_event {
	struct wlf_touch *touch;
	uint32_t time_msec;
	int32_t touch_id;
	double x, y;
};

struct wlf_touch_up_event {
	struct wlf_touch *touch;
	uint32_t time_msec;
	int32_t touch_id;
};

struct wlf_touch_motion_event {
	struct wlf_touch *touch;
	uint32_t time_msec;
	int32_t touch_id;
	double x, y;
};

struct wlf_touch_cancel_event {
	struct wlf_touch *touch;
	uint32_t time_msec;
	int32_t touch_id;
};

#endif // TYPES_WLF_TOUCH_H
