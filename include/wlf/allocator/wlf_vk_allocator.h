/**
 * @file        wlf_vk_allocator.h
 * @brief       Vulkan-based buffer allocator.
 * @details     This file provides a buffer allocator implementation using Vulkan.
 *              The allocator creates GPU-native buffers suitable for hardware-accelerated
 *              rendering through Vulkan. Buffers are allocated with device-local memory
 *              and can be imported/exported via external memory handles (DMA-BUF).
 * @author      YaoBing Xiao
 * @date        2026-01-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-22, initial version\n
 */

#ifndef ALLOCATOR_WLF_VK_ALLOCATOR_H
#define ALLOCATOR_WLF_VK_ALLOCATOR_H

#include "wlf/allocator/wlf_allocator.h"
#include "wlf/utils/wlf_linked_list.h"

#include <vulkan/vulkan.h>
#include <stdbool.h>

struct wlf_vk_device;

/**
 * @brief A Vulkan-based buffer allocator.
 *
 * This allocator creates buffers backed by Vulkan device memory,
 * providing GPU-native buffer allocation suitable for hardware-accelerated
 * rendering. Buffers can be exported as DMA-BUF file descriptors for
 * zero-copy sharing with other system components.
 */
struct wlf_vk_allocator {
	struct wlf_allocator base;          /**< Base allocator structure */

	struct wlf_vk_device *device;       /**< Vulkan device handle */
	struct wlf_linked_list buffers;     /**< List of allocated buffers */
};

/**
 * @brief Creates a new Vulkan allocator.
 *
 * Creates an allocator that uses Vulkan to allocate GPU-native buffers.
 * The Vulkan device must support external memory export.
 *
 * @param device Vulkan device wrapper. Must remain valid for the allocator's lifetime.
 * @return Pointer to the created allocator, or NULL on failure.
 */
struct wlf_allocator *wlf_vk_allocator_create(struct wlf_vk_device *device);

/**
 * @brief Gets the Vulkan allocator from a generic allocator.
 *
 * @param allocator Generic allocator pointer.
 * @return Vulkan allocator pointer, or NULL if the allocator is not a Vulkan allocator.
 */
struct wlf_vk_allocator *wlf_vk_allocator_from_allocator(
	struct wlf_allocator *allocator);

/**
 * @brief Checks if an allocator is a Vulkan allocator.
 *
 * @param allocator Allocator to check.
 * @return true if the allocator is a Vulkan allocator, false otherwise.
 */
bool wlf_allocator_is_vk(struct wlf_allocator *allocator);

#endif // ALLOCATOR_WLF_VK_ALLOCATOR_H
