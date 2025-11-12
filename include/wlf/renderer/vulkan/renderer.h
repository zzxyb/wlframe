/**
 * @file        renderer.h
 * @brief       Vulkan renderer backend implementation for wlframe.
 * @details     This file defines the Vulkan-based renderer implementation for wlframe.
 *              It extends the generic `wlf_renderer` interface to provide hardware-accelerated
 *              rendering through the Vulkan API. This backend manages Vulkan devices,
 *              command pools, synchronization primitives, and integrates with wlframe’s
 *              renderer abstraction.
 *
 * @author      YaoBing Xiao
 * @date        2025-11-03
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2025-11-03, initial version.
 */

#ifndef VULKAN_VK_RENDERER_H
#define VULKAN_VK_RENDERER_H

#include "wlf/renderer/wlf_renderer.h"
#include "wlf/utils/wlf_log.h"

#include <vulkan/vulkan.h>

#include <stdbool.h>

struct wlf_vk_instance;
struct wlf_vk_device;
struct wlf_backend;

/**
 * @struct wlf_vk_renderer
 * @brief Vulkan renderer implementation for wlframe.
 *
 * This structure represents a Vulkan-based renderer, derived from `wlf_renderer`.
 * It encapsulates Vulkan resources, device handles, command pools, and synchronization
 * objects required for GPU rendering.
 */
struct wlf_vk_renderer {
	struct wlf_renderer base;        /**< Base renderer interface (must be first for inheritance). */
	struct wlf_backend *backend;     /**< Associated wlframe backend. */
	struct wlf_vk_device *dev;       /**< Vulkan logical device wrapper. */

	VkCommandPool command_pool;      /**< Vulkan command pool for rendering command buffers. */

	VkSemaphore timeline_semaphore;  /**< Vulkan timeline semaphore used for frame synchronization. */
};

/**
 * @brief Creates a Vulkan renderer from a wlframe backend.
 *
 * This function initializes the Vulkan renderer by creating a Vulkan instance,
 * selecting a suitable device, and allocating necessary GPU resources.
 *
 * @param backend Pointer to the wlframe backend.
 * @return Pointer to the created `wlf_renderer`, or NULL on failure.
 */
struct wlf_renderer *wlf_vk_renderer_create_from_backend(
	struct wlf_backend *backend);

/**
 * @brief Destroys a Vulkan renderer instance.
 *
 * Releases Vulkan resources including command pools, semaphores, and device handles.
 * Should be called when the Vulkan renderer is no longer needed.
 *
 * @param vk_render Pointer to the Vulkan renderer instance.
 */
void wlf_vk_renderer_destroy(struct wlf_vk_renderer *vk_render);

/**
 * @brief Checks whether a renderer is a Vulkan renderer.
 *
 * @param wlf_renderer Pointer to a generic renderer instance.
 * @return true if the renderer uses Vulkan as backend, false otherwise.
 */
bool wlf_renderer_is_vk(struct wlf_renderer *wlf_renderer);

/**
 * @brief Converts a generic renderer pointer to a Vulkan renderer pointer.
 *
 * This function safely casts a `wlf_renderer` to its derived `wlf_vk_renderer`
 * type if the renderer is Vulkan-based.
 *
 * @param wlf_renderer Pointer to a generic renderer.
 * @return Pointer to the Vulkan renderer, or NULL if not Vulkan-based.
 */
struct wlf_vk_renderer *wlf_vk_renderer_from_render(struct wlf_renderer *wlf_renderer);

/**
 * @brief Checks if a Vulkan extension is available.
 *
 * This helper function checks for the existence of a Vulkan extension
 * among a list of available extensions.
 *
 * @param avail Array of available Vulkan extension properties.
 * @param avail_len Number of available extensions.
 * @param name Name of the extension to check.
 * @return true if the extension is found, false otherwise.
 */
bool check_extension(const VkExtensionProperties *avail,
	uint32_t avail_len, const char *name);

/**
 * @brief Creates a Vulkan renderer for a specific Vulkan device.
 *
 * Similar to `wlf_vk_renderer_create_from_backend`, but explicitly takes
 * a `wlf_vk_device` instead of inferring one from the backend.
 *
 * @param device Pointer to a Vulkan device wrapper.
 * @return Pointer to the created `wlf_renderer`, or NULL on failure.
 */
struct wlf_renderer *wlr_vk_render_create_for_device(struct wlf_vk_device *device);

/**
 * @brief Returns a human-readable Vulkan error string.
 *
 * Converts a Vulkan `VkResult` code into a descriptive string for
 * logging and debugging purposes.
 *
 * @param err Vulkan result code.
 * @return Pointer to a static string describing the error.
 */
const char *wlf_vulkan_strerror(VkResult err);

#if __STDC_VERSION__ >= 202311L
/**
 * @brief Logging macro for Vulkan errors (C23 and later).
 *
 * Formats a Vulkan error message with a human-readable error string.
 *
 * @param fmt Log message format string.
 * @param res Vulkan result code.
 * @param ... Additional arguments for formatting.
 */
#define wlf_vk_error(fmt, res, ...) wlf_log(WLF_ERROR, fmt ": %s (%d)", \
	wlf_vulkan_strerror(res), res __VA_OPT__(,) __VA_ARGS__)
#else
/**
 * @brief Logging macro for Vulkan errors (pre-C23).
 *
 * Same as above, but compatible with compilers not supporting `__VA_OPT__`.
 *
 * @param fmt Log message format string.
 * @param res Vulkan result code.
 * @param ... Additional arguments for formatting.
 */
#define wlf_vk_error(fmt, res, ...) wlf_log(WLF_ERROR, fmt ": %s (%d)", \
	wlf_vulkan_strerror(res), res, ##__VA_ARGS__)
#endif

#endif // VULKAN_VK_RENDERER_H
