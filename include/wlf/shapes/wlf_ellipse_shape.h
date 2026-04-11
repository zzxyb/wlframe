/**
 * @file        wlf_ellipse_shape.h
 * @brief       Ellipse shape type for wlframe.
 * @details     Defines an ellipse geometry with center and radii.
 * @author      YaoBing Xiao
 * @date        2026-04-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-07, initial version\n
 */

#ifndef SHAPES_WLF_ELLIPSE_SHAPE_H
#define SHAPES_WLF_ELLIPSE_SHAPE_H

#include "wlf/shapes/wlf_shape.h"

#include <stdbool.h>

struct wlf_ellipse_shape {
	struct wlf_shape base;
	float cx; /**< Ellipse center x. */
	float cy; /**< Ellipse center y. */
	float rx; /**< Radius on x axis. */
	float ry; /**< Radius on y axis. */
	struct wlf_shape_state state; /**< Shared fill/stroke style payload. */
};

/**
 * @brief Create an ellipse shape.
 * @param cx Center x.
 * @param cy Center y.
 * @param rx Radius on x axis.
 * @param ry Radius on y axis.
 * @return New shape pointer, or NULL on failure.
 */
struct wlf_shape *wlf_ellipse_shape_create(float cx, float cy, float rx, float ry);

/**
 * @brief Check whether a shape is an ellipse.
 * @param shape Shape to test.
 * @return true if shape is ellipse, false otherwise.
 */
bool wlf_shape_is_ellipse(struct wlf_shape *shape);

/**
 * @brief Cast a base shape to an ellipse shape.
 * @param shape Base shape pointer.
 * @return Ellipse shape pointer (asserts if type mismatch).
 */
struct wlf_ellipse_shape *wlf_ellipse_shape_from_shape(struct wlf_shape *shape);

#endif // SHAPES_WLF_ELLIPSE_SHAPE_H
