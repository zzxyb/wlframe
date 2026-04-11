/**
 * @file        wlf_text_shape.h
 * @brief       Text shape type for wlframe.
 * @details     Defines text geometry and typography attributes inspired by SVG <text>.
 * @author      YaoBing Xiao
 * @date        2026-04-08
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-08, initial version\n
 */

#ifndef SHAPES_WLF_TEXT_SHAPE_H
#define SHAPES_WLF_TEXT_SHAPE_H

#include "wlf/shapes/wlf_shape.h"

#include <stdbool.h>

/**
 * @brief Horizontal text anchor mode.
 */
enum wlf_text_anchor {
	WLF_TEXT_ANCHOR_START = 0,
	WLF_TEXT_ANCHOR_MIDDLE = 1,
	WLF_TEXT_ANCHOR_END = 2,
};

struct wlf_text_shape {
	struct wlf_shape base;
	float x; /**< Text anchor x coordinate. */
	float y; /**< Text anchor y coordinate. */
	char font_family[64]; /**< Font family name. */
	float font_size; /**< Font size in user units. */
	enum wlf_text_anchor text_anchor; /**< Horizontal anchor mode. */
	char text[256]; /**< Text content (UTF-8). */
	struct wlf_shape_state state; /**< Shared fill/stroke style payload. */
};

/**
 * @brief Create a text shape.
 * @param x Text x coordinate.
 * @param y Text y coordinate.
 * @param text Text content (NULL treated as empty string).
 * @param font_family Font family (NULL treated as "sans-serif").
 * @param font_size Font size.
 * @param text_anchor Text anchor mode.
 * @return New shape pointer, or NULL on failure.
 */
struct wlf_shape *wlf_text_shape_create(float x, float y,
	const char *text, const char *font_family, float font_size,
	enum wlf_text_anchor text_anchor);

/**
 * @brief Check whether a shape is a text shape.
 * @param shape Shape to test.
 * @return true if shape is text, false otherwise.
 */
bool wlf_shape_is_text(struct wlf_shape *shape);

/**
 * @brief Cast a base shape to a text shape.
 * @param shape Base shape pointer.
 * @return Text shape pointer (asserts if type mismatch).
 */
struct wlf_text_shape *wlf_text_shape_from_shape(struct wlf_shape *shape);

#endif // SHAPES_WLF_TEXT_SHAPE_H
