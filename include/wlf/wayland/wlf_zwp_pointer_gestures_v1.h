/**
 * @file        wlf_zwp_pointer_gestures_v1.h
 * @brief       Wayland pointer-gestures-unstable-v1 wrapper for wlframe.
 * @details     Adds touchpad gesture recognition (swipe, pinch, hold) on top
 *              of the core pointer.
 *
 *              Interfaces wrapped:
 *              - wlf_zwp_pointer_gestures_v1            — global manager.
 *              - wlf_zwp_pointer_gesture_swipe_v1       — two-finger swipe.
 *              - wlf_zwp_pointer_gesture_pinch_v1       — pinch / rotate.
 *              - wlf_zwp_pointer_gesture_hold_v1        — hold gesture
 *                (v3+).
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_ZWP_POINTER_GESTURES_V1_H
#define WAYLAND_WLF_ZWP_POINTER_GESTURES_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_pointer;
struct wl_surface;
struct zwp_pointer_gestures_v1;

/* -------------------------------------------------------------------------
 * Swipe gesture
 * ---------------------------------------------------------------------- */

/**
 * @brief State snapshot for one swipe gesture event.
 *
 * Populated before emitting begin / update / end signals.
 */
struct wlf_zwp_pointer_gesture_swipe_v1_state {
	uint32_t serial;           /**< Serial from begin                        */
	uint32_t time;             /**< Millisecond timestamp                    */
	struct wl_surface *surface; /**< Surface under the gesture (begin only) */
	uint32_t fingers;          /**< Number of fingers (begin only)           */
	double dx;                 /**< Delta X (update only)                    */
	double dy;                 /**< Delta Y (update only)                    */
	int cancelled;             /**< Non-zero if gesture was cancelled (end)  */
};

/**
 * @brief Swipe gesture wrapper.
 */
struct wlf_zwp_pointer_gesture_swipe_v1 {
	struct zwp_pointer_gesture_swipe_v1 *base;

	struct wlf_zwp_pointer_gesture_swipe_v1_state current;

	struct {
		/** Data: self */
		struct wlf_signal begin;
		/** Data: self */
		struct wlf_signal update;
		/** Data: self */
		struct wlf_signal end;
		struct wlf_signal destroy;
	} events;
};

/* -------------------------------------------------------------------------
 * Pinch gesture
 * ---------------------------------------------------------------------- */

/**
 * @brief State snapshot for one pinch gesture event.
 */
struct wlf_zwp_pointer_gesture_pinch_v1_state {
	uint32_t serial;
	uint32_t time;
	struct wl_surface *surface;
	uint32_t fingers;
	double dx;
	double dy;
	double scale;     /**< Scale factor relative to gesture start (update) */
	double rotation;  /**< Rotation in degrees (update)                    */
	int cancelled;
};

/**
 * @brief Pinch gesture wrapper.
 */
struct wlf_zwp_pointer_gesture_pinch_v1 {
	struct zwp_pointer_gesture_pinch_v1 *base;

	struct wlf_zwp_pointer_gesture_pinch_v1_state current;

	struct {
		struct wlf_signal begin;
		struct wlf_signal update;
		struct wlf_signal end;
		struct wlf_signal destroy;
	} events;
};

/* -------------------------------------------------------------------------
 * Hold gesture (protocol version >= 3)
 * ---------------------------------------------------------------------- */

/**
 * @brief State snapshot for one hold gesture event.
 */
struct wlf_zwp_pointer_gesture_hold_v1_state {
	uint32_t serial;
	uint32_t time;
	struct wl_surface *surface;
	uint32_t fingers;
	int cancelled;
};

/**
 * @brief Hold gesture wrapper.
 */
struct wlf_zwp_pointer_gesture_hold_v1 {
	struct zwp_pointer_gesture_hold_v1 *base;

	struct wlf_zwp_pointer_gesture_hold_v1_state current;

	struct {
		struct wlf_signal begin;
		struct wlf_signal end;
		struct wlf_signal destroy;
	} events;
};

/* -------------------------------------------------------------------------
 * Manager
 * ---------------------------------------------------------------------- */

/**
 * @brief Wrapper around the zwp_pointer_gestures_v1 global.
 */
struct wlf_zwp_pointer_gestures_v1 {
	struct zwp_pointer_gestures_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the zwp_pointer_gestures_v1 global.
 */
struct wlf_zwp_pointer_gestures_v1 *wlf_zwp_pointer_gestures_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager.
 */
void wlf_zwp_pointer_gestures_v1_destroy(
	struct wlf_zwp_pointer_gestures_v1 *gestures);

/**
 * @brief Create a swipe gesture object for @p pointer.
 */
struct wlf_zwp_pointer_gesture_swipe_v1 *
wlf_zwp_pointer_gestures_v1_get_swipe_gesture(
	struct wlf_zwp_pointer_gestures_v1 *gestures,
	struct wl_pointer *pointer);

/**
 * @brief Create a pinch gesture object for @p pointer.
 */
struct wlf_zwp_pointer_gesture_pinch_v1 *
wlf_zwp_pointer_gestures_v1_get_pinch_gesture(
	struct wlf_zwp_pointer_gestures_v1 *gestures,
	struct wl_pointer *pointer);

/**
 * @brief Create a hold gesture object for @p pointer (requires version >= 3).
 */
struct wlf_zwp_pointer_gesture_hold_v1 *
wlf_zwp_pointer_gestures_v1_get_hold_gesture(
	struct wlf_zwp_pointer_gestures_v1 *gestures,
	struct wl_pointer *pointer);

/**
 * @brief Destroy a swipe gesture object.
 */
void wlf_zwp_pointer_gesture_swipe_v1_destroy(
	struct wlf_zwp_pointer_gesture_swipe_v1 *swipe);

/**
 * @brief Destroy a pinch gesture object.
 */
void wlf_zwp_pointer_gesture_pinch_v1_destroy(
	struct wlf_zwp_pointer_gesture_pinch_v1 *pinch);

/**
 * @brief Destroy a hold gesture object.
 */
void wlf_zwp_pointer_gesture_hold_v1_destroy(
	struct wlf_zwp_pointer_gesture_hold_v1 *hold);

#endif /* WAYLAND_WLF_ZWP_POINTER_GESTURES_V1_H */
