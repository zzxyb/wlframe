/**
 * @file        vk_instance.h
 * @brief       Vulkan instance management for wlframe.
 * @details     This file defines the Vulkan instance wrapper used by wlframe.
 *              It encapsulates the Vulkan `VkInstance` object, debug utilities,
 *              and dynamically loaded extension functions. The Vulkan instance
 *              serves as the root object for all Vulkan operations, including
 *              device creation, surface management, and debug callbacks.
 *
 *              The structure and functions provided here simplify instance
 *              creation, debug layer setup, and proper resource cleanup.
 *
 * @author      YaoBing Xiao
 * @date        2025-11-03
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2025-11-03, initial version.
 */

#ifndef VULKAN_VK_INSTANCE_H
#define VULKAN_VK_INSTANCE_H

#include <vulkan/vulkan.h>
#include <stdbool.h>

/**
 * @struct wlf_vk_instance
 * @brief Vulkan instance wrapper for wlframe.
 *
 * This structure wraps the Vulkan instance handle and provides
 * access to dynamically loaded extension entry points (such as
 * debug utilities). It is typically created once per wlframe backend
 * and used across all Vulkan renderers and devices.
 */
struct wlf_vk_instance {
	VkInstance base; /**< Vulkan instance handle. */

	VkDebugUtilsMessengerEXT messenger; /**< Optional debug messenger for validation layer logging. */

	/**
	 * @brief Vulkan API function pointers for optional extensions.
	 *
	 * These are loaded at runtime using `vkGetInstanceProcAddr` to
	 * ensure compatibility across systems with or without validation layers.
	 */
	struct {
		PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT;   /**< Function pointer to `vkCreateDebugUtilsMessengerEXT`. */
		PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT; /**< Function pointer to `vkDestroyDebugUtilsMessengerEXT`. */
	} api;
};

/**
 * @brief Creates a Vulkan instance.
 *
 * Initializes a Vulkan instance, optionally enabling validation layers and
 * debug utilities when `debug` is set to true.
 *
 * @param debug Whether to enable Vulkan validation layers and debug utilities.
 * @return Pointer to the created Vulkan instance wrapper, or NULL on failure.
 */
struct wlf_vk_instance *wlf_vk_instance_create(bool debug);

/**
 * @brief Destroys a Vulkan instance.
 *
 * Cleans up the Vulkan instance and any associated debug utilities.
 * After destruction, the pointer becomes invalid and must not be reused.
 *
 * @param instance Pointer to the Vulkan instance wrapper to destroy.
 */
void wlf_vk_instance_destroy(struct wlf_vk_instance *instance);

#endif /* VULKAN_VK_INSTANCE_H */
