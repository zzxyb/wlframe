#ifndef ANIMATOR_WLF_UNIFORM_ANIMATOR_H
#define ANIMATOR_WLF_UNIFORM_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_uniform_animator;

/**
 * Uniform animator structure for animating generic float values
 * Similar to Qt Quick's NumberAnimation
 */
struct wlf_uniform_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting value */
	float to;                  /**< Ending value */
	float current;             /**< Current value */
	float *target;             /**< Pointer to target variable */
};

/**
 * Create a uniform (generic float) animator
 * @param duration Duration in milliseconds
 * @param from Starting value
 * @param to Ending value
 * @param target Pointer to target variable to animate
 * @return Pointer to the created animator, or NULL on failure
 */
struct wlf_uniform_animator *wlf_uniform_animator_create(int64_t duration,
                                                          float from,
                                                          float to,
                                                          float *target);

/**
 * Destroy a uniform animator
 * @param animator The animator to destroy
 */
void wlf_uniform_animator_destroy(struct wlf_uniform_animator *animator);

/**
 * Get the base animator
 * @param animator The uniform animator
 * @return Pointer to the base animator
 */
struct wlf_animator *wlf_uniform_animator_get_base(struct wlf_uniform_animator *animator);

#endif // ANIMATOR_WLF_UNIFORM_ANIMATOR_H
