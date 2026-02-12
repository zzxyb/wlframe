/**
 * @file        wlf_curve_quart.h
 * @brief       Quartic easing curves with strong acceleration in wlframe.
 * @details     This file provides quartic easing curve implementations.
 *              Quartic curves use fourth-degree polynomial equations (t^4) to create
 *              strong acceleration and deceleration effects. The motion follows a
 *              quartic function, providing more pronounced easing than quadratic or cubic
 *              curves but gentler than quintic curves.
 * @author      YaoBing Xiao
 * @date        2026-02-12
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-12, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_QUART_H
#define ANIMATOR_WLF_CURVE_QUART_H

#include "wlf/animator/wlf_curve.h"

#include <stdbool.h>

/**
 * @brief Quartic easing curves with strong acceleration.
 *
 * This structure provides quartic easing curves based on quartic polynomial functions.
 * The base member must be the first field to enable safe casting from wlf_curve.
 */
struct wlf_curve_quart {
	struct wlf_curve base;         /**< Base curve structure */
	enum wlf_curve_type type;      /**< Type of quartic curve */
};

/**
 * @brief Creates an ease-in quartic curve.
 *
 * Creates a curve that starts slowly and accelerates following a quartic function.
 * The motion begins very gradually and speeds up with strong acceleration as it progresses.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_quart_create(void);

/**
 * @brief Creates an ease-out quartic curve.
 *
 * Creates a curve that starts quickly and decelerates following a quartic function.
 * The motion begins fast and slows down smoothly with strong deceleration.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_quart_create(void);

/**
 * @brief Creates an ease-in-out quartic curve.
 *
 * Combines quartic ease-in and ease-out effects.
 * Starts slowly, accelerates in the middle, then decelerates to a smooth stop.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_out_quart_create(void);

/**
 * @brief Creates an ease-out-in quartic curve.
 *
 * Reverse of ease-in-out: starts fast, slows in the middle, then accelerates again.
 * Creates a unique motion pattern for special effects.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_in_quart_create(void);

/**
 * @brief Checks if a curve is a quartic curve.
 *
 * Determines whether the given curve is one of the quartic curve types.
 *
 * @param curve The curve to check.
 * @return true if the curve is a quartic curve (in, out, in-out, or out-in), false otherwise.
 */
bool wlf_curve_is_quart(const struct wlf_curve *curve);

/**
 * @brief Converts a generic curve to a quartic curve.
 *
 * Safely converts a wlf_curve pointer to a wlf_curve_quart pointer.
 * This function checks that the curve is actually a quartic curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the quartic curve if successful, or NULL if the curve is not a quartic curve.
 *
 * @code
 * struct wlf_curve *curve = wlf_curve_out_quart_create();
 * struct wlf_curve_quart *quart = wlf_curve_quart_from_curve(curve);
 * if (quart) {
 *     // Can now access quart->type
 * }
 * @endcode
 */
struct wlf_curve_quart *wlf_curve_quart_from_curve(
	struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_QUART_H
