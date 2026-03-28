/**
 * @file        wlf_radial_gradient.h
 * @brief       Radial gradient implementation for wlframe.
 * @author      YaoBing Xiao
 * @date        2026-03-21
 * @version     v1.0
 */

#ifndef TYPES_WLF_RADIAL_GRADIENT_H
#define TYPES_WLF_RADIAL_GRADIENT_H

#include "wlf/types/wlf_gradient.h"

/**
 * @brief Radial gradient definition.
 */
struct wlf_radial_gradient {
	struct wlf_gradient base;   /**< Base gradient */
	struct wlf_fpoint center;   /**< Circle center */
	struct wlf_fpoint focal;    /**< Focal point */
	double radius;              /**< Circle radius */
};

/**
 * @brief Creates a radial gradient.
 * @param center Circle center.
 * @param focal Focal point.
 * @param radius Circle radius.
 * @param stops Color stop array.
 * @param stop_count Number of stops.
 * @return Newly allocated radial gradient or NULL on failure.
 */
struct wlf_radial_gradient *wlf_radial_gradient_create(
	struct wlf_fpoint center, struct wlf_fpoint focal, double radius,
	const struct wlf_gradient_stop *stops, size_t stop_count);

/**
 * @brief Checks whether a gradient is a radial gradient.
 * @param gradient Gradient to test.
 * @return true if the gradient is a radial gradient, false otherwise.
 */
bool wlf_gradient_is_radial(const struct wlf_gradient *gradient);

/**
 * @brief Obtains the radial gradient from a base gradient pointer.
 * @param gradient Base gradient pointer. Must be a radial gradient; passing any
 *   other gradient type triggers an assertion failure.
 * @return The enclosing wlf_radial_gradient.
 */
struct wlf_radial_gradient *wlf_radial_gradient_from_gradient(
	struct wlf_gradient *gradient);

#endif // TYPES_WLF_RADIAL_GRADIENT_H
