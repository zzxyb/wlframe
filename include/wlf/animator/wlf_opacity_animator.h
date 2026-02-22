#ifndef ANIMATOR_WLF_OPACITY_ANIMATOR_H
#define ANIMATOR_WLF_OPACITY_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_opacity_animator;

/**
 * Opacity animator structure (animates opacity from 0.0 to 1.0)
 * Similar to Qt Quick's OpacityAnimator
 */
struct wlf_opacity_animator {
	struct wlf_animator base;  /**< Base animator */

	float from;                /**< Starting opacity value [0.0, 1.0] */
	float to;                  /**< Ending opacity value [0.0, 1.0] */
	float current;             /**< Current opacity value */
	float *target;             /**< Pointer to target opacity variable */
};

/**
 * Create an opacity animator
 * @param duration Duration in milliseconds
 * @param from Starting opacity
 * @param to Ending opacity
 * @param target Pointer to target opacity variable to animate
 * @return Pointer to the created animator, or NULL on failure
 */
struct wlf_opacity_animator *wlf_opacity_animator_create(int64_t duration,
                                                          float from,
                                                          float to,
                                                          float *target);

/**
 * Destroy an opacity animator
 * @param animator The animator to destroy
 */
void wlf_opacity_animator_destroy(struct wlf_opacity_animator *animator);

/**
 * Get the base animator
 * @param animator The opacity animator
 * @return Pointer to the base animator
 */
struct wlf_animator *wlf_opacity_animator_get_base(struct wlf_opacity_animator *animator);

#endif // ANIMATOR_WLF_OPACITY_ANIMATOR_H
