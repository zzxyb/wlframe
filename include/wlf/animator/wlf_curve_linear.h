/**
 * @file        wlf_curve_linear.h
 * @brief       Linear curve with constant rate of change in wlframe.
 * @details     This file provides linear curve implementation.
 *              Linear curve provides no easing - the value changes at a constant rate
 *              from start to finish. This creates uniform motion with no acceleration
 *              or deceleration, following the identity function f(t) = t.
 * @author      YaoBing Xiao
 * @date        2026-02-12
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-12, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_LINEAR_H
#define ANIMATOR_WLF_CURVE_LINEAR_H

#include "wlf/animator/wlf_curve.h"

#include <stdbool.h>

/**
 * @brief Linear curve with constant rate of change.
 *
 * This structure provides linear interpolation with no easing effects.
 * The base member must be the first field to enable safe casting from wlf_curve.
 */
struct wlf_curve_linear {
	struct wlf_curve base;         /**< Base curve structure */
};

/**
 * @brief Creates a linear curve.
 *
 * Creates a curve with constant rate of change and no easing.
 * The output value equals the input time parameter (f(t) = t).
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_linear_create(void);

/**
 * @brief Checks if a curve is a linear curve.
 *
 * Determines whether the given curve is a linear curve.
 *
 * @param curve The curve to check.
 * @return true if the curve is a linear curve, false otherwise.
 */
bool wlf_curve_is_linear(const struct wlf_curve *curve);

/**
 * @brief Converts a generic curve to a linear curve.
 *
 * Safely converts a wlf_curve pointer to a wlf_curve_linear pointer.
 * This function checks that the curve is actually a linear curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the linear curve if successful, or NULL if the curve is not a linear curve.
 *
 * @code
 * struct wlf_curve *curve = wlf_curve_linear_create();
 * struct wlf_curve_linear *linear = wlf_curve_linear_from_curve(curve);
 * if (linear) {
 *     // Can now access linear curve members
 * }
 * @endcode
 */
struct wlf_curve_linear *wlf_curve_linear_from_curve(
	struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_LINEAR_H
