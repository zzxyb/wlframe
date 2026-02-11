/**
 * @file        wlf_animator_curve.h
 * @brief       Easing curve interface and implementation for animation in wlframe.
 * @details     This file provides an abstraction for animator curves, which are used in the easing of animations.
 *              It defines the base structure for animator curves, along with a virtual interface for evaluating 
 *              the curve at a specific time and destroying the curve when it is no longer needed. 
 *              Curves are used to calculate eased values over time during animations, and different implementations 
 *              of easing curves (such as linear, quadratic, cubic, etc.) can be provided by implementing the 
 *              `wlf_animator_curve_impl` structure.
 * @author      YaoBing Xiao
 * @date        2026-01-31
 * @version     v1.0
 * @par Copyright(c): 
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-31, initial version\n
 */

#ifndef ANIMATOR_WLF_ANIMATOR_CURVE_H
#define ANIMATOR_WLF_ANIMATOR_CURVE_H

struct wlf_animator_curve;

/**
 * @brief curve type enumeration.
 *
 * Specifies the type of easing curve.
 */
enum wlf_animator_curve_type {
	WLF_ANIMATOR_CURVE_IN,      /**< Ease-in curve */
	WLF_ANIMATOR_CURVE_OUT,     /**< Ease-out curve */
	WLF_ANIMATOR_CURVE_IN_OUT,  /**< Ease-in-out curve */
	WLF_ANIMATOR_CURVE_OUT_IN,  /**< Ease-out-in curve */
};

/**
 * @brief Virtual methods for animator curve operations.
 *
 * This structure defines the interface that curve implementations must provide.
 */
struct wlf_animator_curve_impl {
	/**
	 * @brief Evaluates the curve at time t
	 * @param curve The curve instance
	 * @param t Normalized time value [0.0, 1.0]
	 * @return The eased value
	 */
	float (*value_at)(const struct wlf_animator_curve *curve, float t);

	/**
	 * @brief Destroys the curve
	 * @param curve The curve instance
	 */
	void (*destroy)(struct wlf_animator_curve *curve);
};

/**
 * @brief Base animator curve structure
 *
 * All curve types must have this structure as their first member.
 */
struct wlf_animator_curve {
	const struct wlf_animator_curve_impl *impl;  /**< Virtual method table */
};

/**
 * @brief Initializes an animator curve.
 *
 * @param curve Curve to initialize.
 * @param impl Implementation methods for this curve.
 */
void wlf_animator_curve_init(struct wlf_animator_curve *curve,
	const struct wlf_animator_curve_impl *impl);

/**
 * @brief Evaluates the easing curve at time t
 * @param curve The easing curve
 * @param t Normalized time value [0.0, 1.0]
 * @return The eased value
 */
float wlf_animator_curve_value_at(const struct wlf_animator_curve *curve, float t);

/**
 * @brief Destroys the easing curve
 * @param curve The curve to destroy
 */
void wlf_animator_curve_destroy(struct wlf_animator_curve *curve);

#endif // ANIMATOR_WLF_ANIMATOR_CURVE_H
