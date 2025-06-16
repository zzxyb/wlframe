/**
 * @file        wlf_svg_image.h
 * @brief       SVG image handling and utility functions for wlframe.
 * @details     This file defines the wlf_svg_image structure and related functions,
 *              providing a unified interface for creating, loading, saving, and
 *              rendering SVG images within wlframe. It supports multiple rendering
 *              backends including pixman, OpenGL ES, and Vulkan.
 *
 *              Typical usage:
 *                  - Create an SVG image object.
 *                  - Load SVG from file or convert from wlf_image.
 *                  - Render SVG using selected backend (pixman, OpenGL ES, Vulkan).
 *                  - Save rendered result or SVG content to file.
 *
 * @author      YaoBing Xiao
 * @date        2025-01-17
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-01-17, initial version\n
 */

#ifndef IMAGE_WLF_SVG_IMAGE_H
#define IMAGE_WLF_SVG_IMAGE_H

#include "wlf/image/wlf_image.h"
#include <stddef.h>

/**
 * @brief Supported SVG rendering backends.
 */
enum wlf_svg_backend_type {
	WLF_SVG_BACKEND_PIXMAN,   /**< Pixman-based CPU rendering */
	WLF_SVG_BACKEND_GLES,     /**< OpenGL ES-based GPU rendering */
	WLF_SVG_BACKEND_VULKAN,   /**< Vulkan-based GPU rendering */
	WLF_SVG_BACKEND_AUTO,     /**< Automatically select best available backend */
};

/**
 * @brief SVG rendering context for backend operations.
 */
struct wlf_svg_render_context;

/**
 * @brief SVG rendering backend interface.
 */
struct wlf_svg_backend {
	enum wlf_svg_backend_type type;

	/**
	 * Initialize the rendering backend.
	 * @param context Rendering context to initialize.
	 * @param width Target render width.
	 * @param height Target render height.
	 * @return true on success, false on failure.
	 */
	bool (*init)(struct wlf_svg_render_context *context, uint32_t width, uint32_t height);

	/**
	 * Render SVG content to the target surface.
	 * @param context Rendering context.
	 * @param svg_data SVG content as string.
	 * @param target_image Target image to render into.
	 * @return true on success, false on failure.
	 */
	bool (*render)(struct wlf_svg_render_context *context, const char *svg_data, struct wlf_image *target_image);

	/**
	 * Clean up the rendering backend.
	 * @param context Rendering context to clean up.
	 */
	void (*cleanup)(struct wlf_svg_render_context *context);
};

/**
 * @brief SVG image structure, extending wlf_image with SVG-specific data.
 */
struct wlf_svg_image {
	struct wlf_image base;                /**< Base image structure */
	char *svg_data;                       /**< SVG content as string */
	size_t svg_data_size;                 /**< Size of SVG data */
	float scale_x;                        /**< Horizontal scaling factor */
	float scale_y;                        /**< Vertical scaling factor */
	uint32_t render_width;                /**< Target render width */
	uint32_t render_height;               /**< Target render height */
	enum wlf_svg_backend_type backend_type; /**< Selected rendering backend */
	struct wlf_svg_render_context *render_context; /**< Backend-specific render context */
};

/**
 * @brief Create a new wlf_svg_image object.
 * @return Pointer to a newly allocated wlf_svg_image structure, or NULL on failure.
 */
struct wlf_svg_image *wlf_svg_image_create(void);

/**
 * @brief Check if a wlf_image is an SVG image.
 * @param image Pointer to the wlf_image structure.
 * @return true if the image is an SVG image, false otherwise.
 */
bool wlf_is_svg_image(const struct wlf_image *image);

/**
 * @brief Convert from wlf_image to wlf_svg_image.
 * @param image Pointer to the wlf_image structure.
 * @return Pointer to wlf_svg_image, or NULL if not an SVG image.
 */
struct wlf_svg_image *wlf_svg_image_from_image(struct wlf_image *image);

/**
 * @brief Load SVG image from file.
 * @param svg_image Pointer to the wlf_svg_image structure.
 * @param filename Path to the SVG file.
 * @return true on success, false on failure.
 */
bool wlf_svg_image_load_from_file(struct wlf_svg_image *svg_image, const char *filename);

/**
 * @brief Load SVG image from string data.
 * @param svg_image Pointer to the wlf_svg_image structure.
 * @param svg_data SVG content as string.
 * @param data_size Size of SVG data.
 * @return true on success, false on failure.
 */
bool wlf_svg_image_load_from_data(struct wlf_svg_image *svg_image, const char *svg_data, size_t data_size);

/**
 * @brief Save SVG image to file.
 * @param svg_image Pointer to the wlf_svg_image structure.
 * @param filename Path to save the SVG file.
 * @return true on success, false on failure.
 */
bool wlf_svg_image_save_to_file(struct wlf_svg_image *svg_image, const char *filename);

/**
 * @brief Set rendering backend for SVG image.
 * @param svg_image Pointer to the wlf_svg_image structure.
 * @param backend_type Type of rendering backend to use.
 * @return true on success, false on failure.
 */
bool wlf_svg_image_set_backend(struct wlf_svg_image *svg_image, enum wlf_svg_backend_type backend_type);

/**
 * @brief Set target render size for SVG image.
 * @param svg_image Pointer to the wlf_svg_image structure.
 * @param width Target render width.
 * @param height Target render height.
 */
void wlf_svg_image_set_render_size(struct wlf_svg_image *svg_image, uint32_t width, uint32_t height);

/**
 * @brief Render SVG to raster image.
 * @param svg_image Pointer to the wlf_svg_image structure.
 * @param target_image Target image to render into.
 * @return true on success, false on failure.
 */
bool wlf_svg_image_render(struct wlf_svg_image *svg_image, struct wlf_image *target_image);

/**
 * @brief Get available SVG rendering backends.
 * @param backends Array to store available backend types.
 * @param max_backends Maximum number of backends to return.
 * @return Number of available backends.
 */
int wlf_svg_get_available_backends(enum wlf_svg_backend_type *backends, int max_backends);

/**
 * @brief Get backend name string.
 * @param backend_type Backend type.
 * @return String name of the backend.
 */
const char *wlf_svg_backend_name(enum wlf_svg_backend_type backend_type);

/**
 * @brief Destroy an SVG image and free its resources.
 * @param svg_image Pointer to the wlf_svg_image structure.
 */
void wlf_svg_image_destroy(struct wlf_svg_image *svg_image);

#endif // IMAGE_WLF_SVG_IMAGE_H
