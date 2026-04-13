/**
 * @file        wlf_poly_shape.h
 * @brief       Polyline/polygon shape type for wlframe.
 * @details     Defines a shape backed by a flat point array that can be open or closed.
 * @author      YaoBing Xiao
 * @date        2026-04-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-07, initial version\n
 */

#ifndef SHAPES_WLF_POLY_SHAPE_H
#define SHAPES_WLF_POLY_SHAPE_H

#include "wlf/shapes/wlf_shape.h"

#include <stdbool.h>

struct wlf_poly_shape {
	struct wlf_shape base;
	float *points; /**< Flat point array: x0,y0,x1,y1,... */
	int count; /**< Number of points. */
	bool closed; /**< true for polygon, false for polyline. */
	struct wlf_shape_state state; /**< Shared fill/stroke style payload. */
};

/**
 * @brief Create a polyline or polygon shape.
 * @param points Flat point array.
 * @param count Number of points.
 * @param closed true for polygon, false for polyline.
 * @return New shape pointer, or NULL on failure.
 */
struct wlf_shape *wlf_poly_shape_create(const float *points, int count, bool closed);

/**
 * @brief Check whether a shape is a poly shape.
 * @param shape Shape to test.
 * @return true if shape is poly, false otherwise.
 */
bool wlf_shape_is_poly(struct wlf_shape *shape);

/**
 * @brief Cast a base shape to a poly shape.
 * @param shape Base shape pointer.
 * @return Poly shape pointer (asserts if type mismatch).
 */
struct wlf_poly_shape *wlf_poly_shape_from_shape(struct wlf_shape *shape);

#endif // SHAPES_WLF_POLY_SHAPE_H
