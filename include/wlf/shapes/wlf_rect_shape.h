/**
 * @file        wlf_rect_shape.h
 * @brief       Rectangle shape type for wlframe.
 * @details     Defines rectangle geometry with optional corner radii.
 * @author      YaoBing Xiao
 * @date        2026-04-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-07, initial version\n
 */

#ifndef SHAPES_WLF_RECT_SHAPE_H
#define SHAPES_WLF_RECT_SHAPE_H

#include "wlf/shapes/wlf_shape.h"

#include <stdbool.h>

struct wlf_rect_shape {
	struct wlf_shape base;
	float x;      /**< Left coordinate. */
	float y;      /**< Top coordinate. */
	float width;  /**< Rectangle width. */
	float height; /**< Rectangle height. */
	float rx;     /**< Corner radius x. */
	float ry;     /**< Corner radius y. */
	struct wlf_shape_state state; /**< Shared fill/stroke style payload. */
};

/**
 * @brief Create a rectangle shape.
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @param width Rectangle width.
 * @param height Rectangle height.
 * @param rx Corner radius x.
 * @param ry Corner radius y.
 * @return New shape pointer, or NULL on failure.
 */
struct wlf_shape *wlf_rect_shape_create(float x, float y,
	float width, float height, float rx, float ry);

/**
 * @brief Check whether a shape is a rectangle.
 * @param shape Shape to test.
 * @return true if shape is rectangle, false otherwise.
 */
bool wlf_shape_is_rect(struct wlf_shape *shape);

/**
 * @brief Cast a base shape to a rectangle shape.
 * @param shape Base shape pointer.
 * @return Rectangle shape pointer (asserts if type mismatch).
 */
struct wlf_rect_shape *wlf_rect_shape_from_shape(struct wlf_shape *shape);

#endif // SHAPES_WLF_RECT_SHAPE_H
