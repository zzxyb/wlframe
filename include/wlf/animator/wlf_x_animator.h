#ifndef ANIMATOR_WLF_X_ANIMATOR_H
#define ANIMATOR_WLF_X_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_x_animator;

/**
 * X position animator structure
 * Similar to Qt Quick's XAnimator
 */
struct wlf_x_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting X position */
	float to;                  /**< Ending X position */
	float current;             /**< Current X position */
	float *target;             /**< Pointer to target X variable */
};

/**
 * Create an X position animator
 * @param duration Duration in milliseconds
 * @param from Starting X position
 * @param to Ending X position
 * @param target Pointer to target X variable to animate
 * @return Pointer to the created animator, or NULL on failure
 */
struct wlf_x_animator *wlf_x_animator_create(int64_t duration,
	float from, float to, float *target);

bool wlf_animator_is_x(const struct wlf_animator *animator);

struct wlf_x_animator *wlf_x_animator_from_animator(struct wlf_animator *animator);

#endif // ANIMATOR_WLF_X_ANIMATOR_H
