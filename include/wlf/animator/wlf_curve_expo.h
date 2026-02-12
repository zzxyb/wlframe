/**
 * @file        wlf_curve_expo.h
 * @brief       Exponential easing curves with accelerating motion in wlframe.
 * @details     This file provides exponential easing curve implementations.
 *              Exponential curves create dramatic acceleration or deceleration effects
 *              using exponential mathematical functions (2^x). These curves start very
 *              slowly and accelerate rapidly (ease-in), or start very fast and decelerate
 *              dramatically (ease-out), creating powerful and dynamic effects.
 *              The exponential function provides one of the most pronounced easing effects
 *              available in the curve library.
 * @author      YaoBing Xiao
 * @date        2026-02-12
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-12, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_EXPO_H
#define ANIMATOR_WLF_CURVE_EXPO_H

#include "wlf/animator/wlf_curve.h"

#include <stdbool.h>

/**
 * @brief Exponential easing curves with dramatic acceleration/deceleration.
 *
 * This structure provides exponential easing curves based on 2^x function.
 * The base member must be the first field to enable safe casting from wlf_curve.
 */
struct wlf_curve_expo {
	struct wlf_curve base;         /**< Base curve structure */
	enum wlf_curve_type type;      /**< Type of exponential curve */
};

/**
 * @brief Creates an ease-in exponential curve.
 *
 * Creates a curve that starts extremely slowly and accelerates dramatically toward the end.
 * The motion begins with minimal change and then rapidly increases using exponential growth.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_expo_create(void);

/**
 * @brief Creates an ease-out exponential curve.
 *
 * Creates a curve that starts very fast and decelerates dramatically toward the end.
 * The motion begins with rapid change and then slows down exponentially to settle smoothly.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_expo_create(void);

/**
 * @brief Creates an ease-in-out exponential curve.
 *
 * Combines exponential ease-in and ease-out effects.
 * Starts with slow, accelerating motion, reaches peak speed, then decelerates dramatically.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_out_expo_create(void);

/**
 * @brief Creates an ease-out-in exponential curve.
 *
 * Reverse of ease-in-out: starts fast with deceleration, moves smoothly in the middle,
 * then accelerates again toward the end. Creates a unique exponential motion pattern.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_in_expo_create(void);

/**
 * @brief Checks if a curve is an exponential curve.
 *
 * Determines whether the given curve is one of the exponential curve types.
 *
 * @param curve The curve to check.
 * @return true if the curve is an exponential curve (in, out, in-out, or out-in), false otherwise.
 */
bool wlf_curve_is_expo(const struct wlf_curve *curve);

/**
 * @brief Converts a generic curve to an exponential curve.
 *
 * Safely converts a wlf_curve pointer to a wlf_curve_expo pointer.
 * This function checks that the curve is actually an exponential curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the exponential curve if successful, or NULL if the curve is not an exponential curve.
 *
 * @code
 * struct wlf_curve *curve = wlf_curve_out_expo_create();
 * struct wlf_curve_expo *expo = wlf_curve_expo_from_curve(curve);
 * if (expo) {
 *     // Can now access expo->type and other members
 * }
 * @endcode
 */
struct wlf_curve_expo *wlf_curve_expo_from_curve(
	struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_EXPO_H
