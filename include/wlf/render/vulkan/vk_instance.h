#ifndef VULKAN_VK_INSTANCE_H
#define VULKAN_VK_INSTANCE_H

#include <vulkan/vulkan.h>

struct wlf_vk_instance {
	VkInstance base;
	VkDebugUtilsMessengerEXT messenger;

	struct {
		PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT;
		PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT;
	} api;
};

struct wlf_vk_instance *wlf_vk_instance_create(bool debug);
void wlf_vk_instance_destroy(struct wlf_vk_instance *instance);

#endif // VULKAN_VK_INSTANCE_H
