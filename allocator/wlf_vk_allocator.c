/**
 * @file        wlf_vk_allocator.c
 * @brief       Vulkan-based buffer allocator implementation.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 */

#include "wlf/allocator/wlf_vk_allocator.h"
#include "wlf/renderer/vulkan/device.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>
#include <unistd.h>

// Forward declaration - will need a Vulkan-native buffer type
// For now, we'll create a simple wrapper
struct wlf_vk_native_buffer {
	struct wlf_buffer base;
	struct wlf_linked_list link;

	VkImage image;
	VkDeviceMemory memory;
	VkFormat format;

	// For DMA-BUF export
	int dmabuf_fd;
	bool exported;
};

struct wlf_vk_native_buffer *wlf_vk_native_buffer_create(
	struct wlf_vk_allocator *alloc, int width, int height,
	uint32_t drm_format);

static const struct wlf_allocator_impl allocator_impl;

static struct wlf_vk_allocator *get_vk_alloc_from_alloc(
		struct wlf_allocator *wlf_alloc) {
	assert(wlf_alloc->impl == &allocator_impl);
	return wlf_container_of(wlf_alloc, struct wlf_vk_allocator, base);
}

struct wlf_allocator *wlf_vk_allocator_create(struct wlf_vk_device *device) {
	if (device == NULL) {
		wlf_log(WLF_ERROR, "Invalid Vulkan device");
		return NULL;
	}

	struct wlf_vk_allocator *alloc = calloc(1, sizeof(*alloc));
	if (alloc == NULL) {
		return NULL;
	}

	alloc->device = device;
	wlf_linked_list_init(&alloc->buffers);

	// Initialize base allocator
	alloc->base.impl = &allocator_impl;
	wlf_signal_init(&alloc->base.events.destroy);

	wlf_log(WLF_DEBUG, "Created Vulkan allocator");
	return &alloc->base;
}

/**
 * @brief Destroys a Vulkan allocator.
 */
static void allocator_destroy(struct wlf_allocator *wlf_alloc) {
	struct wlf_vk_allocator *alloc = get_vk_alloc_from_alloc(wlf_alloc);

	// Destroy all allocated buffers
	struct wlf_vk_native_buffer *buf, *buf_tmp;
	wlf_linked_list_for_each_safe(buf, buf_tmp, &alloc->buffers, link) {
		VkDevice device = alloc->device->base;

		if (buf->exported && buf->dmabuf_fd >= 0) {
			close(buf->dmabuf_fd);
		}

		if (buf->image != VK_NULL_HANDLE) {
			vkDestroyImage(device, buf->image, NULL);
		}

		if (buf->memory != VK_NULL_HANDLE) {
			vkFreeMemory(device, buf->memory, NULL);
		}

		wlf_linked_list_remove(&buf->link);
		wlf_linked_list_init(&buf->link);
	}

	free(alloc);
}

/**
 * @brief Convert DRM format to Vulkan format.
 */
static VkFormat drm_format_to_vk(uint32_t drm_format) {
	switch (drm_format) {
		case DRM_FORMAT_XRGB8888:
		case DRM_FORMAT_ARGB8888:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case DRM_FORMAT_XBGR8888:
		case DRM_FORMAT_ABGR8888:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case DRM_FORMAT_RGB565:
			return VK_FORMAT_R5G6B5_UNORM_PACK16;
		default:
			wlf_log(WLF_ERROR, "Unsupported DRM format: 0x%08X", drm_format);
			return VK_FORMAT_UNDEFINED;
	}
}

/**
 * @brief Creates a Vulkan-native buffer.
 */
struct wlf_vk_native_buffer *wlf_vk_native_buffer_create(
		struct wlf_vk_allocator *alloc, int width, int height,
		uint32_t drm_format) {

	struct wlf_vk_native_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}

	VkDevice device = alloc->device->base;
	buffer->format = drm_format_to_vk(drm_format);
	buffer->dmabuf_fd = -1;
	buffer->exported = false;

	if (buffer->format == VK_FORMAT_UNDEFINED) {
		free(buffer);
		return NULL;
	}

	// Create exportable Vulkan image
	VkExternalMemoryImageCreateInfo external_info = {
		.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
		.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
	};

	VkImageCreateInfo image_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = &external_info,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = buffer->format,
		.extent = {
			.width = width,
			.height = height,
			.depth = 1,
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		         VK_IMAGE_USAGE_SAMPLED_BIT |
		         VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		         VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VkResult res = vkCreateImage(device, &image_info, NULL, &buffer->image);
	if (res != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "vkCreateImage failed: %d", res);
		free(buffer);
		return NULL;
	}

	// Allocate exportable device memory
	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(device, buffer->image, &mem_reqs);

	// Find suitable memory type (device-local preferred)
	VkPhysicalDeviceMemoryProperties mem_props;
	vkGetPhysicalDeviceMemoryProperties(alloc->device->phdev, &mem_props);

	uint32_t mem_type_index = UINT32_MAX;
	for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
		if ((mem_reqs.memoryTypeBits & (1 << i)) &&
		    (mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
			mem_type_index = i;
			break;
		}
	}

	if (mem_type_index == UINT32_MAX) {
		wlf_log(WLF_ERROR, "Failed to find suitable memory type");
		vkDestroyImage(device, buffer->image, NULL);
		free(buffer);
		return NULL;
	}

	VkExportMemoryAllocateInfo export_info = {
		.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
		.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT,
	};

	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &export_info,
		.allocationSize = mem_reqs.size,
		.memoryTypeIndex = mem_type_index,
	};

	res = vkAllocateMemory(device, &alloc_info, NULL, &buffer->memory);
	if (res != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "vkAllocateMemory failed: %d", res);
		vkDestroyImage(device, buffer->image, NULL);
		free(buffer);
		return NULL;
	}

	// Bind memory to image
	res = vkBindImageMemory(device, buffer->image, buffer->memory, 0);
	if (res != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "vkBindImageMemory failed: %d", res);
		vkFreeMemory(device, buffer->memory, NULL);
		vkDestroyImage(device, buffer->image, NULL);
		free(buffer);
		return NULL;
	}

	// Initialize base buffer structure
	// wlf_buffer_init(&buffer->base, &buffer_impl, width, height);

	// Add to allocator's buffer list
	wlf_linked_list_insert(&alloc->buffers, &buffer->link);

	wlf_log(WLF_DEBUG, "Allocated %dx%d Vulkan buffer with format 0x%08X",
		width, height, drm_format);

	return buffer;
}

/**
 * @brief Creates a buffer using the Vulkan allocator.
 *
 * Note: This simplified version uses DRM_FORMAT_XRGB8888 as default.
 * A more complete implementation should accept format parameters.
 */
static struct wlf_buffer *allocator_create_buffer(
		struct wlf_allocator *wlf_alloc, uint32_t width, uint32_t height) {
	struct wlf_vk_allocator *alloc = get_vk_alloc_from_alloc(wlf_alloc);

	// Use default format for now
	// TODO: Should be configurable based on renderer/display requirements
	uint32_t format = DRM_FORMAT_XRGB8888;

	struct wlf_vk_native_buffer *buffer = wlf_vk_native_buffer_create(alloc,
		width, height, format);
	if (buffer == NULL) {
		return NULL;
	}
	return &buffer->base;
}

static const struct wlf_allocator_impl allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};

// Public API implementations

struct wlf_vk_allocator *wlf_vk_allocator_from_allocator(
		struct wlf_allocator *allocator) {
	if (allocator == NULL || allocator->impl != &allocator_impl) {
		return NULL;
	}
	return get_vk_alloc_from_alloc(allocator);
}

bool wlf_allocator_is_vk(struct wlf_allocator *allocator) {
	if (allocator == NULL) {
		return false;
	}
	return allocator->impl == &allocator_impl;
}
