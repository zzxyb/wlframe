/**
 * @file        wlf_curve_bounce.h
 * @brief       Bounce easing curves with realistic bouncing effect in wlframe.
 * @details     This file provides bounce easing curve implementations.
 *              Bounce curves simulate the motion of an object bouncing, similar to a ball
 *              dropping and bouncing on the ground with decreasing amplitude.
 *              The motion includes multiple bounces that gradually settle.
 * @author      YaoBing Xiao
 * @date        2026-02-11
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-11, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_BOUNCE_H
#define ANIMATOR_WLF_CURVE_BOUNCE_H

#include "wlf/animator/wlf_curve.h"

#include <stdbool.h>

/**
 * @brief Bounce easing curves with realistic bouncing effect.
 *
 * This structure provides bounce easing curves that simulate bouncing motion.
 * The base member must be the first field to enable safe casting from wlf_curve.
 */
struct wlf_curve_bounce {
	struct wlf_curve base;         /**< Base curve structure */
	enum wlf_curve_type type;      /**< Type of bounce curve */
};

/**
 * @brief Creates an ease-in bounce curve.
 *
 * Creates a curve with a bouncing effect at the start.
 * The motion starts with several bounces that accelerate towards the end position.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_bounce_create(void);

/**
 * @brief Creates an ease-out bounce curve.
 *
 * Creates a curve that bounces at the end.
 * The motion moves towards the target and bounces several times before settling,
 * like a ball dropping and bouncing on the ground.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_bounce_create(void);

/**
 * @brief Creates an ease-in-out bounce curve.
 *
 * Combines bounce ease-in and ease-out effects.
 * Bounces at both the start and end for a symmetric bounce effect.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_out_bounce_create(void);

/**
 * @brief Creates an ease-out-in bounce curve.
 *
 * Reverse of ease-in-out: bounces in the middle.
 * Creates a unique bouncing pattern for special effects.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_in_bounce_create(void);

/**
 * @brief Checks if a curve is a bounce curve.
 *
 * Determines whether the given curve is one of the bounce curve types.
 *
 * @param curve The curve to check.
 * @return true if the curve is a bounce curve (in, out, in-out, or out-in), false otherwise.
 */
bool wlf_curve_is_bounce(const struct wlf_curve *curve);

/**
 * @brief Converts a generic curve to a bounce curve.
 *
 * Safely converts a wlf_curve pointer to a wlf_curve_bounce pointer.
 * This function checks that the curve is actually a bounce curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the bounce curve if successful, or NULL if the curve is not a bounce curve.
 *
 * @code
 * struct wlf_curve *curve = wlf_curve_out_bounce_create();
 * struct wlf_curve_bounce *bounce = wlf_curve_bounce_from_curve(curve);
 * if (bounce) {
 *     // Can now access bounce->type
 * }
 * @endcode
 */
struct wlf_curve_bounce *wlf_curve_bounce_from_curve(
	struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_BOUNCE_H
