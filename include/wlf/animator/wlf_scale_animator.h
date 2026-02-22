#ifndef ANIMATOR_WLF_SCALE_ANIMATOR_H
#define ANIMATOR_WLF_SCALE_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_scale_animator;

/**
 * Scale animator structure
 * Similar to Qt Quick's ScaleAnimator
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
 * Create a scale animator (uniform scale)
 * @param duration Duration in milliseconds
 * @param from Starting scale factor
 * @param to Ending scale factor
 * @param target_x Pointer to target X scale variable
 * @param target_y Pointer to target Y scale variable
 * @return Pointer to the created animator, or NULL on failure
 */
struct wlf_scale_animator *wlf_scale_animator_create(int64_t duration,
                                                      float from,
                                                      float to,
                                                      float *target_x,
                                                      float *target_y);

/**
 * Create a scale animator with separate X and Y scales
 * @param duration Duration in milliseconds
 * @param from_x Starting X scale factor
 * @param to_x Ending X scale factor
 * @param from_y Starting Y scale factor
 * @param to_y Ending Y scale factor
 * @param target_x Pointer to target X scale variable
 * @param target_y Pointer to target Y scale variable
 * @return Pointer to the created animator, or NULL on failure
 */
struct wlf_scale_animator *wlf_scale_animator_create_xy(int64_t duration,
                                                         float from_x, float to_x,
                                                         float from_y, float to_y,
                                                         float *target_x,
                                                         float *target_y);

/**
 * Destroy a scale animator
 * @param animator The animator to destroy
 */
void wlf_scale_animator_destroy(struct wlf_scale_animator *animator);

/**
 * Set the scale origin
 * @param animator The scale animator
 * @param origin_x Origin X coordinate (normalized 0.0-1.0, default 0.5)
 * @param origin_y Origin Y coordinate (normalized 0.0-1.0, default 0.5)
 */
void wlf_scale_animator_set_origin(struct wlf_scale_animator *animator,
                                    float origin_x, float origin_y);

/**
 * Get the base animator
 * @param animator The scale animator
 * @return Pointer to the base animator
 */
struct wlf_animator *wlf_scale_animator_get_base(struct wlf_scale_animator *animator);

#endif // ANIMATOR_WLF_SCALE_ANIMATOR_H
