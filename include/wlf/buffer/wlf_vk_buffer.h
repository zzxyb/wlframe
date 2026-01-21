/**
 * @file        wlf_vk_buffer.h
 * @brief       Vulkan render buffer management.
 * @details     This file provides structures and functions for managing Vulkan render buffers.
 *              These buffers are used by the Vulkan renderer for GPU-based rendering operations.
 *              They wrap Vulkan images and device memory, and can be imported from DMA-BUF
 *              file descriptors for zero-copy integration with other system components.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-22, initial version\n
 */

#ifndef BUFFER_WLF_VK_BUFFER_H
#define BUFFER_WLF_VK_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/utils/wlf_linked_list.h"

#include <vulkan/vulkan.h>
#include <gbm.h>
#include <stdbool.h>

struct wlf_vk_renderer;

/**
 * @brief A Vulkan render buffer.
 *
 * This structure wraps a Vulkan image and its associated device memory,
 * providing integration between wlframe's buffer abstraction and Vulkan
 * GPU resources. Buffers can be imported from external memory (DMA-BUF)
 * or allocated directly through Vulkan.
 */
struct wlf_vk_buffer {
	struct wlf_buffer *wlf_buffer;       /**< Associated wlframe buffer */
	struct wlf_vk_renderer *renderer;    /**< Vulkan renderer that owns this buffer */
	struct wlf_linked_list link;         /**< Link in renderer's buffer list */

	VkImage image;                       /**< Vulkan image handle */
	VkDeviceMemory memories[GBM_MAX_PLANES]; /**< Device memory for each plane */
	uint32_t mem_count;                  /**< Number of memory objects */

	VkImageLayout layout;                /**< Current image layout */
	VkFormat format;                     /**< Vulkan format of the image */

	bool externally_imported;            /**< True if imported from external memory (DMA-BUF) */
};

/**
 * @brief Creates a Vulkan render buffer from a wlframe buffer.
 *
 * This function creates a Vulkan image and imports or allocates device memory
 * based on the buffer type. For DMA-BUF buffers, it uses external memory
 * import. For SHM buffers, it may create staging resources.
 *
 * @param renderer Vulkan renderer instance.
 * @param buffer wlframe buffer to wrap.
 * @return Pointer to the created Vulkan buffer, or NULL on failure.
 */
struct wlf_vk_buffer *wlf_vk_buffer_create(struct wlf_vk_renderer *renderer,
	struct wlf_buffer *buffer);

/**
 * @brief Destroys a Vulkan render buffer.
 *
 * Releases Vulkan resources including the image and device memory.
 *
 * @param vk_buffer Vulkan buffer to destroy.
 */
void wlf_vk_buffer_destroy(struct wlf_vk_buffer *vk_buffer);

/**
 * @brief Gets a Vulkan buffer associated with a wlframe buffer.
 *
 * Searches for an existing Vulkan buffer wrapping the given wlframe buffer
 * in the renderer's buffer list.
 *
 * @param renderer Vulkan renderer instance.
 * @param buffer wlframe buffer to look up.
 * @return Pointer to the Vulkan buffer, or NULL if not found.
 */
struct wlf_vk_buffer *wlf_vk_buffer_get(struct wlf_vk_renderer *renderer,
	struct wlf_buffer *buffer);

/**
 * @brief Gets the Vulkan buffer from a wlframe buffer.
 *
 * This is a convenience function that searches all renderers for a Vulkan buffer
 * wrapping the given wlframe buffer. Note: This requires the renderer to be
 * provided separately. For direct lookup, use wlf_vk_buffer_get() instead.
 *
 * @param renderer Vulkan renderer instance.
 * @param buffer wlframe buffer to convert.
 * @return Pointer to the Vulkan buffer, or NULL if not found or not a Vulkan buffer.
 */
struct wlf_vk_buffer *wlf_vk_buffer_from_buffer(struct wlf_vk_renderer *renderer,
	struct wlf_buffer *buffer);

/**
 * @brief Checks if a wlframe buffer has an associated Vulkan buffer.
 *
 * Determines whether the given wlframe buffer has been imported into Vulkan
 * rendering context.
 *
 * @param renderer Vulkan renderer instance.
 * @param buffer wlframe buffer to check.
 * @return true if a Vulkan buffer exists for this buffer, false otherwise.
 */
bool wlf_buffer_is_vk(struct wlf_vk_renderer *renderer,
	struct wlf_buffer *buffer);

/**
 * @brief Transitions a Vulkan buffer's image layout.
 *
 * Records a pipeline barrier command to transition the image from one
 * layout to another. This is necessary before using the image for
 * different operations (rendering, presentation, etc.).
 *
 * @param vk_buffer Vulkan buffer containing the image.
 * @param command_buffer Command buffer to record the transition.
 * @param old_layout Current layout of the image.
 * @param new_layout Target layout for the image.
 * @param src_stage_mask Source pipeline stage.
 * @param dst_stage_mask Destination pipeline stage.
 */
void wlf_vk_buffer_transition_layout(struct wlf_vk_buffer *vk_buffer,
	VkCommandBuffer command_buffer, VkImageLayout old_layout,
	VkImageLayout new_layout, VkPipelineStageFlags src_stage_mask,
	VkPipelineStageFlags dst_stage_mask);

#endif // BUFFER_WLF_VK_BUFFER_H
