/**
 * @file        wlf_vk_video_buffer.c
 * @brief       Vulkan video buffer implementation.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-26
 */

#include "wlf/va/wlf_vk_video_buffer.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <unistd.h>
#include <vulkan/vulkan_wayland.h>

/* Vulkan video buffer implementation */

static void vk_video_buffer_destroy(struct wlf_buffer *buffer) {
	struct wlf_vk_video_buffer *vk_buffer =
		(struct wlf_vk_video_buffer *)buffer;

	if (vk_buffer->exported && vk_buffer->dma_buf_fd >= 0) {
		close(vk_buffer->dma_buf_fd);
	}

	if (vk_buffer->image_view) {
		vkDestroyImageView(vk_buffer->device, vk_buffer->image_view, NULL);
	}

	if (vk_buffer->image) {
		vkDestroyImage(vk_buffer->device, vk_buffer->image, NULL);
	}

	if (vk_buffer->memory) {
		vkFreeMemory(vk_buffer->device, vk_buffer->memory, NULL);
	}

	wlf_signal_emit(&buffer->events.destroy, buffer);
	free(vk_buffer);
}

static bool vk_video_buffer_begin_data_ptr_access(struct wlf_buffer *buffer,
	uint32_t flags, void **data, uint32_t *format, size_t *stride) {

	/* Vulkan buffers typically don't support direct CPU access */
	wlf_log(WLF_WARN, "Direct CPU access not supported for Vulkan video buffers");
	return false;
}

static void vk_video_buffer_end_data_ptr_access(struct wlf_buffer *buffer) {
	/* No-op */
}

static const struct wlf_region *vk_video_buffer_opaque_region(
	struct wlf_buffer *buffer) {
	return NULL;  /* Fully opaque */
}

static struct wl_buffer *vk_video_buffer_export_to_wl_buffer(
	struct wlf_video_buffer *buffer, struct wl_display *wl_display) {

	struct wlf_vk_video_buffer *vk_buffer =
		wlf_vk_video_buffer_from_video_buffer(buffer);

	if (!vk_buffer) {
		return NULL;
	}

	/* Export Vulkan memory as DMA-BUF */
	PFN_vkGetMemoryFdKHR vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)
		vkGetDeviceProcAddr(vk_buffer->device, "vkGetMemoryFdKHR");

	if (!vkGetMemoryFdKHR) {
		wlf_log(WLF_ERROR, "VK_KHR_external_memory_fd not supported");
		return NULL;
	}

	VkMemoryGetFdInfoKHR fd_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
		.memory = vk_buffer->memory,
		.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
	};

	int dma_buf_fd;
	VkResult result = vkGetMemoryFdKHR(vk_buffer->device, &fd_info, &dma_buf_fd);
	if (result != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to export DMA-BUF: %d", result);
		return NULL;
	}

	vk_buffer->dma_buf_fd = dma_buf_fd;
	vk_buffer->exported = true;

	wlf_log(WLF_DEBUG, "Exported Vulkan buffer as DMA-BUF fd: %d", dma_buf_fd);

	/* TODO: Complete linux-dmabuf protocol integration */
	wlf_log(WLF_WARN, "Vulkan wl_buffer export pending linux-dmabuf implementation");

	return NULL;
}

const struct wlf_video_buffer_impl vk_video_buffer_impl = {
	.base = {
		.destroy = vk_video_buffer_destroy,
		.begin_data_ptr_access = vk_video_buffer_begin_data_ptr_access,
		.end_data_ptr_access = vk_video_buffer_end_data_ptr_access,
		.opaque_region = vk_video_buffer_opaque_region,
	},
	.export_to_wl_buffer = vk_video_buffer_export_to_wl_buffer,
};

struct wlf_vk_video_buffer *wlf_vk_video_buffer_create(
	VkDevice device,
	VkPhysicalDevice physical_device,
	uint32_t width, uint32_t height,
	VkFormat format) {

	struct wlf_vk_video_buffer *buffer = calloc(1, sizeof(*buffer));
	if (!buffer) {
		wlf_log(WLF_ERROR, "Failed to allocate Vulkan video buffer");
		return NULL;
	}

	wlf_video_buffer_init(&buffer->base, &vk_video_buffer_impl,
		width, height);

	buffer->device = device;
	buffer->physical_device = physical_device;
	buffer->format = format;
	buffer->dma_buf_fd = -1;
	buffer->exported = false;

	/* TODO: Create Vulkan image and allocate memory */

	return buffer;
}
