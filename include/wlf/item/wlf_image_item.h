/**
 * @file        wlf_image_item.h
 * @brief       Image item for wlframe UI components.
 * @details     This file provides an image display item that can render
 *              various image formats including PNG, JPEG, and others.
 *              The image item supports scaling, alignment, and various
 *              compositing modes.
 * @author      YaoBing Xiao
 * @date        2025-07-04
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-07-04, initial version\n
 */

#ifndef ITEM_WLF_IMAGE_ITEM_H
#define ITEM_WLF_IMAGE_ITEM_H

#include "wlf_item.h"
#include "../image/wlf_image.h"

/**
 * @brief Image scaling mode enumeration.
 *
 * Defines how the image should be scaled to fit within the item's bounds.
 */
enum wlf_image_scale_mode {
	WLF_IMAGE_SCALE_NONE,           /**< No scaling, use original size */
	WLF_IMAGE_SCALE_FIT,            /**< Scale to fit, maintain aspect ratio */
	WLF_IMAGE_SCALE_FILL,           /**< Scale to fill, maintain aspect ratio, may crop */
	WLF_IMAGE_SCALE_STRETCH         /**< Stretch to fill, may distort aspect ratio */
};

/**
 * @brief Image alignment enumeration.
 *
 * Defines how the image should be aligned within the item's bounds.
 */
enum wlf_image_align {
	WLF_IMAGE_ALIGN_TOP_LEFT,       /**< Align to top-left corner */
	WLF_IMAGE_ALIGN_TOP,            /**< Align to top center */
	WLF_IMAGE_ALIGN_TOP_RIGHT,      /**< Align to top-right corner */
	WLF_IMAGE_ALIGN_LEFT,           /**< Align to left center */
	WLF_IMAGE_ALIGN_CENTER,         /**< Center alignment */
	WLF_IMAGE_ALIGN_RIGHT,          /**< Align to right center */
	WLF_IMAGE_ALIGN_BOTTOM_LEFT,    /**< Align to bottom-left corner */
	WLF_IMAGE_ALIGN_BOTTOM,         /**< Align to bottom center */
	WLF_IMAGE_ALIGN_BOTTOM_RIGHT    /**< Align to bottom-right corner */
};

/**
 * @brief Image item structure.
 *
 * Extends the base wlf_item with image-specific properties
 * for rendering image content.
 */
struct wlf_image_item {
	struct wlf_item base;               /**< Base item structure */

	struct wlf_image *image;            /**< Image data */
	bool owns_image;                    /**< Whether this item owns the image data */

	enum wlf_image_scale_mode scale_mode;  /**< Image scaling mode */
	enum wlf_image_align alignment;        /**< Image alignment */
	bool smooth_scaling;                /**< Whether to use smooth scaling */

	uint32_t tint_color;                /**< Tint color in RGBA format */
	bool has_tint;                      /**< Whether tinting is enabled */

	struct wlf_rect source_rect;        /**< Source rectangle in image */
	struct wlf_rect dest_rect;          /**< Destination rectangle in item */
	bool layout_dirty;                  /**< Whether layout needs recalculation */
};

/**
 * @brief Create a new image item.
 *
 * Creates a new image item with no image data initially.
 * Use wlf_image_item_set_image() to set the image to display.
 *
 * @param window The window to associate the item with.
 * @return Newly created image item pointer, or NULL on failure.
 *
 * @note The returned item must be destroyed using wlf_image_item_destroy().
 */
struct wlf_image_item* wlf_image_item_create(struct wlf_window *window);

/**
 * @brief Destroy an image item.
 *
 * Destroys the image item and frees all associated resources.
 * If the item owns the image data, it will be destroyed as well.
 *
 * @param item Image item to destroy. Can be NULL (no-op).
 */
void wlf_image_item_destroy(struct wlf_image_item *item);

/**
 * @brief Set image data for the item.
 *
 * Sets the image to be displayed by this item. The item can either
 * take ownership of the image or just reference it.
 *
 * @param item Image item to modify.
 * @param image Image data to display.
 * @param take_ownership true if item should own the image, false to reference.
 */
void wlf_image_item_set_image(struct  wlf_image_item *item, struct wlf_image *image, bool take_ownership);

/**
 * @brief Load image from file.
 *
 * Loads an image from a file path and sets it as the item's image.
 * The item will take ownership of the loaded image.
 *
 * @param item Image item to modify.
 * @param path Path to the image file.
 * @return true if image was loaded successfully, false otherwise.
 */
bool wlf_image_item_load_from_file(struct wlf_image_item *item, const char *path);

/**
 * @brief Set image scaling mode.
 *
 * Sets how the image should be scaled to fit within the item's bounds.
 *
 * @param item Image item to modify.
 * @param mode Scaling mode to use.
 */
void wlf_image_item_set_scale_mode(struct wlf_image_item *item, enum wlf_image_scale_mode mode);

/**
 * @brief Set image alignment.
 *
 * Sets how the image should be aligned within the item's bounds.
 *
 * @param item Image item to modify.
 * @param alignment Alignment mode to use.
 */
void wlf_image_item_set_alignment(struct wlf_image_item *item, enum wlf_image_align alignment);

/**
 * @brief Set image tint color.
 *
 * Sets a tint color that will be applied to the image during rendering.
 * The tint color is multiplied with the image pixels.
 *
 * @param item Image item to modify.
 * @param color Tint color in RGBA format (0xRRGGBBAA).
 */
void wlf_image_item_set_tint(struct wlf_image_item *item, uint32_t color);

/**
 * @brief Enable or disable image tinting.
 *
 * Controls whether the tint color is applied to the image.
 *
 * @param item Image item to modify.
 * @param enable true to enable tinting, false to disable.
 */
void wlf_image_item_set_tint_enabled(struct wlf_image_item *item, bool enable);

/**
 * @brief Enable or disable smooth scaling.
 *
 * Controls whether smooth (bilinear) scaling is used when resizing images.
 *
 * @param item Image item to modify.
 * @param enable true to enable smooth scaling, false for nearest neighbor.
 */
void wlf_image_item_set_smooth_scaling(struct wlf_image_item *item, bool enable);

/**
 * @brief Get image item base.
 *
 * Returns a pointer to the base wlf_item structure for use with
 * generic item functions.
 *
 * @param item Image item.
 * @return Pointer to base item structure.
 */
struct wlf_item* wlf_image_item_get_base(struct wlf_image_item *item);

/**
 * @brief Get image natural size.
 *
 * Returns the natural (original) size of the image, or (0,0) if no image is set.
 *
 * @param item Image item.
 * @param width Pointer to store image width.
 * @param height Pointer to store image height.
 */
void wlf_image_item_get_natural_size(struct wlf_image_item *item, int *width, int *height);

#endif // ITEM_WLF_IMAGE_ITEM_H
