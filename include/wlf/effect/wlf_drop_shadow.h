/**
 * @file        wlf_drop_shadow.h
 * @brief       Drop-shadow effect data for wlframe.
 * @details     This file provides a shape-based drop-shadow primitive.
 *              The primitive describes shadow offset, blur deviation, colour,
 *              opacity, and optional SVG input and result names.
 * @author      YaoBing Xiao
 * @date        2026-09-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-09-09, initial version\n
 */

#ifndef EFFECT_WLF_DROP_SHADOW_H
#define EFFECT_WLF_DROP_SHADOW_H

#include "wlf/shapes/wlf_shape.h"
#include "wlf/types/wlf_color.h"

#include <stdbool.h>

/**
 * @brief A shape-based drop-shadow effect primitive.
 */
struct wlf_drop_shadow {
	struct wlf_shape base; /**< Base shape */
	float dx;              /**< Horizontal shadow offset */
	float dy;              /**< Vertical shadow offset */
	float std_deviation_x; /**< Horizontal Gaussian standard deviation */
	float std_deviation_y; /**< Vertical Gaussian standard deviation */
	struct wlf_color color;/**< Shadow colour */
	float opacity;         /**< Shadow opacity in the range [0, 1] */
	char input[64];        /**< Optional input image name */
	char result[64];       /**< Optional result image name */
};

/**
 * @brief Creates a drop-shadow effect.
 *
 * @param dx Horizontal shadow offset.
 * @param dy Vertical shadow offset.
 * @param std_deviation_x Horizontal Gaussian standard deviation.
 * @param std_deviation_y Vertical Gaussian standard deviation.
 * @param color Shadow colour.
 * @param opacity Shadow opacity in the range [0, 1].
 * @return New drop shadow or NULL if allocation fails.
 */
struct wlf_drop_shadow *wlf_drop_shadow_create(float dx, float dy,
	float std_deviation_x, float std_deviation_y,
	struct wlf_color color, float opacity);

/**
 * @brief Checks whether a shape is a drop shadow.
 *
 * @param shape Shape to test.
 * @return true if the shape is a drop shadow, otherwise false.
 */
bool wlf_shape_is_drop_shadow(struct wlf_shape *shape);

/**
 * @brief Gets a drop shadow from its base shape.
 *
 * @param shape Base shape belonging to a drop shadow.
 * @return Drop shadow containing the base shape.
 */
struct wlf_drop_shadow *wlf_drop_shadow_from_shape(struct wlf_shape *shape);

#endif // EFFECT_WLF_DROP_SHADOW_H
