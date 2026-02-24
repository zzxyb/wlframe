/**
 * @file        wlf_uniform_animator.h
 * @brief       Uniform animator interface in wlframe.
 * @details     This file declares the uniform animator, which interpolates
 *              a generic float target value from a start value to an end value
 *              over a specified duration.
 * @author      YaoBing Xiao
 * @date        2026-02-24
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-24, initial version\n
 */

#ifndef ANIMATOR_WLF_UNIFORM_ANIMATOR_H
#define ANIMATOR_WLF_UNIFORM_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_uniform_animator;

/**
 * @brief Uniform animator structure.
 *
 * Animates generic float values and is similar to Qt Quick's NumberAnimation.
 */
struct wlf_uniform_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting value */
	float to;                  /**< Ending value */
	float current;             /**< Current value */
	float *target;             /**< Pointer to target variable */
};

/**
 * @brief Creates a uniform (generic float) animator.
 *
 * @param duration Duration in milliseconds.
 * @param from Starting value.
 * @param to Ending value.
 * @param target Pointer to target variable to animate.
 * @return Pointer to the created animator, or NULL on failure.
 */
struct wlf_uniform_animator *wlf_uniform_animator_create(int64_t duration,
	 float from, float to, float *target);

/**
 * @brief Checks whether an animator is a uniform animator.
 *
 * @param animator Animator instance to check.
 * @return true if the animator is of uniform animator type, otherwise false.
 */
bool wlf_animator_is_uniform(const struct wlf_animator *animator);

/**
 * @brief Casts a base animator to a uniform animator.
 *
 * @param animator Base animator instance.
 * @return Uniform animator instance if the type matches, otherwise NULL.
 */
struct wlf_uniform_animator *wlf_uniform_animator_from_animator(struct wlf_animator *animator);

#endif // ANIMATOR_WLF_UNIFORM_ANIMATOR_H
