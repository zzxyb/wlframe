/**
 * @file        wlf_curve_elastic.h
 * @brief       Elastic easing curves with spring-like oscillation in wlframe.
 * @details     This file provides elastic easing curve implementations.
 *              Elastic curves simulate spring-like motion with oscillation effects,
 *              creating bouncing or wobbling motion. The motion overshoots the target
 *              and oscillates back and forth before settling, similar to a spring or
 *              elastic band. The amplitude and period parameters control the intensity
 *              and frequency of the oscillation.
 * @author      YaoBing Xiao
 * @date        2026-02-12
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-12, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_ELASTIC_H
#define ANIMATOR_WLF_CURVE_ELASTIC_H

#include "wlf/animator/wlf_curve.h"

#include <stdbool.h>

/**
 * @brief Elastic easing curves with spring-like oscillation.
 *
 * This structure provides elastic easing curves with customizable
 * amplitude and period parameters. The base member must be the first field
 * to enable safe casting from wlf_curve.
 */
struct wlf_curve_elastic {
	struct wlf_curve base;         /**< Base curve structure */
	enum wlf_curve_type type;      /**< Type of elastic curve */
	float amplitude;               /**< Amplitude of oscillation (controls overshoot intensity) */
	float period;                  /**< Period of oscillation (controls oscillation frequency) */
};

/**
 * @brief Creates an ease-in elastic curve.
 *
 * Creates a curve that starts with small oscillations that grow in intensity.
 * The motion begins with subtle spring-like wobbles that increase in intensity.
 *
 * @param amplitude The amplitude of the elastic effect (controls overshoot intensity).
 *                  Higher values create more pronounced overshooting.
 * @param period The period of oscillation (controls frequency).
 *               Smaller values create faster oscillations.
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_elastic_create(float amplitude, float period);

/**
 * @brief Creates an ease-out elastic curve.
 *
 * Creates a curve that ends with spring-like oscillations around the target value.
 * The motion overshoots the target and bounces back several times before settling.
 *
 * @param amplitude The amplitude of the elastic effect (controls overshoot intensity).
 *                  Higher values create more pronounced overshooting.
 * @param period The period of oscillation (controls frequency).
 *               Smaller values create faster oscillations.
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_elastic_create(float amplitude, float period);

/**
 * @brief Creates an ease-in-out elastic curve.
 *
 * Combines elastic ease-in and ease-out effects.
 * Starts with growing oscillations, reaches the target, then settles with dampening oscillations.
 *
 * @param amplitude The amplitude of the elastic effect (controls overshoot intensity).
 *                  Higher values create more pronounced overshooting.
 * @param period The period of oscillation (controls frequency).
 *               Smaller values create faster oscillations.
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_in_out_elastic_create(float amplitude, float period);

/**
 * @brief Creates an ease-out-in elastic curve.
 *
 * Reverse of ease-in-out: starts with oscillations, moves smoothly in the middle,
 * then ends with oscillations. Creates a unique elastic motion pattern.
 *
 * @param amplitude The amplitude of the elastic effect (controls overshoot intensity).
 *                  Higher values create more pronounced overshooting.
 * @param period The period of oscillation (controls frequency).
 *               Smaller values create faster oscillations.
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_curve *wlf_curve_out_in_elastic_create(float amplitude, float period);

/**
 * @brief Checks if a curve is an elastic curve.
 *
 * Determines whether the given curve is one of the elastic curve types.
 *
 * @param curve The curve to check.
 * @return true if the curve is an elastic curve (in, out, in-out, or out-in), false otherwise.
 */
bool wlf_curve_is_elastic(const struct wlf_curve *curve);

/**
 * @brief Converts a generic curve to an elastic curve.
 *
 * Safely converts a wlf_curve pointer to a wlf_curve_elastic pointer.
 * This function checks that the curve is actually an elastic curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the elastic curve if successful, or NULL if the curve is not an elastic curve.
 *
 * @code
 * struct wlf_curve *curve = wlf_curve_out_elastic_create(1.0f, 0.3f);
 * struct wlf_curve_elastic *elastic = wlf_curve_elastic_from_curve(curve);
 * if (elastic) {
 *     // Can now access elastic->amplitude and elastic->period
 * }
 * @endcode
 */
struct wlf_curve_elastic *wlf_curve_elastic_from_curve(
	struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_ELASTIC_H
