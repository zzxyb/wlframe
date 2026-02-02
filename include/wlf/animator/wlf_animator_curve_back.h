/**
 * @file        wlf_animator_curve_back.h
 * @brief       Back easing curves with configurable overshoot for animation in wlframe.
 * @details     This file provides back easing curve implementations for animations.
 *              Back curves create a "wind-up" or "overshoot" effect by pulling back before
 *              moving forward (ease-in) or going past the target before settling (ease-out).
 *              The overshoot parameter controls the magnitude of this effect.
 *              Typical overshoot value is 1.70158 for standard back easing.
 * @author      YaoBing Xiao
 * @date        2026-02-02
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-02, initial version\n
 */

#ifndef ANIMATOR_WLF_ANIMATOR_CURVE_BACK_H
#define ANIMATOR_WLF_ANIMATOR_CURVE_BACK_H

#include <stdbool.h>

#include "wlf/animator/wlf_animator_curve.h"

/**
 * @brief Back curve type enumeration.
 *
 * Specifies the type of back easing curve.
 */
enum wlf_animator_curve_back_type {
	WLF_ANIMATOR_CURVE_BACK_IN,      /**< Ease-in back curve */
	WLF_ANIMATOR_CURVE_BACK_OUT,     /**< Ease-out back curve */
	WLF_ANIMATOR_CURVE_BACK_IN_OUT,  /**< Ease-in-out back curve */
	WLF_ANIMATOR_CURVE_BACK_OUT_IN,  /**< Ease-out-in back curve */
};

/**
 * @brief Back easing curves with configurable overshoot.
 *
 * This structure provides back easing animation curves that create an overshoot effect.
 * The base member must be the first field to enable safe casting from wlf_animator_curve.
 */
struct wlf_animator_curve_back {
	struct wlf_animator_curve base;              /**< Base curve structure */
	enum wlf_animator_curve_back_type type;      /**< Type of back curve */
	float overshoot;                             /**< Overshoot parameter controlling the magnitude of back/overshoot effect */
};

/**
 * @brief Creates an ease-in back curve.
 *
 * Creates a curve with a "wind-up" effect that pulls back before moving forward.
 * The motion goes backward initially, then accelerates past the start point.
 *
 * @param overshoot The overshoot parameter controlling how far back the motion pulls.
 *                  Typical value: 1.70158 for standard back easing.
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_animator_curve *wlf_animator_curve_in_back_create(float overshoot);

/**
 * @brief Creates an ease-out back curve.
 *
 * Creates a curve that overshoots the target then settles back.
 * The motion goes past the target, then pulls back to the final position.
 *
 * @param overshoot The overshoot parameter controlling how far past the target the motion goes.
 *                  Typical value: 1.70158 for standard back easing.
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_animator_curve *wlf_animator_curve_out_back_create(float overshoot);

/**
 * @brief Creates an ease-in-out back curve.
 *
 * Combines back ease-in and ease-out effects.
 * Pulls back at the start, overshoots in the middle, then settles at the end.
 *
 * @param overshoot The overshoot parameter controlling the amount of back/overshoot.
 *                  Typical value: 1.70158 for standard back easing.
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_animator_curve *wlf_animator_curve_in_out_back_create(float overshoot);

/**
 * @brief Creates an ease-out-in back curve.
 *
 * Reverse of ease-in-out: overshoots at the start, pulls back in the middle.
 * Creates a unique motion pattern for special animation effects.
 *
 * @param overshoot The overshoot parameter controlling the amount of back/overshoot.
 *                  Typical value: 1.70158 for standard back easing.
 * @return Pointer to the created curve, or NULL on allocation failure.
 */
struct wlf_animator_curve *wlf_animator_curve_out_in_back_create(float overshoot);

/**
 * @brief Checks if a curve is a back curve.
 *
 * Determines whether the given curve is one of the back curve types.
 *
 * @param curve The curve to check.
 * @return true if the curve is a back curve (in, out, in-out, or out-in), false otherwise.
 */
bool wlf_animator_curve_is_back(const struct wlf_animator_curve *curve);

/**
 * @brief Converts a generic curve to a back curve.
 *
 * Safely converts a wlf_animator_curve pointer to a wlf_animator_curve_back pointer.
 * This function checks that the curve is actually a back curve before converting.
 *
 * @param curve The curve to convert.
 * @return Pointer to the back curve if successful, or NULL if the curve is not a back curve.
 *
 * @code
 * struct wlf_animator_curve *curve = wlf_animator_curve_out_back_create(1.70158f);
 * struct wlf_animator_curve_back *back = wlf_animator_curve_back_from_curve(curve);
 * if (back) {
 *     // Can now access back->overshoot, back->type
 * }
 * @endcode
 */
struct wlf_animator_curve_back *wlf_animator_curve_back_from_curve(
	struct wlf_animator_curve *curve);

#endif // ANIMATOR_WLF_ANIMATOR_CURVE_BACK_H
