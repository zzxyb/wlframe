#ifndef BUFFER_VULKAN_WLF_VK_BUFFER_H
#define BUFFER_VULKAN_WLF_VK_BUFFER_H

struct wlf_vk_render;
struct wlf_buffer;

struct wlf_vk_buffer {
	struct wlf_buffer *buffer;
	struct wlf_vk_render *render;
};

#endif // BUFFER_VULKAN_WLF_VK_BUFFER_H
