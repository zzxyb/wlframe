#ifndef TEXTURE_WLF_VK_TEXTURE_H
#define TEXTURE_WLF_VK_TEXTURE_H

#include "wlf/texture/wlf_texture.h"

#include <vulkan/vulkan.h>

struct wlf_vk_renderer;

struct wlf_vk_texture {
	struct wlf_texture base;
	struct wlf_vk_renderer *renderer;
	uint32_t mem_count;
	VkDeviceMemory memory;
	VkImage image;
	const struct wlf_vk_format *format;
	struct wlf_vk_command_buffer *last_used_cb;
	bool has_alpha; // whether the image is has alpha channel
};

// ===== Utility Functions =====

/**
 * @brief Get Vulkan image from a generic texture
 * @param texture Generic texture pointer
 * @return Vulkan image handle, or VK_NULL_HANDLE if not a VK texture
 */
VkImage wlf_texture_get_vk_image(struct wlf_texture* texture);

#endif // TEXTURE_WLF_VK_TEXTURE_H
