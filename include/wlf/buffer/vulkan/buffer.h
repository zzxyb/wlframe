/**
 * @file        buffer.h
 * @brief       Vulkan render-buffer structures for wlframe.
 */

#ifndef VULKAN_BUFFER_H
#define VULKAN_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/allocator/vulkan/allocator.h"
#include "wlf/utils/wlf_addon.h"
#include "wlf/utils/wlf_linked_list.h"

#include <gbm.h>
#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

struct wlf_vk_buffer {
	struct wlf_buffer *wlf_buffer;
	struct wlf_addon addon;
	struct wlf_vk_renderer *renderer;
	VkImage image;
	VkImageView image_view;
	VkFramebuffer framebuffer;
	struct wlf_linked_list link; /* wlf_vk_renderer.buffers */
};

struct wlf_vk_buffer *wlf_vk_buffer_create(struct wlf_vk_allocator *alloc,
	int width, int height, uint32_t format);

struct wlf_vk_buffer *wlf_vk_buffer_from_buffer(struct wlf_buffer *wlf_buffer);
bool wlf_buffer_is_vk(const struct wlf_buffer *buffer);

#endif // VULKAN_BUFFER_H
