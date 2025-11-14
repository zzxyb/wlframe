/**
 * @file        wlf_gradient.h
 * @brief       Abstract gradient interface for wlframe.
 * @details     Provides a polymorphic gradient system via implementation structs.
 *              Each gradient type (linear, radial, etc.) implements its own behavior
 *              through a function table similar to the renderer design pattern.
 * @author      YaoBing Xiao
 * @date        2025-11-11
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef TYPES_WLF_GRADIENT_H
#define TYPES_WLF_GRADIENT_H

#include "wlf/types/wlf_color.h"

#include <stdint.h>
#include <stdbool.h>

struct wlf_gradient;

/**
 * @brief Virtual function table for gradient implementations.
 */
struct wlf_gradient_impl {
	void (*destroy)(struct wlf_gradient *gradient);
	struct wlf_color (*sample)(const struct wlf_gradient *gradient, double t);
};

/**
 * @brief Common base for all gradients.
 */
struct wlf_gradient {
	const struct wlf_gradient_impl *impl; /**< Implementation function table */
	struct wlf_gradient_stop *stops;      /**< Color stops */
	uint32_t stop_count;                  /**< Number of stops */
};

/**
 * @brief Color stop structure.
 */
struct wlf_gradient_stop {
	double position;        /**< Stop position (0.0–1.0) */
	struct wlf_color color; /**< Color value */
};

/**
 * @brief Initializes a gradient (for use by derived implementations).
 */
void wlf_gradient_init(struct wlf_gradient *gradient, const struct wlf_gradient_impl *impl);

/**
 * @brief Adds a color stop.
 */
bool wlf_gradient_add_stop(struct wlf_gradient *gradient, double position, struct wlf_color color);

/**
 * @brief Sorts the color stops in ascending order.
 */
void wlf_gradient_sort_stops(struct wlf_gradient *gradient);

/**
 * @brief Frees memory used by color stops (for impls' destroy).
 */
void wlf_gradient_release_stops(struct wlf_gradient *gradient);

/**
 * @brief Samples color through the gradient’s implementation.
 */
static inline struct wlf_color
wlf_gradient_sample(const struct wlf_gradient *gradient, double t)
{
	return gradient && gradient->impl && gradient->impl->sample
	           ? gradient->impl->sample(gradient, t)
	           : WLF_COLOR_TRANSPARENT;
}

/**
 * @brief Destroys a gradient through its implementation.
 */
static inline void wlf_gradient_destroy(struct wlf_gradient *gradient)
{
	if (gradient && gradient->impl && gradient->impl->destroy)
		gradient->impl->destroy(gradient);
}

#endif // TYPES_WLF_GRADIENT_H
