/**
 * @file        wlf_symbol_shape.h
 * @brief       Symbol reference shape type for wlframe.
 * @details     Stores a symbol id for deferred symbol-based geometry composition.
 * @author      YaoBing Xiao
 * @date        2026-04-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-07, initial version\n
 */

#ifndef SHAPES_WLF_SYMBOL_SHAPE_H
#define SHAPES_WLF_SYMBOL_SHAPE_H

#include "wlf/shapes/wlf_shape.h"

#include <stdbool.h>

struct wlf_symbol_shape {
	struct wlf_shape base;
	char id[64]; /**< Symbol identifier. */
};

/**
 * @brief Create a symbol shape.
 * @param id Symbol id.
 * @return New shape pointer, or NULL on failure.
 */
struct wlf_shape *wlf_symbol_shape_create(const char *id);

/**
 * @brief Check whether a shape is a symbol shape.
 * @param shape Shape to test.
 * @return true if shape is symbol, false otherwise.
 */
bool wlf_shape_is_symbol(struct wlf_shape *shape);

/**
 * @brief Cast a base shape to a symbol shape.
 * @param shape Base shape pointer.
 * @return Symbol shape pointer (asserts if type mismatch).
 */
struct wlf_symbol_shape *wlf_symbol_shape_from_shape(struct wlf_shape *shape);

#endif // SHAPES_WLF_SYMBOL_SHAPE_H
