/**
 * @file        wlf_curve_circ.h
 * @brief       Circular easing curves with smooth acceleration in wlframe.
 * @details     This file provides circular easing curve implementations.
 *              Circular curves use the equation of a quarter circle to create smooth
 *              acceleration and deceleration effects. The motion follows a circular arc,
 *              creating a gentle start or end compared to polynomial curves.
 * @author      YaoBing Xiao
 * @date        2026-02-11
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-11, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_CIRC_H
#define ANIMATOR_WLF_CURVE_CIRC_H

#include "wlf/animator/wlf_curve.h"

#include <stdbool.h>

/**
 * @brief Circular easing curves with smooth acceleration.
 *
 * This structure provides circular easing curves based on circular arc motion.
 * The base member must be the first field to enable safe casting from wlf_curve.
 */
struct wlf_curve_circ {
	struct wlf_curve base;         /**< Base curve structure */
	enum wlf_curve_type type;      /**< Type of circular curve */
};

/**
 * @brief Creates an ease-in circular curve.
 *
 * Creates a curve that starts slowly and accelerates following a circular arc.
 * The motion begins very gradually and speeds up as it progresses.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_circ_create(void);

/**
 * @brief Creates an ease-out circular curve.
 *
 * Creates a curve that starts quickly and decelerates following a circular arc.
 * The motion begins fast and slows down gradually to a gentle stop.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_circ_create(void);

/**
 * @brief Creates an ease-in-out circular curve.
 *
 * Combines circular ease-in and ease-out effects.
 * Starts slowly, accelerates in the middle, then decelerates to a smooth stop.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_out_circ_create(void);

/**
 * @brief Creates an ease-out-in circular curve.
 *
 * Reverse of ease-in-out: starts fast, slows in the middle, then accelerates again.
 * Creates a unique motion pattern for special effects.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_in_circ_create(void);

/**
 * @brief Checks if a curve is a circular curve.
 *
 * Determines whether the given curve is one of the circular curve types.
 *
 * @param curve The curve to check.
 * @return true if the curve is a circular curve (in, out, in-out, or out-in), false otherwise.
 */
bool wlf_curve_is_circ(const struct wlf_curve *curve);

/**
 * @brief Converts a generic curve to a circular curve.
 *
 * Safely converts a wlf_curve pointer to a wlf_curve_circ pointer.
 * This function checks that the curve is actually a circular curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the circular curve if successful, or NULL if the curve is not a circular curve.
 *
 * @code
 * struct wlf_curve *curve = wlf_curve_out_circ_create();
 * struct wlf_curve_circ *circ = wlf_curve_circ_from_curve(curve);
 * if (circ) {
 *     // Can now access circ->type
 * }
 * @endcode
 */
struct wlf_curve_circ *wlf_curve_circ_from_curve(
	struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_CIRC_H
