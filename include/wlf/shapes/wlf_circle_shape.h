/**
 * @file        wlf_circle_shape.h
 * @brief       Circle shape type for wlframe.
 * @details     Defines a circle geometry with center and radius.
 * @author      YaoBing Xiao
 * @date        2026-04-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-07, initial version\n
 */

#ifndef SHAPES_WLF_CIRCLE_SHAPE_H
#define SHAPES_WLF_CIRCLE_SHAPE_H

#include "wlf/shapes/wlf_shape.h"

#include <stdbool.h>

struct wlf_circle_shape {
	struct wlf_shape base;
	float cx; /**< Circle center x. */
	float cy; /**< Circle center y. */
	float r;  /**< Circle radius. */
	struct wlf_shape_state state; /**< Shared fill/stroke style payload. */
};

/**
 * @brief Create a circle shape.
 * @param cx Center x.
 * @param cy Center y.
 * @param r Radius.
 * @return New shape pointer, or NULL on failure.
 */
struct wlf_shape *wlf_circle_shape_create(float cx, float cy, float r);

/**
 * @brief Check whether a shape is a circle.
 * @param shape Shape to test.
 * @return true if shape is circle, false otherwise.
 */
bool wlf_shape_is_circle(struct wlf_shape *shape);

/**
 * @brief Cast a base shape to a circle shape.
 * @param shape Base shape pointer.
 * @return Circle shape pointer (asserts if type mismatch).
 */
struct wlf_circle_shape *wlf_circle_shape_from_shape(struct wlf_shape *shape);

#endif // SHAPES_WLF_CIRCLE_SHAPE_H
