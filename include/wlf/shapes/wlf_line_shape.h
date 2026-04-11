/**
 * @file        wlf_line_shape.h
 * @brief       Line segment shape type for wlframe.
 * @details     Defines a straight line between two points.
 * @author      YaoBing Xiao
 * @date        2026-04-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-07, initial version\n
 */

#ifndef SHAPES_WLF_LINE_SHAPE_H
#define SHAPES_WLF_LINE_SHAPE_H

#include "wlf/shapes/wlf_shape.h"

#include <stdbool.h>

struct wlf_line_shape {
	struct wlf_shape base;
	float x1; /**< Start x. */
	float y1; /**< Start y. */
	float x2; /**< End x. */
	float y2; /**< End y. */
	struct wlf_shape_state state; /**< Shared fill/stroke style payload. */
};

/**
 * @brief Create a line shape.
 * @param x1 Start x.
 * @param y1 Start y.
 * @param x2 End x.
 * @param y2 End y.
 * @return New shape pointer, or NULL on failure.
 */
struct wlf_shape *wlf_line_shape_create(float x1, float y1, float x2, float y2);

/**
 * @brief Check whether a shape is a line.
 * @param shape Shape to test.
 * @return true if shape is line, false otherwise.
 */
bool wlf_shape_is_line(struct wlf_shape *shape);

/**
 * @brief Cast a base shape to a line shape.
 * @param shape Base shape pointer.
 * @return Line shape pointer (asserts if type mismatch).
 */
struct wlf_line_shape *wlf_line_shape_from_shape(struct wlf_shape *shape);

#endif // SHAPES_WLF_LINE_SHAPE_H
