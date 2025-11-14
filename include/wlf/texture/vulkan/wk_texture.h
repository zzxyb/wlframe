#ifndef VULKAN_VK_TEXTURE_H
#define VULKAN_VK_TEXTURE_H

#include "wlf/texture/wlf_texture.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"

#include <vulkan/vulkan.h>

#define WLF_DMABUF_MAX_PLANES 4

struct wlf_renderer;

struct wlf_vk_format {
	VkFormat vk;
	VkFormat vk_srgb; // sRGB version of the format, or 0 if nonexistent
	bool is_ycbcr;
};

struct wlf_vk_texture {
	struct wlf_texture base;
	struct wlf_renderer *render;
	struct wlf_linked_list link;

	VkDeviceMemory memories[WLF_DMABUF_MAX_PLANES];
	VkImage image;

	const struct wlr_vk_format *format;
};

#endif // VULKAN_VK_TEXTURE_H
