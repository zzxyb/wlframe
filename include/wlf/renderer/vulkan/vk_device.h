/**
 * @file        vk_device.h
 * @brief       Vulkan logical device management for wlframe.
 * @details     This file defines the Vulkan device wrapper used by wlframe.
 *              It provides management for Vulkan logical and physical devices,
 *              as well as dynamically loaded device-level extension APIs.
 *
 *              The Vulkan device is responsible for submitting command buffers,
 *              managing GPU queues, synchronization primitives, and importing/exporting
 *              synchronization file descriptors for interop with the Wayland compositor.
 *
 *              All Vulkan-based renderers in wlframe use this device abstraction to
 *              access GPU capabilities in a unified and backend-agnostic manner.
 *
 * @author      YaoBing Xiao
 * @date        2025-11-03
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2025-11-03, initial version.
 */

#ifndef VULKAN_VK_DEVICE_H
#define VULKAN_VK_DEVICE_H

#include <vulkan/vulkan.h>
#include <stdbool.h>

struct wlf_vk_instance;

/**
 * @struct wlf_vk_device
 * @brief Vulkan device wrapper for wlframe.
 *
 * This structure encapsulates both the Vulkan physical and logical device handles,
 * as well as device-specific function pointers loaded dynamically at runtime.
 * It also contains capability flags indicating support for advanced synchronization
 * and YCbCr color conversion.
 */
struct wlf_vk_device {
	VkDevice base;                    /**< Vulkan logical device handle. */
	VkPhysicalDevice phdev;           /**< Vulkan physical device handle. */
	struct wlf_vk_instance *instance; /**< Pointer to the parent Vulkan instance wrapper. */

	bool sync_file_import_export;     /**< Whether the device supports sync file import/export (VK_EXT_external_fd). */
	bool implicit_sync_interop;       /**< Whether implicit synchronization interop is supported. */
	bool sampler_ycbcr_conversion;    /**< Whether the device supports YCbCr sampler conversion. */

	uint32_t queue_family;            /**< Index of the primary queue family used for rendering and transfer. */
	VkQueue queue;                    /**< Primary Vulkan queue used for command submission. */

	/**
	 * @brief Vulkan device-level API function pointers.
	 *
	 * These are loaded dynamically using `vkGetDeviceProcAddr` and provide
	 * access to extended Vulkan features not guaranteed by the core API.
	 */
	struct {
		PFN_vkGetMemoryFdPropertiesKHR vkGetMemoryFdPropertiesKHR;   /**< Retrieves memory FD properties (for DMA-BUF interop). */
		PFN_vkWaitSemaphoresKHR vkWaitSemaphoresKHR;                 /**< Waits on timeline semaphores. */
		PFN_vkGetSemaphoreCounterValueKHR vkGetSemaphoreCounterValueKHR; /**< Queries semaphore counter value. */
		PFN_vkGetSemaphoreFdKHR vkGetSemaphoreFdKHR;                 /**< Exports a semaphore to a file descriptor. */
		PFN_vkImportSemaphoreFdKHR vkImportSemaphoreFdKHR;           /**< Imports a semaphore from a file descriptor. */
		PFN_vkQueueSubmit2KHR vkQueueSubmit2KHR;                     /**< Submits commands using the Vulkan 1.3 submission model. */
	} api;
};

/**
 * @brief Creates a Vulkan device.
 *
 * Selects queue families, creates a logical device, and loads required
 * extension APIs for synchronization and DMA-BUF interoperation.
 *
 * @param instance Pointer to the Vulkan instance wrapper.
 * @param phdev Vulkan physical device handle.
 * @return Pointer to the created Vulkan device wrapper, or NULL on failure.
 */
struct wlf_vk_device *wlf_vk_device_create(struct wlf_vk_instance *instance,
	VkPhysicalDevice phdev);

/**
 * @brief Destroys a Vulkan device.
 *
 * Cleans up the Vulkan logical device, releases queues and extensions,
 * and frees internal resources associated with the device.
 *
 * @param device Pointer to the Vulkan device wrapper to destroy.
 */
void wlf_vk_device_destroy(struct wlf_vk_device *device);

/**
 * @brief Loads a device-level Vulkan function pointer.
 *
 * Dynamically retrieves a Vulkan device procedure address and stores it
 * in the specified function pointer location.
 *
 * @param device Pointer to the Vulkan device.
 * @param name Name of the Vulkan function (e.g. "vkQueueSubmit2KHR").
 * @param proc_ptr Pointer to the location where the function pointer will be stored.
 */
void load_device_proc(struct wlf_vk_device *device, const char *name,
	void *proc_ptr);

/**
 * @brief Finds a suitable Vulkan physical device.
 *
 * Enumerates physical devices from the given Vulkan instance and selects
 * one based on wlframe's preferred criteria (e.g. discrete GPU priority).
 *
 * @param instance Pointer to the Vulkan instance wrapper.
 * @return Vulkan physical device handle, or VK_NULL_HANDLE on failure.
 */
VkPhysicalDevice wlf_vk_find_phdev(struct wlf_vk_instance *instance);

#endif /* VULKAN_VK_DEVICE_H */
