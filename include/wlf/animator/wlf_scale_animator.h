/**
 * @file        wlf_scale_animator.h
 * @brief       Scale animator interface in wlframe.
 * @details     This file declares the scale animator, which interpolates
 *              target scale values over a specified duration.
 * @author      YaoBing Xiao
 * @date        2026-02-24
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-24, initial version\n
 */

#ifndef ANIMATOR_WLF_SCALE_ANIMATOR_H
#define ANIMATOR_WLF_SCALE_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_scale_animator;

/**
 * @brief Scale animator structure.
 *
 * Similar to Qt Quick's ScaleAnimator.
 */
struct wlf_scale_animator {
	struct wlf_animator base;      /**< Base animator */

	float from_x;                  /**< Starting X scale factor */
	float to_x;                    /**< Ending X scale factor */
	float from_y;                  /**< Starting Y scale factor */
	float to_y;                    /**< Ending Y scale factor */
	float current_x;               /**< Current X scale factor */
	float current_y;               /**< Current Y scale factor */
	float *target_x;               /**< Pointer to target X scale variable */
	float *target_y;               /**< Pointer to target Y scale variable */
	float origin_x;                /**< Scale origin X (normalized 0.0-1.0) */
	float origin_y;                /**< Scale origin Y (normalized 0.0-1.0) */
};

/**
 * @brief Creates a scale animator (uniform scale).
 *
 * @param duration Duration in milliseconds.
 * @param from Starting scale factor.
 * @param to Ending scale factor.
 * @param target_x Pointer to target X scale variable.
 * @param target_y Pointer to target Y scale variable.
 * @return Pointer to the created animator, or NULL on failure.
 */
struct wlf_scale_animator *wlf_scale_animator_create(int64_t duration,
	 float from, float to, float *target_x, float *target_y);

/**
 * @brief Creates a scale animator with separate X and Y scales.
 *
 * @param duration Duration in milliseconds.
 * @param from_x Starting X scale factor.
 * @param to_x Ending X scale factor.
 * @param from_y Starting Y scale factor.
 * @param to_y Ending Y scale factor.
 * @param target_x Pointer to target X scale variable.
 * @param target_y Pointer to target Y scale variable.
 * @return Pointer to the created animator, or NULL on failure.
 */
struct wlf_scale_animator *wlf_scale_animator_create_xy(int64_t duration,
	 float from_x, float to_x, float from_y, float to_y,
	 float *target_x, float *target_y);

/**
 * @brief Checks whether an animator is a scale animator.
 *
 * @param animator Animator instance to check.
 * @return true if the animator is of scale animator type, otherwise false.
 */
bool wlf_animator_is_scale(const struct wlf_animator *animator);

/**
 * @brief Casts a base animator to a scale animator.
 *
 * @param animator Base animator instance.
 * @return Scale animator instance if the type matches, otherwise NULL.
 */
struct wlf_scale_animator *wlf_scale_animator_from_animator(struct wlf_animator *animator);

/**
 * @brief Sets the scale origin.
 *
 * @param animator Scale animator.
 * @param origin_x Origin X coordinate (normalized 0.0-1.0, default 0.5).
 * @param origin_y Origin Y coordinate (normalized 0.0-1.0, default 0.5).
 */
void wlf_scale_animator_set_origin(struct wlf_scale_animator *animator,
	 float origin_x, float origin_y);

#endif // ANIMATOR_WLF_SCALE_ANIMATOR_H
