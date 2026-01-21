/**
 * @file        wlf_touch.h
 * @brief       Touch gesture recognition library for wlframe.
 * @details     This file provides structures and functions for touch gesture recognition,
 *              including support for various gestures like touch, move, rotate, pinch, and delay.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef TOUCH_WLF_TOUCH_H
#define TOUCH_WLF_TOUCH_H

#include <stdint.h>

/**
 * @brief Touch action types.
 */
enum wlf_touch_action_type {
	/**
	 * Pressing or releasing a finger to or from the touch device.
	 */
	WLF_TOUCH_ACTION_TOUCH,
	/**
	 * A difference in the position of the center of the touch group over time.
	 */
	WLF_TOUCH_ACTION_MOVE,
	/**
	 * The angle of rotation between each finger and the center of the touch
	 * group changes.
	 */
	WLF_TOUCH_ACTION_ROTATE,
	/**
	 * The average distance between each finger and the center of the touch
	 * group changes.
	 */
	WLF_TOUCH_ACTION_PINCH,
	/**
	 * No change within the configured thresholds over a certain time frame.
	 */
	WLF_TOUCH_ACTION_DELAY,
};

/**
 * @brief Touch mode flags.
 * @details Represents a change in the number of touch points in the current touch group.
 * DOWN represents pressing a finger against the touch device, and UP represents
 * removing the finger from the device.
 */
enum wlf_touch_mode {
	WLF_TOUCH_UP = 1 << 0,    /**< Finger lifted from touch device */
	WLF_TOUCH_DOWN = 1 << 1,  /**< Finger pressed to touch device */
};

/**
 * @brief Movement direction flags.
 * @details Represents the directions in which a WLF_TOUCH_ACTION_MOVE can occur. Both
 * POSITIVE_X and NEGATIVE_X corresponds to movement in either direction along
 * the X axis; the same holds for Y.
 */
enum wlf_touch_move_dir {
	WLF_TOUCH_MOVE_POSITIVE_X = 1 << 0,  /**< Movement in positive X direction */
	WLF_TOUCH_MOVE_POSITIVE_Y = 1 << 1,  /**< Movement in positive Y direction */
	WLF_TOUCH_MOVE_NEGATIVE_X = 1 << 2,  /**< Movement in negative X direction */
	WLF_TOUCH_MOVE_NEGATIVE_Y = 1 << 3,  /**< Movement in negative Y direction */
};

/**
 * @brief Rotation direction flags.
 * @details Represents the direction of rotation in which a WLF_TOUCH_ACTION_ROTATE can
 * occur. Both CLOCKWISE and ANTICLOCKWISE corresponds to a rotation in either
 * direction.
 */
enum wlf_touch_rotate_dir {
	WLF_TOUCH_ROTATE_CLOCKWISE = 1 << 0,      /**< Clockwise rotation */
	WLF_TOUCH_ROTATE_ANTICLOCKWISE = 1 << 1,  /**< Anticlockwise rotation */
};

/**
 * @brief Pinch direction flags.
 * @details Represents the direction in which a WLF_TOUCH_ACTION_PINCH can occur.
 */
enum wlf_touch_scale_dir {
	WLF_TOUCH_PINCH_IN = 1 << 0,   /**< Pinch inward (zoom out) */
	WLF_TOUCH_PINCH_OUT = 1 << 1,  /**< Pinch outward (zoom in) */
};

/**
 * @brief Forward declarations.
 */
struct wlf_touch_gesture_progress;
struct wlf_touch_progress_tracker;

/**
 * @brief Touch engine structure.
 * @details Represents the internal state. The only holder of state information.
 */
struct wlf_touch_engine;

/**
 * @brief Touch gesture structure.
 * @details Represents a gesture, which is defined as a sequence of actions.
 * Declarative, no info of state.
 */
struct wlf_touch_gesture;

/**
 * @brief Touch action structure.
 * @details Represents a part of a gesture.
 * Declarative, no information of state.
 */
struct wlf_touch_action;

/**
 * @brief Touch target structure.
 * @details A region or other delimited area, within which an action listens.
 * Declarative, no information of state.
 */
struct wlf_touch_target;

/**
 * @brief Creates a new touch engine.
 * @return A pointer to the newly created touch engine.
 */
struct wlf_touch_engine *wlf_touch_engine_create(void);

/**
 * @brief Creates a new gesture within the touch engine.
 * @param engine Pointer to the touch engine.
 * @return A pointer to the newly created gesture.
 */
struct wlf_touch_gesture *wlf_touch_gesture_create(
	struct wlf_touch_engine *engine);

/**
 * @brief Sets the minimum movement tolerance for an action.
 * @details Set a min movement before it starts counting as movement.
 * Useful for, for instance long pressing, in case of a not 100% stable finger
 * or to ignore possible miss-swipes.
 * @param action Pointer to the action.
 * @param min Minimum movement threshold.
 */
void wlf_touch_action_move_tolerance(struct wlf_touch_action *action, double min);

/**
 * @brief Creates a new touch target area.
 * @param engine Pointer to the touch engine.
 * @param x X coordinate of the target area.
 * @param y Y coordinate of the target area.
 * @param width Width of the target area.
 * @param height Height of the target area.
 * @return A pointer to the newly created target.
 */
struct wlf_touch_target *wlf_touch_target_create(
	struct wlf_touch_engine *engine, double x, double y,
	double width, double height);

/**
 * @brief Registers a touch event with the progress tracker.
 * @param tracker Pointer to the progress tracker.
 * @param timestamp Milliseconds from an arbitrary epoch (e.g. CLOCK_MONOTONIC).
 * @param slot The slot of this event (e.g. which finger the event was caused by).
 * @param mode The touch mode (up or down).
 * @param x X coordinate of the touch.
 * @param y Y coordinate of the touch.
 */
void wlf_touch_progress_register_touch(
	struct wlf_touch_progress_tracker *tracker,
	uint32_t timestamp, int slot, enum wlf_touch_mode mode,
	double x, double y);

/**
 * @brief Registers a touch movement event with the progress tracker.
 * @param tracker Pointer to the progress tracker.
 * @param timestamp Milliseconds from an arbitrary epoch (e.g. CLOCK_MONOTONIC).
 * @param slot The slot of the event (e.g. which finger).
 * @param dx Delta X movement.
 * @param dy Delta Y movement.
 */
void wlf_touch_progress_register_move(
	struct wlf_touch_progress_tracker *tracker,
	uint32_t timestamp, int slot,
	double dx, double dy);


/**
 * @brief Adds a touch action to a gesture.
 * @param gesture Pointer to the gesture.
 * @param mode Touch mode flags.
 * @return Pointer to the newly created action.
 */
struct wlf_touch_action *wlf_touch_gesture_add_touch(
	struct wlf_touch_gesture *gesture, uint32_t mode);

/**
 * @brief Adds a move action to a gesture.
 * @param gesture Pointer to the gesture.
 * @param direction Movement direction flags.
 * @return Pointer to the newly created action.
 */
struct wlf_touch_action *wlf_touch_gesture_add_move(
	struct wlf_touch_gesture *gesture, uint32_t direction);

/**
 * @brief Adds a rotate action to a gesture.
 * @param gesture Pointer to the gesture.
 * @param direction Rotation direction flags.
 * @return Pointer to the newly created action.
 */
struct wlf_touch_action *wlf_touch_gesture_add_rotate(
	struct wlf_touch_gesture *gesture, uint32_t direction);

/**
 * @brief Adds a pinch action to a gesture.
 * @param gesture Pointer to the gesture.
 * @param direction Pinch direction flags.
 * @return Pointer to the newly created action.
 */
struct wlf_touch_action *wlf_touch_gesture_add_pinch(
	struct wlf_touch_gesture *gesture, uint32_t direction);

/**
 * @brief Adds a delay action to a gesture.
 * @param gesture Pointer to the gesture.
 * @param duration Duration in milliseconds.
 * @return Pointer to the newly created action.
 */
struct wlf_touch_action *wlf_touch_gesture_add_delay(
	struct wlf_touch_gesture *gesture, uint32_t duration);



/**
 * @brief Sets the threshold of change for an action to be considered complete.
 * @details The map of threshold units to action type is as follows:
 * - WLF_TOUCH_ACTION_TOUCH:  number of touch points
 * - WLF_TOUCH_ACTION_MOVE:   positional units (percent of screen)
 * - WLF_TOUCH_ACTION_ROTATE: degrees
 * - WLF_TOUCH_ACTION_PINCH:  scale (in percent) of original touch
 * - WLF_TOUCH_ACTION_DELAY:  milliseconds (must be positive)
 * @param action Pointer to the action.
 * @param threshold The threshold value.
 */
void wlf_touch_action_set_threshold(
	struct wlf_touch_action *action, int threshold);

/**
 * @brief Sets a target that the action must reach to be considered complete.
 * @details Valid for WLF_TOUCH_ACTION_MOVE, where the movement must finish.
 * Cannot be used together with threshold.
 * For WLF_TOUCH_ACTION_TOUCH, target defines where we must press.
 * @param action Pointer to the action.
 * @param target Pointer to the target area.
 */
void wlf_touch_action_set_target(
	struct wlf_touch_action *action,
	struct wlf_touch_target *target);

/**
 * @brief Sets the minimum duration for an action.
 * @details Sets the minimum duration this action must take place during to be considered
 * a match. For instance, if not all n fingers are pressed the same frame,
 * we can consider n fingers down within duration_ms to be a n-finger touch.
 * @param action Pointer to the action.
 * @param duration_ms Duration in milliseconds.
 */
void wlf_touch_action_set_duration(
	struct wlf_touch_action *action,
	uint32_t duration_ms);

/**
 * @brief Gets the current progress of an action.
 * @param tracker Pointer to the progress tracker.
 * @return Progress value between 0 and 1.
 */
double wlf_touch_action_get_progress(
	struct wlf_touch_progress_tracker *tracker);


/**
 * @brief Resets the progress of a gesture.
 * @param gesture Pointer to the gesture progress.
 */
void wlf_touch_gesture_reset_progress(
	struct wlf_touch_gesture_progress *gesture);

/**
 * @brief Gets the current active action for a gesture.
 * @param gesture Pointer to the gesture progress.
 * @return Pointer to the active action.
 */
struct wlf_touch_action *wlf_touch_gesture_get_current_action(
	struct wlf_touch_gesture_progress *gesture);

/**
 * @brief Fills an array of gesture progress pointers sorted by progress.
 * @param tracker Pointer to the progress tracker.
 * @param array Array to fill with gesture progress pointers.
 * @param count Number of elements in the array.
 * @return The highest progress value.
 */
double wlf_touch_fill_progress_array(
	struct wlf_touch_progress_tracker *tracker,
	struct wlf_touch_gesture_progress **array,
	uint32_t count);

/**
 * @brief Handles and returns a completed gesture.
 * @details Returns a completed gesture, and resets its progress.
 * If none exist, returns NULL. Call repeatedly to get all finished gestures.
 * @param tracker Pointer to the progress tracker.
 * @return Pointer to the completed gesture, or NULL if none.
 */
struct wlf_touch_gesture *wlf_touch_handle_finished_gesture(
	struct wlf_touch_progress_tracker *tracker);

/**
 * @brief Creates a new progress tracker.
 * @param engine Pointer to the touch engine.
 * @return Pointer to the newly created progress tracker.
 */
struct wlf_touch_progress_tracker *wlf_touch_progress_tracker_create(
	struct wlf_touch_engine *engine);

/**
 * @brief Gets the number of gestures in a progress tracker.
 * @param tracker Pointer to the progress tracker.
 * @return Number of gestures.
 */
uint32_t wlf_touch_progress_tracker_n_gestures(
	struct wlf_touch_progress_tracker *tracker);

/**
 * @brief Gets gesture progress by index.
 * @param tracker Pointer to the progress tracker.
 * @param index Index of the gesture.
 * @return Pointer to the gesture progress.
 */
struct wlf_touch_gesture_progress *wlf_touch_gesture_get_progress(
	struct wlf_touch_progress_tracker *tracker, uint32_t index);

/**
 * @brief Gets the progress value of a gesture.
 * @param gesture Pointer to the gesture progress.
 * @return Progress value between 0 and 1.
 */
double wlf_touch_gesture_progress_get_progress(
	struct wlf_touch_gesture_progress *gesture);

#endif
