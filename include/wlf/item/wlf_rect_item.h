/**
 * @file        wlf_rect_item.h
 * @brief       Rectangle item for wlframe UI components.
 * @details     This file provides a rectangular shape item that can be used for
 *              backgrounds, borders, dividers, and other simple geometric shapes.
 *              The rectangle item supports various styling options including
 *              fill colors, stroke properties, and corner radius.
 * @author      YaoBing Xiao
 * @date        2025-07-04
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-07-04, initial version\n
 */

#ifndef ITEM_WLF_RECT_ITEM_H
#define ITEM_WLF_RECT_ITEM_H

#include "wlf_item.h"

/**
 * @brief Rectangle style properties.
 *
 * Defines the visual appearance of a rectangle item including
 * fill color, stroke properties, and corner radius.
 */
struct wlf_rect_style {
	bool has_fill;                      /**< Whether to fill the rectangle */
	uint32_t fill_color;                /**< Fill color in RGBA format */

	bool has_stroke;                    /**< Whether to draw stroke/border */
	uint32_t stroke_color;              /**< Stroke color in RGBA format */
	float stroke_width;                 /**< Stroke width in pixels */

	float corner_radius;                /**< Corner radius for rounded rectangles */
};

/**
 * @brief Rectangle item structure.
 *
 * Extends the base wlf_item with rectangle-specific properties
 * for rendering geometric shapes.
 */
struct wlf_rect_item {
	struct wlf_item base;				/**< Base item structure */
	struct wlf_rect_style style;		/**< Rectangle style properties */
};

/**
 * @brief Create a new rectangle item.
 *
 * Creates a new rectangle item with default styling (transparent fill,
 * no stroke, no corner radius).
 *
 * @param window The window to associate the item with.
 * @return Newly created rectangle item pointer, or NULL on failure.
 *
 * @note The returned item must be destroyed using wlf_rect_item_destroy().
 */
struct wlf_rect_item* wlf_rect_item_create(struct wlf_window *window);

/**
 * @brief Destroy a rectangle item.
 *
 * Destroys the rectangle item and frees all associated resources.
 *
 * @param item Rectangle item to destroy. Can be NULL (no-op).
 */
void wlf_rect_item_destroy(struct wlf_rect_item *item);

/**
 * @brief Set rectangle fill color.
 *
 * Sets the fill color for the rectangle. The color should be in RGBA format
 * where each component is packed into a 32-bit integer.
 *
 * @param item Rectangle item to modify.
 * @param color Fill color in RGBA format (0xRRGGBBAA).
 */
void wlf_rect_item_set_fill_color(struct wlf_rect_item *item, uint32_t color);

/**
 * @brief Set rectangle stroke properties.
 *
 * Sets the stroke (border) color and width for the rectangle.
 *
 * @param item Rectangle item to modify.
 * @param color Stroke color in RGBA format (0xRRGGBBAA).
 * @param width Stroke width in pixels.
 */
void wlf_rect_item_set_stroke(struct wlf_rect_item *item, uint32_t color, float width);

/**
 * @brief Set rectangle corner radius.
 *
 * Sets the corner radius for rounded rectangles. A radius of 0
 * creates sharp corners.
 *
 * @param item Rectangle item to modify.
 * @param radius Corner radius in pixels.
 */
void wlf_rect_item_set_corner_radius(struct wlf_rect_item *item, float radius);

/**
 * @brief Enable or disable rectangle fill.
 *
 * Controls whether the rectangle interior is filled with color.
 *
 * @param item Rectangle item to modify.
 * @param enable true to enable fill, false to disable.
 */
void wlf_rect_item_set_fill_enabled(struct wlf_rect_item *item, bool enable);

/**
 * @brief Enable or disable rectangle stroke.
 *
 * Controls whether the rectangle border/outline is drawn.
 *
 * @param item Rectangle item to modify.
 * @param enable true to enable stroke, false to disable.
 */
void wlf_rect_item_set_stroke_enabled(struct wlf_rect_item *item, bool enable);

/**
 * @brief Get rectangle item base.
 *
 * Returns a pointer to the base wlf_item structure for use with
 * generic item functions.
 *
 * @param item Rectangle item.
 * @return Pointer to base item structure.
 */
struct wlf_item* wlf_rect_item_get_base(struct wlf_rect_item *item);

#endif // ITEM_WLF_RECT_ITEM_H
