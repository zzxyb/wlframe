/**
 * @file        wlf_image.h
 * @brief       Image abstraction and utility functions for wlframe.
 * @details     This file defines the wlf_image structure and related enums, providing
 *              a unified interface for loading, saving, and manipulating images of
 *              various formats (PNG, JPEG, SVG, etc.). It also includes helpers for
 *              querying image properties and managing image memory.
 *
 *              Typical usage:
 *                  - Load an image using the appropriate loader.
 *                  - Access image pixel data, width, height, format, etc.
 *                  - Save or destroy the image when done.
 *
 * @author      YaoBing Xiao
 * @date        2025-06-08
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-08, initial version\n
 */

#ifndef IMAGE_WLF_IMAGE_H
#define IMAGE_WLF_IMAGE_H

#include <stdint.h>
#include <stdbool.h>

struct wlf_image;

/**
 * @brief Supported image types.
 */
enum wlf_image_type {
	WLF_IMAGE_TYPE_UNKNOWN, /**< Unknown or unsupported image type */
	WLF_IMAGE_TYPE_PNG,     /**< PNG (Portable Network Graphics) image */
	WLF_IMAGE_TYPE_JPEG,    /**< JPEG (Joint Photographic Experts Group) image */
	WLF_IMAGE_TYPE_SVG,     /**< SVG (Scalable Vector Graphics) image */
};

/**
 * @brief Mapping between image type enum and string name.
 */
struct wlf_image_type_map {
	enum wlf_image_type type;
	const char *name;
};

/**
 * @brief Image implementation interface for format-specific operations.
 */
struct wlf_image_impl {
	bool (*save)(struct wlf_image *image, const char *filename);
	bool (*load)(struct wlf_image *image, const char *filename, bool enable_16_bit);
	void (*destroy)(struct wlf_image *image);
};

/**
 * @brief Image type string mapping table.
 */
static const struct wlf_image_type_map image_type[] = {
	{ WLF_IMAGE_TYPE_UNKNOWN, "unknown" },
	{ WLF_IMAGE_TYPE_PNG,  "png"  },
	{ WLF_IMAGE_TYPE_JPEG, "jpeg" },
	{ WLF_IMAGE_TYPE_SVG,  "svg"  },
};

/**
 * @brief Supported image bit depths.
 */
enum wlf_image_bit_depth {
	WLF_IMAGE_BIT_DEPTH_1 = 1,   /**< 1 bit per channel (black and white) */
	WLF_IMAGE_BIT_DEPTH_2 = 2,   /**< 2 bits per channel */
	WLF_IMAGE_BIT_DEPTH_4 = 4,   /**< 4 bits per channel */
	WLF_IMAGE_BIT_DEPTH_8 = 8,   /**< 8 bits per channel (standard) */
	WLF_IMAGE_BIT_DEPTH_16 = 16, /**< 16 bits per channel (high precision) */
};

/**
 * @brief Supported image pixel formats.
 */
enum wlf_image_format {
	WLF_COLOR_TYPE_UNKNOWN,      /**< Unknown or unsupported pixel format */
	WLF_COLOR_TYPE_RGB,          /**< RGB format: 3 channels (Red, Green, Blue) */
	WLF_COLOR_TYPE_RGBA,         /**< RGBA format: 4 channels (Red, Green, Blue, Alpha) */
	WLF_COLOR_TYPE_GRAY,         /**< Grayscale format: 1 channel */
	WLF_COLOR_TYPE_GRAY_ALPHA,   /**< Grayscale with alpha: 2 channels (Gray, Alpha) */
};

/**
 * @brief Main image structure.
 */
struct wlf_image {
	const struct wlf_image_impl *impl; /**< Implementation for format-specific operations */

	uint32_t width;        /**< Image width in pixels */
	uint32_t height;       /**< Image height in pixels */
	unsigned char *data;   /**< Pointer to pixel data */
	uint32_t format;       /**< Pixel format (see wlf_image_format) */
	uint32_t stride;       /**< Bytes per row */
	enum wlf_image_bit_depth bit_depth; /**< Bit depth per channel */

	enum wlf_image_type image_type; /**< The image file type (see @ref wlf_image_type), e.g., PNG, JPEG, SVG, etc. */
	bool has_alpha_channel; /**< True if image has alpha channel */
	bool is_opaque;         /**< True if image is fully opaque */
};

/**
 * @brief Initialize a wlf_image structure.
 * @param image Pointer to the image structure.
 * @param impl Implementation for format-specific operations.
 * @param width Image width.
 * @param height Image height.
 * @param format Pixel format.
 */
void wlf_image_init(struct wlf_image *image,
	const struct wlf_image_impl *impl, uint32_t width, uint32_t height, uint32_t format);

/**
 * @brief Release resources held by a wlf_image structure.
 * @param image Pointer to the image structure.
 */
void wlf_image_finish(struct wlf_image *image);

/**
 * @brief Get image type from string.
 * @param str String representation of image type.
 * @return Corresponding enum wlf_image_type.
 */
enum wlf_image_type wlf_image_type_from_string(const char *str);

/**
 * @brief Get the string representation of the image type for a given image.
 * @param image Pointer to the wlf_image structure.
 * @return String name of the image type (e.g., "png", "jpeg", "svg", or "unknown").
 */
const char *wlf_image_get_type_string(const struct wlf_image *image);

/**
 * @brief Get the number of channels for the image format.
 * @param image Pointer to the image structure.
 * @return Number of channels (e.g., 3 for RGB, 4 for RGBA).
 */
int wlf_image_get_channels(const struct wlf_image *image);

/**
 * @brief Save an image to a file.
 * @param image Pointer to the wlf_image structure to save.
 * @param filename Path to the file where the image will be saved.
 * @return true on success, false on failure.
 */
bool wlf_image_save(struct wlf_image *image, const char *filename);

/**
 * @brief Load an image from a file.
 * @param filename Path to the image file to load.
 * @return Pointer to a newly allocated wlf_image structure, or NULL on failure.
 */
struct wlf_image *wlf_image_load(const char *filename);

#endif // IMAGE_WLF_IMAGE_H
