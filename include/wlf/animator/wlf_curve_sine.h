/**
 * @file        wlf_curve_sine.h
 * @brief       Sinusoidal easing curves with smooth acceleration in wlframe.
 * @details     This file provides sinusoidal easing curve implementations.
 *              Sine curves use trigonometric sine functions to create smooth
 *              acceleration and deceleration effects. The motion follows a
 *              sinusoidal function, providing gentle and natural easing that
 *              resembles smooth circular motion.
 * @author      YaoBing Xiao
 * @date        2026-02-13
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-13, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_SINE_H
#define ANIMATOR_WLF_CURVE_SINE_H

#include "wlf/animator/wlf_curve.h"

#include <stdbool.h>

/**
 * @brief Sinusoidal easing curves with smooth acceleration.
 *
 * This structure provides sinusoidal easing curves based on sine functions.
 * The base member must be the first field to enable safe casting from wlf_curve.
 */
struct wlf_curve_sine {
	struct wlf_curve base;         /**< Base curve structure */
	enum wlf_curve_type type;      /**< Type of sine curve */
};

/**
 * @brief Creates an ease-in sine curve.
 *
 * Creates a curve that starts slowly and accelerates following a sine function.
 * The motion begins gently and speeds up smoothly with natural acceleration.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_sine_create(void);

/**
 * @brief Creates an ease-out sine curve.
 *
 * Creates a curve that starts quickly and decelerates following a sine function.
 * The motion begins fast and slows down smoothly with gentle deceleration.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_sine_create(void);

/**
 * @brief Creates an ease-in-out sine curve.
 *
 * Combines sine ease-in and ease-out effects.
 * Starts slowly, accelerates in the middle, then decelerates to a smooth stop.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_out_sine_create(void);

/**
 * @brief Creates an ease-out-in sine curve.
 *
 * Reverse of ease-in-out: starts fast, slows in the middle, then accelerates again.
 * Creates a unique motion pattern for special effects.
 *
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_in_sine_create(void);

/**
 * @brief Checks if a curve is a sine curve.
 *
 * Determines whether the given curve is one of the sine curve types.
 *
 * @param curve The curve to check.
 * @return true if the curve is a sine curve (in, out, in-out, or out-in), false otherwise.
 */
bool wlf_curve_is_sine(const struct wlf_curve *curve);

/**
 * @brief Converts a generic curve to a sine curve.
 *
 * Safely converts a wlf_curve pointer to a wlf_curve_sine pointer.
 * This function checks that the curve is actually a sine curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the sine curve if successful, or NULL if the curve is not a sine curve.
 *
 * @code
 * struct wlf_curve *curve = wlf_curve_out_sine_create();
 * struct wlf_curve_sine *sine = wlf_curve_sine_from_curve(curve);
 * if (sine) {
 *     // Can now access sine->type
 * }
 * @endcode
 */
struct wlf_curve_sine *wlf_curve_sine_from_curve(
	struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_SINE_H
