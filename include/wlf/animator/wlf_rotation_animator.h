/**
 * @file        wlf_rotation_animator.h
 * @brief       Rotation animator interface in wlframe.
 * @details     This file declares the rotation animator, which interpolates
 *              a target angle value from a start angle to an end angle
 *              over a specified duration.
 * @author      YaoBing Xiao
 * @date        2026-02-24
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-24, initial version\n
 */

#ifndef ANIMATOR_WLF_ROTATION_ANIMATOR_H
#define ANIMATOR_WLF_ROTATION_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_rotation_animator;

/**
 * @brief Rotation axis enumeration.
 */
enum wlf_rotation_axis {
	WLF_ROTATION_AXIS_X,  /**< Rotate around X axis */
	WLF_ROTATION_AXIS_Y,  /**< Rotate around Y axis */
	WLF_ROTATION_AXIS_Z,  /**< Rotate around Z axis (2D rotation) */
};

/**
 * @brief Rotation animator structure.
 *
 * Similar to Qt Quick's RotationAnimator.
 */
struct wlf_rotation_animator {
	struct wlf_animator base;           /**< Base animator */

	float from;                         /**< Starting angle in degrees */
	float to;                           /**< Ending angle in degrees */
	float current;                      /**< Current angle in degrees */
	float *target;                      /**< Pointer to target angle variable */
	enum wlf_rotation_axis axis;        /**< Rotation axis */
	float origin_x;                     /**< Rotation origin X (normalized 0.0-1.0) */
	float origin_y;                     /**< Rotation origin Y (normalized 0.0-1.0) */
};

/**
 * @brief Creates a rotation animator.
 *
 * @param duration Duration in milliseconds.
 * @param from Starting angle in degrees.
 * @param to Ending angle in degrees.
 * @param target Pointer to target angle variable to animate.
 * @return Pointer to the created animator, or NULL on failure.
 */
struct wlf_rotation_animator *wlf_rotation_animator_create(int64_t duration,
	 float from, float to, float *target);

/**
 * @brief Checks whether an animator is a rotation animator.
 *
 * @param animator Animator instance to check.
 * @return true if the animator is of rotation animator type, otherwise false.
 */
bool wlf_animator_is_rotation(const struct wlf_animator *animator);

/**
 * @brief Casts a base animator to a rotation animator.
 *
 * @param animator Base animator instance.
 * @return Rotation animator instance if the type matches, otherwise NULL.
 */
struct wlf_rotation_animator *wlf_rotation_animator_from_animator(struct wlf_animator *animator);

/**
 * @brief Sets the rotation axis.
 *
 * @param animator Rotation animator.
 * @param axis Rotation axis.
 */
void wlf_rotation_animator_set_axis(struct wlf_rotation_animator *animator,
	 enum wlf_rotation_axis axis);

/**
 * @brief Sets the rotation origin.
 *
 * @param animator Rotation animator.
 * @param origin_x Origin X coordinate (normalized 0.0-1.0, default 0.5).
 * @param origin_y Origin Y coordinate (normalized 0.0-1.0, default 0.5).
 */
void wlf_rotation_animator_set_origin(struct wlf_rotation_animator *animator,
	 float origin_x, float origin_y);

#endif // ANIMATOR_WLF_ROTATION_ANIMATOR_H
