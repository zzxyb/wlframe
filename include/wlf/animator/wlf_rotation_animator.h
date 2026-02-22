#ifndef ANIMATOR_WLF_ROTATION_ANIMATOR_H
#define ANIMATOR_WLF_ROTATION_ANIMATOR_H

#include "wlf/animator/wlf_animator.h"

struct wlf_rotation_animator;

/**
 * Rotation axis enumeration
 */
enum wlf_rotation_axis {
	WLF_ROTATION_AXIS_X,  /**< Rotate around X axis */
	WLF_ROTATION_AXIS_Y,  /**< Rotate around Y axis */
	WLF_ROTATION_AXIS_Z,  /**< Rotate around Z axis (2D rotation) */
};

/**
 * Rotation animator structure
 * Similar to Qt Quick's RotationAnimator
 */
struct wlf_rotation_animator {
	struct wlf_animator base;           /**< Base animator */

	float from;                         /**< Starting angle in degrees */
	float to;                           /**< Ending angle in degrees */
	float current;                      /**< Current angle in degrees */
	float *target;                      /**< Pointer to target angle variable */
	enum wlf_rotation_axis axis;        /**< Rotation axis */
	float origin_x;                     /**< Rotation origin X (normalized 0.0-1.0) */
	float origin_y;                     /**< Rotation origin Y (normalized 0.0-1.0) */
};

/**
 * Create a rotation animator
 * @param duration Duration in milliseconds
 * @param from Starting angle in degrees
 * @param to Ending angle in degrees
 * @param target Pointer to target angle variable to animate
 * @return Pointer to the created animator, or NULL on failure
 */
struct wlf_rotation_animator *wlf_rotation_animator_create(int64_t duration,
                                                            float from,
                                                            float to,
                                                            float *target);

/**
 * Destroy a rotation animator
 * @param animator The animator to destroy
 */
void wlf_rotation_animator_destroy(struct wlf_rotation_animator *animator);

/**
 * Set the rotation axis
 * @param animator The rotation animator
 * @param axis The rotation axis
 */
void wlf_rotation_animator_set_axis(struct wlf_rotation_animator *animator,
                                     enum wlf_rotation_axis axis);

/**
 * Set the rotation origin
 * @param animator The rotation animator
 * @param origin_x Origin X coordinate (normalized 0.0-1.0, default 0.5)
 * @param origin_y Origin Y coordinate (normalized 0.0-1.0, default 0.5)
 */
void wlf_rotation_animator_set_origin(struct wlf_rotation_animator *animator,
                                       float origin_x, float origin_y);

/**
 * Get the base animator
 * @param animator The rotation animator
 * @return Pointer to the base animator
 */
struct wlf_animator *wlf_rotation_animator_get_base(struct wlf_rotation_animator *animator);

#endif // ANIMATOR_WLF_ROTATION_ANIMATOR_H
