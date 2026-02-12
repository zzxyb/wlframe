/**
 * @file        wlf_curve_cubic.h
 * @brief       Cubic easing curves with smooth acceleration in wlframe.
 * @details     This file provides cubic easing curve implementations.
 *              Cubic curves use third-degree polynomial equations (t^3) to create
 *              smooth acceleration and deceleration effects. The motion follows a
 *              cubic function, providing more pronounced easing compared to quadratic
 *              curves but gentler than quartic or quintic curves.
 * @author      YaoBing Xiao
 * @date        2026-02-12
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-12, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_CUBIC_H
#define ANIMATOR_WLF_CURVE_CUBIC_H

#include "wlf/animator/wlf_curve.h"

#include <stdbool.h>

/**
 * @brief Cubic easing curves with smooth acceleration.
 *
 * This structure provides cubic easing curves based on cubic polynomial functions.
 * The base member must be the first field to enable safe casting from wlf_curve.
 */
struct wlf_curve_cubic {
	struct wlf_curve base;         /**< Base curve structure */
	enum wlf_curve_type type;      /**< Type of cubic curve */
};

/**
 * @brief Creates an ease-in cubic curve.
 *
 * Creates a curve that starts slowly and accelerates following a cubic function.
 * The motion begins gradually and speeds up with increasing rate as it progresses.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_cubic_create(void);

/**
 * @brief Creates an ease-out cubic curve.
 *
 * Creates a curve that starts quickly and decelerates following a cubic function.
 * The motion begins fast and slows down gradually to a smooth stop.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_cubic_create(void);

/**
 * @brief Creates an ease-in-out cubic curve.
 *
 * Combines cubic ease-in and ease-out effects.
 * Starts slowly, accelerates in the middle, then decelerates to a smooth stop.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_out_cubic_create(void);

/**
 * @brief Creates an ease-out-in cubic curve.
 *
 * Reverse of ease-in-out: starts fast, slows in the middle, then accelerates again.
 * Creates a unique motion pattern for special effects.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_in_cubic_create(void);

/**
 * @brief Checks if a curve is a cubic curve.
 *
 * Determines whether the given curve is one of the cubic curve types.
 *
 * @param curve The curve to check.
 * @return true if the curve is a cubic curve (in, out, in-out, or out-in), false otherwise.
 */
bool wlf_curve_is_cubic(const struct wlf_curve *curve);

/**
 * @brief Converts a generic curve to a cubic curve.
 *
 * Safely converts a wlf_curve pointer to a wlf_curve_cubic pointer.
 * This function checks that the curve is actually a cubic curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the cubic curve if successful, or NULL if the curve is not a cubic curve.
 *
 * @code
 * struct wlf_curve *curve = wlf_curve_out_cubic_create();
 * struct wlf_curve_cubic *cubic = wlf_curve_cubic_from_curve(curve);
 * if (cubic) {
 *     // Can now access cubic->type
 * }
 * @endcode
 */
struct wlf_curve_cubic *wlf_curve_cubic_from_curve(
	struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_CUBIC_H
