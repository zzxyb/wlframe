/**
 * @file        wlf_curve_quad.h
 * @brief       Quadratic easing curves with gentle acceleration in wlframe.
 * @details     This file provides quadratic easing curve implementations.
 *              Quadratic curves use second-degree polynomial equations (t^2) to create
 *              gentle acceleration and deceleration effects. The motion follows a
 *              quadratic function, providing smooth easing that is more subtle than
 *              higher-order polynomial curves like cubic or quartic.
 * @author      YaoBing Xiao
 * @date        2026-02-12
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-12, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_QUAD_H
#define ANIMATOR_WLF_CURVE_QUAD_H

#include "wlf/animator/wlf_curve.h"

#include <stdbool.h>

/**
 * @brief Quadratic easing curves with gentle acceleration.
 *
 * This structure provides quadratic easing curves based on quadratic polynomial functions.
 * The base member must be the first field to enable safe casting from wlf_curve.
 */
struct wlf_curve_quad {
	struct wlf_curve base;         /**< Base curve structure */
	enum wlf_curve_type type;      /**< Type of quadratic curve */
};

/**
 * @brief Creates an ease-in quadratic curve.
 *
 * Creates a curve that starts slowly and accelerates following a quadratic function.
 * The motion begins gradually and speeds up smoothly as it progresses.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_quad_create(void);

/**
 * @brief Creates an ease-out quadratic curve.
 *
 * Creates a curve that starts quickly and decelerates following a quadratic function.
 * The motion begins fast and slows down smoothly to a gentle stop.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_quad_create(void);

/**
 * @brief Creates an ease-in-out quadratic curve.
 *
 * Combines quadratic ease-in and ease-out effects.
 * Starts slowly, accelerates in the middle, then decelerates to a smooth stop.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_out_quad_create(void);

/**
 * @brief Creates an ease-out-in quadratic curve.
 *
 * Reverse of ease-in-out: starts fast, slows in the middle, then accelerates again.
 * Creates a unique motion pattern for special effects.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_in_quad_create(void);

/**
 * @brief Checks if a curve is a quadratic curve.
 *
 * Determines whether the given curve is one of the quadratic curve types.
 *
 * @param curve The curve to check.
 * @return true if the curve is a quadratic curve (in, out, in-out, or out-in), false otherwise.
 */
bool wlf_curve_is_quad(const struct wlf_curve *curve);

/**
 * @brief Converts a generic curve to a quadratic curve.
 *
 * Safely converts a wlf_curve pointer to a wlf_curve_quad pointer.
 * This function checks that the curve is actually a quadratic curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the quadratic curve if successful, or NULL if the curve is not a quadratic curve.
 *
 * @code
 * struct wlf_curve *curve = wlf_curve_out_quad_create();
 * struct wlf_curve_quad *quad = wlf_curve_quad_from_curve(curve);
 * if (quad) {
 *     // Can now access quad->type
 * }
 * @endcode
 */
struct wlf_curve_quad *wlf_curve_quad_from_curve(
	struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_QUAD_H
