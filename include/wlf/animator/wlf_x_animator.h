/**
 * @file        wlf_x_animator.h
 * @brief       X-axis animator interface in wlframe.
 * @details     This file declares the X position animator, which interpolates
 *              a target float value from a start position to an end position
 *              over a specified duration.
 * @author      YaoBing Xiao
 * @date        2026-02-24
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-24, initial version\n
 */

#ifndef ANIMATOR_WLF_X_ANIMATOR_H
#define ANIMATOR_WLF_X_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_x_animator;

/**
 * @brief X position animator structure.
 *
 * Similar to Qt Quick's XAnimator.
 */
struct wlf_x_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting X position */
	float to;                  /**< Ending X position */
	float current;             /**< Current X position */
	float *target;             /**< Pointer to target X variable */
};

/**
 * @brief Creates an X position animator.
 *
 * @param duration Duration in milliseconds.
 * @param from Starting X position.
 * @param to Ending X position.
 * @param target Pointer to target X variable to animate.
 * @return Pointer to the created animator, or NULL on failure.
 */
struct wlf_x_animator *wlf_x_animator_create(int64_t duration,
	float from, float to, float *target);

/**
 * @brief Checks whether an animator is an X animator.
 *
 * @param animator Animator instance to check.
 * @return true if the animator is of X animator type, otherwise false.
 */
bool wlf_animator_is_x(const struct wlf_animator *animator);

/**
 * @brief Casts a base animator to an X animator.
 *
 * @param animator Base animator instance.
 * @return X animator instance if the type matches, otherwise NULL.
 */
struct wlf_x_animator *wlf_x_animator_from_animator(struct wlf_animator *animator);

#endif // ANIMATOR_WLF_X_ANIMATOR_H
