/**
 * @file        wlf_gradient.h
 * @brief       Gradient primitives for wlframe.
 * @details     This file defines gradient base types and sampling interfaces.
 * @author      YaoBing Xiao
 * @date        2026-03-21
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-21, initial version\n
 */

#ifndef TYPES_WLF_GRADIENT_H
#define TYPES_WLF_GRADIENT_H

#include "wlf/math/wlf_fpoint.h"
#include "wlf/types/wlf_color.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct wlf_gradient;

/**
 * @brief A gradient color stop.
 */
struct wlf_gradient_stop {
	double offset;            /**< Stop offset (usually in [0,1]) */
	struct wlf_color color;   /**< Stop color */
};

/**
 * @brief Virtual method table for gradients.
 */
struct wlf_gradient_impl {
	/**
	 * @brief Samples the gradient at a point.
	 * @param gradient Gradient to sample.
	 * @param p Point in gradient space.
	 * @return The sampled color.
	 */
	struct wlf_color (*sample)(struct wlf_gradient *gradient,
		const struct wlf_fpoint *p);

	/**
	 * @brief Destroys the gradient implementation.
	 * @param gradient Gradient to destroy.
	 */
	void (*destroy)(struct wlf_gradient *gradient);
};

/**
 * @brief Base gradient object.
 */
struct wlf_gradient {
	const struct wlf_gradient_impl *impl;  /**< Virtual method table */

	size_t stop_count;                     /**< Number of color stops */
	struct wlf_gradient_stop *stops;       /**< Stop array (sorted by offset) */

	float xform[6];                        /**< Affine transform (a,b,c,d,e,f) */
	bool has_xform;                        /**< Whether transform is enabled */
};

/**
 * @brief Initializes a gradient base structure.
 * @param gradient Gradient to initialize.
 * @param impl Gradient implementation.
 */
void wlf_gradient_init(struct wlf_gradient *gradient,
	const struct wlf_gradient_impl *impl);

/**
 * @brief Sets the gradient stops (copies and sorts).
 * @param gradient Gradient to modify.
 * @param stops Stop array to copy.
 * @param stop_count Number of stops.
 * @return true on success, false on allocation failure or invalid input.
 */
bool wlf_gradient_set_stops(struct wlf_gradient *gradient,
	const struct wlf_gradient_stop *stops, size_t stop_count);

/**
 * @brief Clears and frees all gradient stops.
 * @param gradient Gradient to modify.
 */
void wlf_gradient_destroy_stops(struct wlf_gradient *gradient);

/**
 * @brief Samples a gradient at a point.
 * @param gradient Gradient to sample.
 * @param p Point in world space; transform applied before sampling.
 * @return The sampled color.
 */
struct wlf_color wlf_gradient_sample(struct wlf_gradient *gradient,
	const struct wlf_fpoint *p);

/**
 * @brief Samples the gradient stops at t.
 * @param gradient Gradient containing stops.
 * @param t Parameter in [0,1] (clamped by caller if needed).
 * @return Interpolated stop color.
 */
struct wlf_color wlf_gradient_sample_stops(struct wlf_gradient *gradient,
	double t);

/**
 * @brief Sets the affine transform for a gradient.
 * @param gradient Gradient to modify.
 * @param xform 6-float array [a,b,c,d,e,f] or NULL to disable transform.
 */
void wlf_gradient_set_transform(struct wlf_gradient *gradient,
	const float xform[6]);

/**
 * @brief Disables the transform and resets it to identity.
 * @param gradient Gradient to modify.
 */
void wlf_gradient_set_identity(struct wlf_gradient *gradient);

/**
 * @brief Destroys a gradient and releases associated resources.
 * @param gradient Gradient to destroy.
 */
void wlf_gradient_destroy(struct wlf_gradient *gradient);

#endif // TYPES_WLF_GRADIENT_H
