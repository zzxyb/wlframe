/**
 * @file        wlf_vk_video_buffer.h
 * @brief       Vulkan video buffer implementation.
 * @details     This file defines Vulkan-based video buffer type.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-26
 * @version     v2.1
 * @par Copyright:
 * @par History:
 *      version: v2.1, YaoBing Xiao, 2026-01-26, split into separate file\n
 */

#ifndef VA_WLF_VK_VIDEO_BUFFER_H
#define VA_WLF_VK_VIDEO_BUFFER_H

#include "wlf/va/wlf_video_buffer.h"
#include <vulkan/vulkan.h>

/**
 * @struct wlf_vk_video_buffer
 * @brief Vulkan video buffer.
 */
struct wlf_vk_video_buffer {
	struct wlf_video_buffer base;         /**< Base video buffer */

	/* Vulkan resources */
	VkDevice device;
	VkPhysicalDevice physical_device;
	VkImage image;
	VkDeviceMemory memory;
	VkImageView image_view;
	VkFormat format;

	/* DMA-BUF export support */
	int dma_buf_fd;
	bool exported;
};

/**
 * @brief Create a Vulkan video buffer.
 *
 * @param device Vulkan device.
 * @param physical_device Vulkan physical device.
 * @param width Buffer width.
 * @param height Buffer height.
 * @param format Vulkan image format.
 * @return Pointer to created buffer, or NULL on failure.
 */
struct wlf_vk_video_buffer *wlf_vk_video_buffer_create(
	VkDevice device,
	VkPhysicalDevice physical_device,
	uint32_t width, uint32_t height,
	VkFormat format);

/**
 * @brief Get Vulkan video buffer from base video buffer.
 *
 * @param buffer Base video buffer.
 * @return Vulkan video buffer, or NULL if not a Vulkan buffer.
 */
static inline struct wlf_vk_video_buffer *wlf_vk_video_buffer_from_video_buffer(
	struct wlf_video_buffer *buffer);

/* Vulkan video buffer implementation */
extern const struct wlf_video_buffer_impl vk_video_buffer_impl;

/* Inline implementation */
static inline struct wlf_vk_video_buffer *wlf_vk_video_buffer_from_video_buffer(
	struct wlf_video_buffer *buffer) {
	if (buffer && buffer->impl == &vk_video_buffer_impl) {
		return (struct wlf_vk_video_buffer *)buffer;
	}
	return NULL;
}

#endif /* VA_WLF_VK_VIDEO_BUFFER_H */
