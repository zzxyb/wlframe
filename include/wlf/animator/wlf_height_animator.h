#ifndef ANIMATOR_WLF_HEIGHT_ANIMATOR_H
#define ANIMATOR_WLF_HEIGHT_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_height_animator;

/**
 * Height animator structure
 * Similar to Qt Quick's property animations for height
 */
struct wlf_height_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting height */
	float to;                  /**< Ending height */
	float current;             /**< Current height */
	float *target;             /**< Pointer to target height variable */
};

/**
 * Create a height animator
 * @param duration Duration in milliseconds
 * @param from Starting height
 * @param to Ending height
 * @param target Pointer to target height variable to animate
 * @return Pointer to the created animator, or NULL on failure
 */
struct wlf_height_animator *wlf_height_animator_create(int64_t duration,
                                                        float from,
                                                        float to,
                                                        float *target);

/**
 * Destroy a height animator
 * @param animator The animator to destroy
 */
void wlf_height_animator_destroy(struct wlf_height_animator *animator);

/**
 * Get the base animator
 * @param animator The height animator
 * @return Pointer to the base animator
 */
struct wlf_animator *wlf_height_animator_get_base(struct wlf_height_animator *animator);

#endif // ANIMATOR_WLF_HEIGHT_ANIMATOR_H
