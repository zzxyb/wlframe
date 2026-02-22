#ifndef ANIMATOR_WLF_WIDTH_ANIMATOR_H
#define ANIMATOR_WLF_WIDTH_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_width_animator;

/**
 * Width animator structure
 * Similar to Qt Quick's property animations for width
 */
struct wlf_width_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting width */
	float to;                  /**< Ending width */
	float current;             /**< Current width */
	float *target;             /**< Pointer to target width variable */
};

/**
 * Create a width animator
 * @param duration Duration in milliseconds
 * @param from Starting width
 * @param to Ending width
 * @param target Pointer to target width variable to animate
 * @return Pointer to the created animator, or NULL on failure
 */
struct wlf_width_animator *wlf_width_animator_create(int64_t duration,
                                                      float from,
                                                      float to,
                                                      float *target);

/**
 * Destroy a width animator
 * @param animator The animator to destroy
 */
void wlf_width_animator_destroy(struct wlf_width_animator *animator);

/**
 * Get the base animator
 * @param animator The width animator
 * @return Pointer to the base animator
 */
struct wlf_animator *wlf_width_animator_get_base(struct wlf_width_animator *animator);

#endif // ANIMATOR_WLF_WIDTH_ANIMATOR_H
