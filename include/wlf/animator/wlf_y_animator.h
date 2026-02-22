#ifndef ANIMATOR_WLF_Y_ANIMATOR_H
#define ANIMATOR_WLF_Y_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_y_animator;

/**
 * Y position animator structure
 * Similar to Qt Quick's YAnimator
 */
struct wlf_y_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting Y position */
	float to;                  /**< Ending Y position */
	float current;             /**< Current Y position */
	float *target;             /**< Pointer to target Y variable */
};

/**
 * Create a Y position animator
 * @param duration Duration in milliseconds
 * @param from Starting Y position
 * @param to Ending Y position
 * @param target Pointer to target Y variable to animate
 * @return Pointer to the created animator, or NULL on failure
 */
struct wlf_y_animator *wlf_y_animator_create(int64_t duration,
                                              float from,
                                              float to,
                                              float *target);

/**
 * Destroy a Y position animator
 * @param animator The animator to destroy
 */
void wlf_y_animator_destroy(struct wlf_y_animator *animator);

/**
 * Get the base animator
 * @param animator The Y animator
 * @return Pointer to the base animator
 */
struct wlf_animator *wlf_y_animator_get_base(struct wlf_y_animator *animator);

#endif // ANIMATOR_WLF_Y_ANIMATOR_H
