/**
 * @file        wlf_opacity_animator.h
 * @brief       Opacity animator interface in wlframe.
 * @details     This file declares the opacity animator, which interpolates
 *              a target opacity value from a start value to an end value
 *              over a specified duration.
 * @author      YaoBing Xiao
 * @date        2026-02-24
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-24, initial version\n
 */

#ifndef ANIMATOR_WLF_OPACITY_ANIMATOR_H
#define ANIMATOR_WLF_OPACITY_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_opacity_animator;

/**
 * @brief Opacity animator structure.
 *
 * Animates opacity values and is similar to Qt Quick's OpacityAnimator.
 */
struct wlf_opacity_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting opacity value [0.0, 1.0] */
	float to;                  /**< Ending opacity value [0.0, 1.0] */
	float current;             /**< Current opacity value */
	float *target;             /**< Pointer to target opacity variable */
};

/**
 * @brief Creates an opacity animator.
 *
 * @param duration Duration in milliseconds.
 * @param from Starting opacity.
 * @param to Ending opacity.
 * @param target Pointer to target opacity variable to animate.
 * @return Pointer to the created animator, or NULL on failure.
 */
struct wlf_opacity_animator *wlf_opacity_animator_create(int64_t duration,
	 float from, float to, float *target);

/**
 * @brief Checks whether an animator is an opacity animator.
 *
 * @param animator Animator instance to check.
 * @return true if the animator is of opacity animator type, otherwise false.
 */
bool wlf_animator_is_opacity(const struct wlf_animator *animator);

/**
 * @brief Casts a base animator to an opacity animator.
 *
 * @param animator Base animator instance.
 * @return Opacity animator instance if the type matches, otherwise NULL.
 */
struct wlf_opacity_animator *wlf_opacity_animator_from_animator(struct wlf_animator *animator);

#endif // ANIMATOR_WLF_OPACITY_ANIMATOR_H
