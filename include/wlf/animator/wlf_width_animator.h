/**
 * @file        wlf_width_animator.h
 * @brief       Width animator interface in wlframe.
 * @details     This file declares the width animator, which interpolates
 *              a target float value from a start width to an end width
 *              over a specified duration.
 * @author      YaoBing Xiao
 * @date        2026-02-24
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-24, initial version\n
 */

#ifndef ANIMATOR_WLF_WIDTH_ANIMATOR_H
#define ANIMATOR_WLF_WIDTH_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_width_animator;

/**
 * @brief Width animator structure.
 *
 * Similar to Qt Quick's property animations for width.
 */
struct wlf_width_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting width */
	float to;                  /**< Ending width */
	float current;             /**< Current width */
	float *target;             /**< Pointer to target width variable */
};

/**
 * @brief Creates a width animator.
 *
 * @param duration Duration in milliseconds.
 * @param from Starting width.
 * @param to Ending width.
 * @param target Pointer to target width variable to animate.
 * @return Pointer to the created animator, or NULL on failure.
 */
struct wlf_width_animator *wlf_width_animator_create(int64_t duration,
	 float from, float to, float *target);

/**
 * @brief Checks whether an animator is a width animator.
 *
 * @param animator Animator instance to check.
 * @return true if the animator is of width animator type, otherwise false.
 */
bool wlf_animator_is_width(const struct wlf_animator *animator);

/**
 * @brief Casts a base animator to a width animator.
 *
 * @param animator Base animator instance.
 * @return Width animator instance if the type matches, otherwise NULL.
 */
struct wlf_width_animator *wlf_width_animator_from_animator(struct wlf_animator *animator);

#endif // ANIMATOR_WLF_WIDTH_ANIMATOR_H
