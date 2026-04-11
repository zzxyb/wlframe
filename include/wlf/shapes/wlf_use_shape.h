/**
 * @file        wlf_use_shape.h
 * @brief       SVG-like use reference shape type for wlframe.
 * @details     Represents a reference to previously defined symbol geometry with offset.
 * @author      YaoBing Xiao
 * @date        2026-04-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-07, initial version\n
 */

#ifndef SHAPES_WLF_USE_SHAPE_H
#define SHAPES_WLF_USE_SHAPE_H

#include "wlf/shapes/wlf_shape.h"

#include <stdbool.h>

struct wlf_use_shape {
	struct wlf_shape base;
	char href[64]; /**< Referenced symbol id/href. */
	float x; /**< X offset. */
	float y; /**< Y offset. */
};

/**
 * @brief Create a use shape.
 * @param href Referenced symbol id/href.
 * @param x X offset.
 * @param y Y offset.
 * @return New shape pointer, or NULL on failure.
 */
struct wlf_shape *wlf_use_shape_create(const char *href, float x, float y);

/**
 * @brief Check whether a shape is a use shape.
 * @param shape Shape to test.
 * @return true if shape is use, false otherwise.
 */
bool wlf_shape_is_use(struct wlf_shape *shape);

/**
 * @brief Cast a base shape to a use shape.
 * @param shape Base shape pointer.
 * @return Use shape pointer (asserts if type mismatch).
 */
struct wlf_use_shape *wlf_use_shape_from_shape(struct wlf_shape *shape);

#endif // SHAPES_WLF_USE_SHAPE_H
