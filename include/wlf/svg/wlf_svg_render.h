/**
 * @file        wlf_svg_render.h
 * @brief       SVG rendering backend implementations for wlframe.
 * @details     This file defines the implementation structures and functions for
 *              different SVG rendering backends including pixman, OpenGL ES, and
 *              Vulkan. Each backend provides hardware-appropriate rendering
 *              optimizations for SVG content.
 *
 * @author      YaoBing Xiao
 * @date        2025-01-17
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-01-17, initial version\n
 */

#ifndef SVG_WLF_SVG_RENDER_H
#define SVG_WLF_SVG_RENDER_H

#include "wlf/image/wlf_svg_image.h"
#include <stddef.h>

#ifdef WLF_HAS_PIXMAN
#include <pixman.h>
#endif

#ifdef WLF_HAS_GLES
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#endif

#ifdef WLF_HAS_VULKAN
#include <vulkan/vulkan.h>
#endif

/**
 * @brief Common SVG rendering context.
 */
struct wlf_svg_render_context {
	enum wlf_svg_backend_type backend_type;
	uint32_t width;
	uint32_t height;
	void *backend_data;  /**< Backend-specific data */
};

#ifdef WLF_HAS_PIXMAN
/**
 * @brief Pixman-specific rendering context.
 */
struct wlf_svg_pixman_context {
	pixman_image_t *target_image;
	pixman_image_t *source_image;
	pixman_format_code_t format;
};

/**
 * @brief Get Pixman SVG rendering backend.
 * @return Pointer to Pixman backend implementation.
 */
const struct wlf_svg_backend *wlf_svg_get_pixman_backend(void);
#endif

#ifdef WLF_HAS_GLES
/**
 * @brief OpenGL ES-specific rendering context.
 */
struct wlf_svg_gles_context {
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	GLuint framebuffer;
	GLuint texture;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program;
};

/**
 * @brief Get OpenGL ES SVG rendering backend.
 * @return Pointer to OpenGL ES backend implementation.
 */
const struct wlf_svg_backend *wlf_svg_get_gles_backend(void);
#endif

#ifdef WLF_HAS_VULKAN
/**
 * @brief Vulkan-specific rendering context.
 */
struct wlf_svg_vulkan_context {
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkQueue graphics_queue;
	VkCommandPool command_pool;
	VkCommandBuffer command_buffer;
	VkRenderPass render_pass;
	VkFramebuffer framebuffer;
	VkImage target_image;
	VkDeviceMemory target_memory;
	VkImageView target_view;
	VkPipeline graphics_pipeline;
	VkPipelineLayout pipeline_layout;
};

/**
 * @brief Get Vulkan SVG rendering backend.
 * @return Pointer to Vulkan backend implementation.
 */
const struct wlf_svg_backend *wlf_svg_get_vulkan_backend(void);
#endif

/**
 * @brief Initialize SVG rendering subsystem.
 * @return true on success, false on failure.
 */
bool wlf_svg_render_init(void);

/**
 * @brief Cleanup SVG rendering subsystem.
 */
void wlf_svg_render_cleanup(void);

/**
 * @brief Auto-select best available SVG rendering backend.
 * @return Pointer to best available backend, or NULL if none available.
 */
const struct wlf_svg_backend *wlf_svg_get_auto_backend(void);

/**
 * @brief Parse SVG and extract dimensions.
 * @param svg_data SVG content as string.
 * @param width Pointer to store extracted width.
 * @param height Pointer to store extracted height.
 * @return true on success, false on failure.
 */
bool wlf_svg_parse_dimensions(const char *svg_data, uint32_t *width, uint32_t *height);

#endif // SVG_WLF_SVG_RENDER_H
