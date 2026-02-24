/**
 * @file        wlf_y_animator.h
 * @brief       Y-axis animator interface in wlframe.
 * @details     This file declares the Y position animator, which interpolates
 *              a target float value from a start position to an end position
 *              over a specified duration.
 * @author      YaoBing Xiao
 * @date        2026-02-24
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-24, initial version\n
 */

#ifndef ANIMATOR_WLF_Y_ANIMATOR_H
#define ANIMATOR_WLF_Y_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_y_animator;

/**
 * @brief Y position animator structure.
 *
 * Similar to Qt Quick's YAnimator.
 */
struct wlf_y_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting Y position */
	float to;                  /**< Ending Y position */
	float current;             /**< Current Y position */
	float *target;             /**< Pointer to target Y variable */
};

/**
 * @brief Creates a Y position animator.
 *
 * @param duration Duration in milliseconds.
 * @param from Starting Y position.
 * @param to Ending Y position.
 * @param target Pointer to target Y variable to animate.
 * @return Pointer to the created animator, or NULL on failure.
 */
struct wlf_y_animator *wlf_y_animator_create(int64_t duration,
	 float from, float to, float *target);

/**
 * @brief Checks whether an animator is a Y animator.
 *
 * @param animator Animator instance to check.
 * @return true if the animator is of Y animator type, otherwise false.
 */
bool wlf_animator_is_y(const struct wlf_animator *animator);

/**
 * @brief Casts a base animator to a Y animator.
 *
 * @param animator Base animator instance.
 * @return Y animator instance if the type matches, otherwise NULL.
 */
struct wlf_y_animator *wlf_y_animator_from_animator(struct wlf_animator *animator);

#endif // ANIMATOR_WLF_Y_ANIMATOR_H
