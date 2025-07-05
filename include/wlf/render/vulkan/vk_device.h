#ifndef VULKAN_VK_DEVICE_H
#define VULKAN_VK_DEVICE_H

#include <vulkan/vulkan.h>

struct wlf_vk_instance;

struct wlf_vk_device {
	struct wlf_vk_instance *instance;

	VkPhysicalDevice phdev;
	VkDevice dev;

	bool sync_file_import_export;
	bool implicit_sync_interop;
	bool sampler_ycbcr_conversion;

	uint32_t queue_family;
	VkQueue queue;

	struct {
		PFN_vkGetMemoryFdPropertiesKHR vkGetMemoryFdPropertiesKHR;
		PFN_vkWaitSemaphoresKHR vkWaitSemaphoresKHR;
		PFN_vkGetSemaphoreCounterValueKHR vkGetSemaphoreCounterValueKHR;
		PFN_vkGetSemaphoreFdKHR vkGetSemaphoreFdKHR;
		PFN_vkImportSemaphoreFdKHR vkImportSemaphoreFdKHR;
		PFN_vkQueueSubmit2KHR vkQueueSubmit2KHR;
	} api;
};

struct wlf_vk_device *wlf_vk_device_create(struct wlf_vk_instance *ini,
	VkPhysicalDevice phdev);
void wlf_vk_device_destroy(struct wlf_vk_device *device);
void load_device_proc(struct wlf_vk_device *dev, const char *name,
	void *proc_ptr);
VkPhysicalDevice wlf_vk_find_phdev(struct wlf_vk_instance *ini);

#endif // VULKAN_VK_DEVICE_H
