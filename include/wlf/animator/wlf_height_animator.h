/**
 * @file        wlf_height_animator.h
 * @brief       Height animator interface in wlframe.
 * @details     This file declares the height animator, which interpolates
 *              a target float value from a start height to an end height
 *              over a specified duration.
 * @author      YaoBing Xiao
 * @date        2026-02-24
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-24, initial version\n
 */

#ifndef ANIMATOR_WLF_HEIGHT_ANIMATOR_H
#define ANIMATOR_WLF_HEIGHT_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_height_animator;

/**
 * @brief Height animator structure.
 *
 * Similar to Qt Quick's property animations for height.
 */
struct wlf_height_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting height */
	float to;                  /**< Ending height */
	float current;             /**< Current height */
	float *target;             /**< Pointer to target height variable */
};

/**
 * @brief Creates a height animator.
 *
 * @param duration Duration in milliseconds.
 * @param from Starting height.
 * @param to Ending height.
 * @param target Pointer to target height variable to animate.
 * @return Pointer to the created animator, or NULL on failure.
 */
struct wlf_height_animator *wlf_height_animator_create(int64_t duration,
	 float from, float to, float *target);

/**
 * @brief Checks whether an animator is a height animator.
 *
 * @param animator Animator instance to check.
 * @return true if the animator is of height animator type, otherwise false.
 */
bool wlf_animator_is_height(const struct wlf_animator *animator);

/**
 * @brief Casts a base animator to a height animator.
 *
 * @param animator Base animator instance.
 * @return Height animator instance if the type matches, otherwise NULL.
 */
struct wlf_height_animator *wlf_height_animator_from_animator(struct wlf_animator *animator);

#endif // ANIMATOR_WLF_HEIGHT_ANIMATOR_H
