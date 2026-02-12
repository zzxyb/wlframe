/**
 * @file        wlf_curve_quint.h
 * @brief       Quintic easing curves with very strong acceleration in wlframe.
 * @details     This file provides quintic easing curve implementations.
 *              Quintic curves use fifth-degree polynomial equations (t^5) to create
 *              very strong acceleration and deceleration effects. The motion follows a
 *              quintic function, providing the most pronounced easing among polynomial
 *              curves, with more dramatic effects than quartic curves.
 * @author      YaoBing Xiao
 * @date        2026-02-13
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-13, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_QUINT_H
#define ANIMATOR_WLF_CURVE_QUINT_H

#include "wlf/animator/wlf_curve.h"

#include <stdbool.h>

/**
 * @brief Quintic easing curves with very strong acceleration.
 *
 * This structure provides quintic easing curves based on quintic polynomial functions.
 * The base member must be the first field to enable safe casting from wlf_curve.
 */
struct wlf_curve_quint {
	struct wlf_curve base;         /**< Base curve structure */
	enum wlf_curve_type type;      /**< Type of quintic curve */
};

/**
 * @brief Creates an ease-in quintic curve.
 *
 * Creates a curve that starts slowly and accelerates following a quintic function.
 * The motion begins very gradually and speeds up with very strong acceleration as it progresses.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_quint_create(void);

/**
 * @brief Creates an ease-out quintic curve.
 *
 * Creates a curve that starts quickly and decelerates following a quintic function.
 * The motion begins fast and slows down smoothly with very strong deceleration.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_quint_create(void);

/**
 * @brief Creates an ease-in-out quintic curve.
 *
 * Combines quintic ease-in and ease-out effects.
 * Starts slowly, accelerates in the middle, then decelerates to a smooth stop.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_out_quint_create(void);

/**
 * @brief Creates an ease-out-in quintic curve.
 *
 * Reverse of ease-in-out: starts fast, slows in the middle, then accelerates again.
 * Creates a unique motion pattern for special effects.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_in_quint_create(void);

/**
 * @brief Checks if a curve is a quintic curve.
 *
 * Determines whether the given curve is one of the quintic curve types.
 *
 * @param curve The curve to check.
 * @return true if the curve is a quintic curve (in, out, in-out, or out-in), false otherwise.
 */
bool wlf_curve_is_quint(const struct wlf_curve *curve);

/**
 * @brief Converts a generic curve to a quintic curve.
 *
 * Safely converts a wlf_curve pointer to a wlf_curve_quint pointer.
 * This function checks that the curve is actually a quintic curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the quintic curve if successful, or NULL if the curve is not a quintic curve.
 *
 * @code
 * struct wlf_curve *curve = wlf_curve_out_quint_create();
 * struct wlf_curve_quint *quint = wlf_curve_quint_from_curve(curve);
 * if (quint) {
 *     // Can now access quint->type
 * }
 * @endcode
 */
struct wlf_curve_quint *wlf_curve_quint_from_curve(
	struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_QUINT_H
