/**
 * @file        wlf_text_item.h
 * @brief       Text item for wlframe UI components.
 * @details     This file provides a text display item that can render
 *              text with various fonts, sizes, colors, and alignment options.
 *              The text item supports multi-line text, text wrapping,
 *              and various typography features.
 * @author      YaoBing Xiao
 * @date        2025-07-04
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-07-04, initial version\n
 */

#ifndef WLF_TEXT_ITEM_H
#define WLF_TEXT_ITEM_H

#include "wlf_item.h"
#include <stddef.h>

/**
 * @brief Text alignment enumeration.
 *
 * Defines how text should be aligned within the item's bounds.
 */
enum wlf_text_align {
	WLF_TEXT_ALIGN_LEFT,            /**< Left alignment */
	WLF_TEXT_ALIGN_CENTER,          /**< Center alignment */
	WLF_TEXT_ALIGN_RIGHT,           /**< Right alignment */
	WLF_TEXT_ALIGN_JUSTIFY          /**< Justified alignment */
};

/**
 * @brief Text vertical alignment enumeration.
 *
 * Defines how text should be vertically aligned within the item's bounds.
 */
enum wlf_text_valign {
	WLF_TEXT_VALIGN_TOP,            /**< Top alignment */
	WLF_TEXT_VALIGN_CENTER,         /**< Center alignment */
	WLF_TEXT_VALIGN_BOTTOM          /**< Bottom alignment */
};

/**
 * @brief Text wrapping mode enumeration.
 *
 * Defines how text should wrap when it exceeds the item's width.
 */
enum wlf_text_wrap {
	WLF_TEXT_WRAP_NONE,             /**< No wrapping, text may overflow */
	WLF_TEXT_WRAP_WORD,             /**< Wrap at word boundaries */
	WLF_TEXT_WRAP_CHAR              /**< Wrap at character boundaries */
};

/**
 * @brief Text style properties.
 *
 * Defines the visual appearance of text including font, size, color,
 * and various text effects.
 */
struct wlf_text_style {
	char *font_family;                  /**< Font family name */
	int font_size;                      /**< Font size in points */
	bool bold;                          /**< Whether text is bold */
	bool italic;                        /**< Whether text is italic */
	bool underline;                     /**< Whether text is underlined */
	bool strikethrough;                 /**< Whether text is struck through */

	uint32_t color;                     /**< Text color in RGBA format */
	uint32_t background_color;          /**< Background color in RGBA format */
	bool has_background;                /**< Whether background is enabled */

	enum wlf_text_align alignment;      /**< Horizontal text alignment */
	enum wlf_text_valign valignment;    /**< Vertical text alignment */
	enum wlf_text_wrap wrap_mode;       /**< Text wrapping mode */
	float line_spacing;                 /**< Line spacing multiplier */

	bool has_shadow;                    /**< Whether shadow is enabled */
	uint32_t shadow_color;              /**< Shadow color in RGBA format */
	float shadow_offset_x;              /**< Shadow X offset */
	float shadow_offset_y;              /**< Shadow Y offset */
	float shadow_blur;                  /**< Shadow blur radius */

	bool has_outline;                   /**< Whether outline is enabled */
	uint32_t outline_color;             /**< Outline color in RGBA format */
	float outline_width;                /**< Outline width */
};

/**
 * @brief Text item structure.
 *
 * Extends the base wlf_item with text-specific properties
 * for rendering text content.
 */
struct wlf_text_item {
	struct wlf_item base;               /**< Base item structure */

	char *text;                         /**< UTF-8 text content */
	size_t text_length;                 /**< Length of text in bytes */

	struct wlf_text_style style;        /**< Text style properties */

	int max_width;                      /**< Maximum text width (0 = no limit) */
	int max_height;                     /**< Maximum text height (0 = no limit) */

	struct wlf_rect text_bounds;        /**< Calculated text bounds */
	bool layout_dirty;                  /**< Whether layout needs recalculation */
	void *font_cache;                   /**< Cached font resources */
};

/**
 * @brief Create a new text item.
 *
 * Creates a new text item with default styling and no text content.
 * Use wlf_text_item_set_text() to set the text to display.
 *
 * @param window The window to associate the item with.
 * @return Newly created text item pointer, or NULL on failure.
 *
 * @note The returned item must be destroyed using wlf_text_item_destroy().
 */
struct wlf_text_item* wlf_text_item_create(struct wlf_window *window);

/**
 * @brief Destroy a text item.
 *
 * Destroys the text item and frees all associated resources
 * including text content and font cache.
 *
 * @param item Text item to destroy. Can be NULL (no-op).
 */
void wlf_text_item_destroy(struct wlf_text_item *item);

/**
 * @brief Set text content.
 *
 * Sets the text content to be displayed by this item.
 * The text should be valid UTF-8 encoded string.
 *
 * @param item Text item to modify.
 * @param text UTF-8 text content. Can be NULL to clear text.
 */
void wlf_text_item_set_text(struct wlf_text_item *item, const char *text);

/**
 * @brief Set font family.
 *
 * Sets the font family name for the text. Common names include
 * "Arial", "Times New Roman", "Helvetica", etc.
 *
 * @param item Text item to modify.
 * @param font_family Font family name. Can be NULL for default font.
 */
void wlf_text_item_set_font_family(struct wlf_text_item *item, const char *font_family);

/**
 * @brief Set font size.
 *
 * Sets the font size in points.
 *
 * @param item Text item to modify.
 * @param size Font size in points (typically 8-72).
 */
void wlf_text_item_set_font_size(struct wlf_text_item *item, int size);

/**
 * @brief Set text color.
 *
 * Sets the color of the text.
 *
 * @param item Text item to modify.
 * @param color Text color in RGBA format (0xRRGGBBAA).
 */
void wlf_text_item_set_color(struct wlf_text_item *item, uint32_t color);

/**
 * @brief Set text alignment.
 *
 * Sets the horizontal alignment of the text within the item's bounds.
 *
 * @param item Text item to modify.
 * @param alignment Horizontal alignment mode.
 */
void wlf_text_item_set_alignment(struct wlf_text_item *item, enum wlf_text_align alignment);

/**
 * @brief Set text vertical alignment.
 *
 * Sets the vertical alignment of the text within the item's bounds.
 *
 * @param item Text item to modify.
 * @param valignment Vertical alignment mode.
 */
void wlf_text_item_set_valignment(struct wlf_text_item *item, enum wlf_text_valign valignment);

/**
 * @brief Set text wrapping mode.
 *
 * Sets how text should wrap when it exceeds the item's width.
 *
 * @param item Text item to modify.
 * @param wrap_mode Text wrapping mode.
 */
void wlf_text_item_set_wrap_mode(struct wlf_text_item *item, enum wlf_text_wrap wrap_mode);

/**
 * @brief Set font style flags.
 *
 * Sets various font style flags like bold, italic, underline, etc.
 *
 * @param item Text item to modify.
 * @param bold Whether text should be bold.
 * @param italic Whether text should be italic.
 * @param underline Whether text should be underlined.
 * @param strikethrough Whether text should be struck through.
 */
void wlf_text_item_set_style_flags(struct wlf_text_item *item, bool bold, bool italic,
	bool underline, bool strikethrough);

/**
 * @brief Set text shadow.
 *
 * Sets shadow properties for the text.
 *
 * @param item Text item to modify.
 * @param color Shadow color in RGBA format.
 * @param offset_x Shadow X offset in pixels.
 * @param offset_y Shadow Y offset in pixels.
 * @param blur Shadow blur radius in pixels.
 */
void wlf_text_item_set_shadow(struct wlf_text_item *item, uint32_t color,
	float offset_x, float offset_y, float blur);

/**
 * @brief Set text outline.
 *
 * Sets outline properties for the text.
 *
 * @param item Text item to modify.
 * @param color Outline color in RGBA format.
 * @param width Outline width in pixels.
 */
void wlf_text_item_set_outline(struct wlf_text_item *item, uint32_t color, float width);

/**
 * @brief Enable or disable text shadow.
 *
 * Controls whether the text shadow is rendered.
 *
 * @param item Text item to modify.
 * @param enable true to enable shadow, false to disable.
 */
void wlf_text_item_set_shadow_enabled(struct wlf_text_item *item, bool enable);

/**
 * @brief Enable or disable text outline.
 *
 * Controls whether the text outline is rendered.
 *
 * @param item Text item to modify.
 * @param enable true to enable outline, false to disable.
 */
void wlf_text_item_set_outline_enabled(struct wlf_text_item *item, bool enable);

/**
 * @brief Set maximum text dimensions.
 *
 * Sets the maximum width and height for text layout.
 * Text will wrap or be clipped to fit within these bounds.
 *
 * @param item Text item to modify.
 * @param max_width Maximum width in pixels (0 = no limit).
 * @param max_height Maximum height in pixels (0 = no limit).
 */
void wlf_text_item_set_max_size(struct wlf_text_item *item, int max_width, int max_height);

/**
 * @brief Get text item base.
 *
 * Returns a pointer to the base wlf_item structure for use with
 * generic item functions.
 *
 * @param item Text item.
 * @return Pointer to base item structure.
 */
struct wlf_item* wlf_text_item_get_base(struct wlf_text_item *item);

/**
 * @brief Get text bounds.
 *
 * Returns the calculated bounds of the text content.
 *
 * @param item Text item.
 * @return Rectangle containing text bounds.
 */
struct wlf_rect wlf_text_item_get_text_bounds(struct wlf_text_item *item);

/**
 * @brief Measure text size.
 *
 * Calculates the size that the text would occupy with current styling.
 *
 * @param item Text item.
 * @param width Pointer to store calculated width.
 * @param height Pointer to store calculated height.
 */
void wlf_text_item_measure_text(struct wlf_text_item *item, int *width, int *height);

#endif /* WLF_TEXT_ITEM_H */
