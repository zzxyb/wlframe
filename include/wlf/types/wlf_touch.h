/**
 * @file        wlf_touch.h
 * @brief       Touch input interface and event definitions in wlframe.
 * @details     This file defines the generic touch object, its virtual interface,
 *              and all touch-related event payload structures used with
 *              wlf_signal.
 * @author      YaoBing Xiao
 * @date        2026-04-26
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-26, initial version\n
 */

#ifndef TYPES_WLF_TOUCH_H
#define TYPES_WLF_TOUCH_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wlf_touch;

/**
 * @brief Virtual methods for touch operations.
 */
struct wlf_touch_impl {
	const char *name; /**< Implementation name. */

	/**
	 * @brief Destroys backend-specific touch resources.
	 * @param touch Touch instance.
	 */
	void (*destroy)(struct wlf_touch *touch);
};

/**
 * @brief Base touch object.
 *
 * Backend-specific touch types should embed this struct.
 */
struct wlf_touch {
	const struct wlf_touch_impl *impl; /**< Virtual method table. */

	struct {
		struct wlf_signal destroy; /**< Emitted before touch is destroyed. */
		struct wlf_signal down; /**< Payload: wlf_touch_down_event. */
		struct wlf_signal up; /**< Payload: wlf_touch_up_event. */
		struct wlf_signal motion; /**< Payload: wlf_touch_motion_event. */
		struct wlf_signal cancel; /**< Payload: wlf_touch_cancel_event. */
		struct wlf_signal frame; /**< Emitted to delimit a logical event frame. */
		struct wlf_signal shape; /**< Payload: wlf_touch_shape_event. */
		struct wlf_signal orientation; /**< Payload: wlf_touch_orientation_event. */
	} events;

	void *data; /**< User-defined data pointer. */
};

/**
 * @brief Touch down event payload.
 */
struct wlf_touch_down_event {
	struct wlf_touch *touch; /**< Touch device that generated the event. */
	void *surface;  /**< surface touched. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	int32_t touch_id; /**< Logical touch point identifier. */
	double x, y; /**< Normalized absolute position in [0, 1]. */
};

/**
 * @brief Touch up event payload.
 */
struct wlf_touch_up_event {
	struct wlf_touch *touch; /**< Touch device that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	int32_t touch_id; /**< Logical touch point identifier. */
};

/**
 * @brief Touch motion event payload.
 */
struct wlf_touch_motion_event {
	struct wlf_touch *touch; /**< Touch device that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	int32_t touch_id; /**< Logical touch point identifier. */
	double x, y; /**< Normalized absolute position in [0, 1]. */
};

/**
 * @brief Touch cancel event payload.
 */
struct wlf_touch_cancel_event {
	struct wlf_touch *touch; /**< Touch device that generated the event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	int32_t touch_id; /**< Logical touch point identifier. */
};

/**
 * @brief Touch shape event payload.
 */
struct wlf_touch_shape_event {
	struct wlf_touch *touch; /**< Touch device that generated the event. */
	int32_t touch_id; /**< Logical touch point identifier. */
	double major, minor; /**< Major/minor axis lengths of touch contact. */
};

/**
 * @brief Touch orientation event payload.
 */
struct wlf_touch_orientation_event {
	struct wlf_touch *touch; /**< Touch device that generated the event. */
	int32_t touch_id; /**< Logical touch point identifier. */
	double orientation; /**< Contact orientation angle in degrees. */
};

/**
 * @brief Initializes a touch object.
 *
 * @param touch Touch object to initialize.
 * @param impl Implementation virtual methods.
 */
void wlf_touch_init(struct wlf_touch *touch, const struct wlf_touch_impl *impl);

/**
 * @brief Destroys a touch object.
 *
 * @param touch Touch object to destroy.
 */
void wlf_touch_destroy(struct wlf_touch *touch);

#endif // TYPES_WLF_TOUCH_H
