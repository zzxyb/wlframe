#ifndef RENDER_WLF_VK_RENDER_H
#define RENDER_WLF_VK_RENDER_H

#include "wlf/render/wlf_render.h"
#include "wlf/utils/wlf_log.h"

#include <vulkan/vulkan.h>

#include <stdbool.h>

struct wlf_vk_render {
	struct wlf_render base;
};

struct wlf_vk_instance {
	VkInstance instance;
	VkDebugUtilsMessengerEXT messenger;

	struct {
		PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT;
		PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT;
	} ext;
};

struct wlf_vk_instance *wlf_vk_instance_create(bool debug);
void wlf_vk_instance_destroy(struct wlf_vk_instance *instance);

struct wlf_render *wlf_vk_renderer_create(void);

#if __STDC_VERSION__ >= 202311L

#define wlf_vk_error(fmt, res, ...) wlf_log(WLF_ERROR, fmt ": %s (%d)", \
	vulkan_strerror(res), res __VA_OPT__(,) __VA_ARGS__)

#else

#define wlf_vk_error(fmt, res, ...) wlf_log(WLF_ERROR, fmt ": %s (%d)", \
	vulkan_strerror(res), res, ##__VA_ARGS__)

#endif

#endif // RENDER_WLF_VK_RENDER_H
