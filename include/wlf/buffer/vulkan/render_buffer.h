
#ifndef VULKAN_RENDER_BUFFER_H
#define VULKAN_RENDER_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/renderer/vulkan/renderer.h"

struct wlf_vk_render_buffer {
	struct wlf_buffer *wlf_buffer;
	struct wlf_vk_renderer *renderer;
	struct wlf_linked_list link; // wlf_vk_renderer.buffers

	VkImage image;
	VkImageView image_view;
	VkFramebuffer framebuffer;
	VkFormat format;
};

struct wlf_vk_render_buffer *wlf_vk_render_buffer_create(
	struct wlf_vk_renderer *renderer, struct wlf_buffer *wlf_buffer);
void wlf_vk_render_buffer_destroy(struct wlf_vk_render_buffer *buffer);
struct wlf_vk_render_buffer *wlf_vk_render_buffer_get(
	struct wlf_vk_renderer *renderer, struct wlf_buffer *wlf_buffer);

#endif /* VULKAN_RENDER_BUFFER_H */
