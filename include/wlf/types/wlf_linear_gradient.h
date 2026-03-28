/**
 * @file        wlf_linear_gradient.h
 * @brief       Linear gradient implementation for wlframe.
 * @author      YaoBing Xiao
 * @date        2026-03-21
 * @version     v1.0
 */

#ifndef TYPES_WLF_LINEAR_GRADIENT_H
#define TYPES_WLF_LINEAR_GRADIENT_H

#include "wlf/types/wlf_gradient.h"

/**
 * @brief Linear gradient definition.
 */
struct wlf_linear_gradient {
	struct wlf_gradient base;   /**< Base gradient */
	struct wlf_fpoint start;    /**< Start point */
	struct wlf_fpoint end;      /**< End point */
};

/**
 * @brief Creates a linear gradient.
 * @param start Start point.
 * @param end End point.
 * @param stops Color stop array.
 * @param stop_count Number of stops.
 * @return Newly allocated linear gradient or NULL on failure.
 */
struct wlf_linear_gradient *wlf_linear_gradient_create(
	struct wlf_fpoint start, struct wlf_fpoint end,
	const struct wlf_gradient_stop *stops, size_t stop_count);

/**
 * @brief Checks whether a gradient is a linear gradient.
 * @param gradient Gradient to test.
 * @return true if the gradient is a linear gradient, false otherwise.
 */
bool wlf_gradient_is_linear(const struct wlf_gradient *gradient);

/**
 * @brief Obtains the linear gradient from a base gradient pointer.
 * @param gradient Base gradient pointer. Must be a linear gradient; passing any
 *   other gradient type triggers an assertion failure.
 * @return The enclosing wlf_linear_gradient.
 */
struct wlf_linear_gradient *wlf_linear_gradient_from_gradient(
	struct wlf_gradient *gradient);

#endif // TYPES_WLF_LINEAR_GRADIENT_H
