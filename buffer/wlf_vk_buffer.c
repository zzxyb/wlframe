/**
 * @file        wlf_vk_buffer.c
 * @brief       Vulkan render buffer implementation.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 */

#include "wlf/buffer/wlf_vk_buffer.h"
#include "wlf/buffer/wlf_gbm_buffer.h"
#include "wlf/buffer/wlf_shm_buffer.h"
#include "wlf/renderer/vulkan/renderer.h"
#include "wlf/renderer/vulkan/device.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Convert DRM format to Vulkan format.
 */
static VkFormat drm_format_to_vk(uint32_t drm_format) {
	// Simple mapping for common formats
	// A complete implementation would have a full format table
	switch (drm_format) {
		case 0x34325258: // DRM_FORMAT_XRGB8888
			return VK_FORMAT_B8G8R8A8_UNORM;
		case 0x34324152: // DRM_FORMAT_ARGB8888
			return VK_FORMAT_B8G8R8A8_UNORM;
		case 0x34325241: // DRM_FORMAT_ABGR8888
			return VK_FORMAT_R8G8B8A8_UNORM;
		case 0x34325258: // DRM_FORMAT_XBGR8888
			return VK_FORMAT_R8G8B8A8_UNORM;
		default:
			wlf_log(WLF_ERROR, "Unsupported DRM format: 0x%08X", drm_format);
			return VK_FORMAT_UNDEFINED;
	}
}

/**
 * @brief Import DMA-BUF as Vulkan external memory.
 */
static bool import_dmabuf(struct wlf_vk_buffer *vk_buffer,
		struct wlf_dmabuf_attributes *dmabuf) {
	struct wlf_vk_renderer *renderer = vk_buffer->renderer;
	struct wlf_vk_device *dev = renderer->dev;
	VkDevice device = dev->base;

	vk_buffer->format = drm_format_to_vk(dmabuf->format);
	if (vk_buffer->format == VK_FORMAT_UNDEFINED) {
		return false;
	}

	// Create Vulkan image
	VkExternalMemoryImageCreateInfo external_info = {
		.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
		.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
	};

	VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = &external_info,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = vk_buffer->format,
		.extent = {
			.width = dmabuf->width,
			.height = dmabuf->height,
			.depth = 1,
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		         VK_IMAGE_USAGE_SAMPLED_BIT |
		         VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VkResult res = vkCreateImage(device, &image_info, NULL, &vk_buffer->image);
	if (res != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "vkCreateImage failed: %d", res);
		return false;
	}

	// Import external memory for each plane
	vk_buffer->mem_count = dmabuf->n_planes;
	for (uint32_t i = 0; i < dmabuf->n_planes; i++) {
		VkMemoryFdPropertiesKHR fd_props = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR,
		};

		// Note: This requires VK_KHR_external_memory_fd extension
		// res = dev->api.vkGetMemoryFdPropertiesKHR(device,
		// 	VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
		// 	dmabuf->fd[i], &fd_props);
		// Simplified for now - assume memory type 0

		VkMemoryRequirements mem_reqs;
		vkGetImageMemoryRequirements(device, vk_buffer->image, &mem_reqs);

		VkImportMemoryFdInfoKHR import_info = {
			.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR,
			.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
			.fd = dup(dmabuf->fd[i]), // Duplicate FD as Vulkan takes ownership
		};

		VkMemoryAllocateInfo alloc_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = &import_info,
			.allocationSize = mem_reqs.size,
			.memoryTypeIndex = 0, // Simplified
		};

		res = vkAllocateMemory(device, &alloc_info, NULL, &vk_buffer->memories[i]);
		if (res != VK_SUCCESS) {
			wlf_log(WLF_ERROR, "vkAllocateMemory failed: %d", res);
			// Cleanup allocated memories
			for (uint32_t j = 0; j < i; j++) {
				vkFreeMemory(device, vk_buffer->memories[j], NULL);
			}
			vkDestroyImage(device, vk_buffer->image, NULL);
			return false;
		}
	}

	// Bind memory to image
	res = vkBindImageMemory(device, vk_buffer->image, vk_buffer->memories[0], 0);
	if (res != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "vkBindImageMemory failed: %d", res);
		for (uint32_t i = 0; i < vk_buffer->mem_count; i++) {
			vkFreeMemory(device, vk_buffer->memories[i], NULL);
		}
		vkDestroyImage(device, vk_buffer->image, NULL);
		return false;
	}

	vk_buffer->layout = VK_IMAGE_LAYOUT_UNDEFINED;
	vk_buffer->externally_imported = true;

	wlf_log(WLF_DEBUG, "Imported DMA-BUF as Vulkan image: %dx%d, format 0x%08X",
		dmabuf->width, dmabuf->height, dmabuf->format);

	return true;
}

/**
 * @brief Handle buffer destroy event.
 */
static void handle_buffer_destroy(struct wlf_listener *listener, void *data) {
	(void)data;
	struct wlf_vk_buffer *vk_buffer = wlf_container_of(listener,
		struct wlf_vk_buffer, wlf_buffer);
	wlf_vk_buffer_destroy(vk_buffer);
}

struct wlf_vk_buffer *wlf_vk_buffer_create(struct wlf_vk_renderer *renderer,
		struct wlf_buffer *buffer) {
	assert(renderer != NULL && buffer != NULL);

	struct wlf_vk_buffer *vk_buffer = calloc(1, sizeof(*vk_buffer));
	if (vk_buffer == NULL) {
		return NULL;
	}

	vk_buffer->wlf_buffer = buffer;
	vk_buffer->renderer = renderer;
	vk_buffer->image = VK_NULL_HANDLE;
	vk_buffer->mem_count = 0;

	// Try to get DMA-BUF attributes if it's a GBM buffer
	struct wlf_gbm_buffer *gbm_buffer = wlf_gbm_buffer_from_buffer(buffer);
	if (gbm_buffer != NULL) {
		struct wlf_dmabuf_attributes dmabuf;
		if (wlf_gbm_buffer_get_dmabuf(gbm_buffer, &dmabuf)) {
			if (!import_dmabuf(vk_buffer, &dmabuf)) {
				free(vk_buffer);
				return NULL;
			}
		} else {
			wlf_log(WLF_ERROR, "Failed to get DMA-BUF attributes from GBM buffer");
			free(vk_buffer);
			return NULL;
		}
	} else {
		// For SHM buffers or other types, we would need different handling
		wlf_log(WLF_ERROR, "Unsupported buffer type for Vulkan import");
		free(vk_buffer);
		return NULL;
	}

	// Add to renderer's buffer list
	wlf_linked_list_insert(&renderer->buffers, &vk_buffer->link);

	// Listen for buffer destruction
	// Note: This would require adding a listener field to wlf_vk_buffer
	// and proper event handling in wlf_buffer

	wlf_log(WLF_DEBUG, "Created Vulkan render buffer");
	return vk_buffer;
}

void wlf_vk_buffer_destroy(struct wlf_vk_buffer *vk_buffer) {
	if (vk_buffer == NULL) {
		return;
	}

	struct wlf_vk_device *dev = vk_buffer->renderer->dev;
	VkDevice device = dev->base;

	// Free device memory
	for (uint32_t i = 0; i < vk_buffer->mem_count; i++) {
		if (vk_buffer->memories[i] != VK_NULL_HANDLE) {
			vkFreeMemory(device, vk_buffer->memories[i], NULL);
		}
	}

	// Destroy image
	if (vk_buffer->image != VK_NULL_HANDLE) {
		vkDestroyImage(device, vk_buffer->image, NULL);
	}

	// Remove from list
	wlf_linked_list_remove(&vk_buffer->link);

	free(vk_buffer);
}

struct wlf_vk_buffer *wlf_vk_buffer_get(struct wlf_vk_renderer *renderer,
		struct wlf_buffer *buffer) {
	struct wlf_vk_buffer *vk_buffer;
	wlf_linked_list_for_each(vk_buffer, &renderer->buffers, link) {
		if (vk_buffer->wlf_buffer == buffer) {
			return vk_buffer;
		}
	}
	return NULL;
}

struct wlf_vk_buffer *wlf_vk_buffer_from_buffer(struct wlf_vk_renderer *renderer,
		struct wlf_buffer *buffer) {
	if (renderer == NULL || buffer == NULL) {
		return NULL;
	}
	return wlf_vk_buffer_get(renderer, buffer);
}

bool wlf_buffer_is_vk(struct wlf_vk_renderer *renderer,
		struct wlf_buffer *buffer) {
	if (renderer == NULL || buffer == NULL) {
		return false;
	}
	return wlf_vk_buffer_get(renderer, buffer) != NULL;
}

void wlf_vk_buffer_transition_layout(struct wlf_vk_buffer *vk_buffer,
		VkCommandBuffer command_buffer, VkImageLayout old_layout,
		VkImageLayout new_layout, VkPipelineStageFlags src_stage_mask,
		VkPipelineStageFlags dst_stage_mask) {
	assert(vk_buffer != NULL && command_buffer != VK_NULL_HANDLE);

	VkImageMemoryBarrier barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = vk_buffer->image,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};

	// Set access masks based on layouts
	switch (old_layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			barrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			break;
		default:
			break;
	}

	switch (new_layout) {
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
	}

	vkCmdPipelineBarrier(command_buffer,
		src_stage_mask, dst_stage_mask,
		0,
		0, NULL,
		0, NULL,
		1, &barrier);

	vk_buffer->layout = new_layout;
}
