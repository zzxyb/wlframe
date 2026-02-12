/**
 * @file        wlf_curve.h
 * @brief       Easing curve interface and implementation in wlframe.
 * @details     This file provides a general abstraction for easing curves.
 *              It defines the base curve structure along with a virtual interface for evaluating
 *              the curve at a specific time and destroying the curve when it is no longer needed.
 *              Different curve implementations (such as linear, quadratic, cubic, exponential, etc.)
 *              can be provided by implementing the `wlf_curve_impl` structure.
 * @author      YaoBing Xiao
 * @date        2026-01-31
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-31, initial version\n
 */

#ifndef ANIMATOR_WLF_CURVE_H
#define ANIMATOR_WLF_CURVE_H

struct wlf_curve;

/**
 * @brief curve type enumeration.
 *
 * Specifies the type of easing curve.
 */
enum wlf_curve_type {
	WLF_CURVE_IN,      /**< Ease-in curve */
	WLF_CURVE_OUT,     /**< Ease-out curve */
	WLF_CURVE_IN_OUT,  /**< Ease-in-out curve */
	WLF_CURVE_OUT_IN,  /**< Ease-out-in curve */
};

/**
 * @brief Virtual methods for curve operations.
 *
 * This structure defines the interface that curve implementations must provide.
 */
struct wlf_curve_impl {
	/**
	 * @brief Evaluates the curve at time t
	 * @param curve The curve instance
	 * @param t Normalized time value [0.0, 1.0]
	 * @return The eased value
	 */
	float (*value_at)(const struct wlf_curve *curve, float t);

	/**
	 * @brief Destroys the curve
	 * @param curve The curve instance
	 */
	void (*destroy)(struct wlf_curve *curve);
};

/**
 * @brief Base curve structure
 *
 * All curve types must have this structure as their first member.
 */
struct wlf_curve {
	const struct wlf_curve_impl *impl;  /**< Virtual method table */
};

/**
 * @brief Initializes a curve.
 *
 * @param curve Curve to initialize.
 * @param impl Implementation methods for this curve.
 */
void wlf_curve_init(struct wlf_curve *curve,
	const struct wlf_curve_impl *impl);

/**
 * @brief Evaluates the easing curve at time t
 * @param curve The easing curve
 * @param t Normalized time value [0.0, 1.0]
 * @return The eased value
 */
float wlf_curve_value_at(const struct wlf_curve *curve, float t);

/**
 * @brief Destroys the easing curve
 * @param curve The curve to destroy
 */
void wlf_curve_destroy(struct wlf_curve *curve);

#endif // ANIMATOR_WLF_CURVE_H
