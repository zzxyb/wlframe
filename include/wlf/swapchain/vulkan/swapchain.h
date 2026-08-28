/**
 * @file        swapchain.h
 * @brief       Vulkan swapchain implementation.
 * @details     Declares the Vulkan surface and image queue used to present
 *              rendered frames for a wlframe window.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H

#include "wlf/config.h"
#include "wlf/swapchain/wlf_swapchain.h"

#if WLF_HAS_LINUX_PLATFORM
#ifndef VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#endif

#include <vulkan/vulkan.h>
#if WLF_HAS_LINUX_PLATFORM
#include <vulkan/vulkan_wayland.h>
#endif

#include <stdbool.h>

/**
 * @brief Swapchain backed by a Vulkan presentation surface.
 *
 * The image array belongs to the Vulkan swapchain and is replaced whenever
 * the swapchain is recreated.
 */
struct wlf_vk_swapchain {
	struct wlf_swapchain base; /**< Generic swapchain interface. */

	VkSurfaceKHR surface; /**< Vulkan presentation surface. */
	VkSwapchainKHR swapchain; /**< Vulkan swapchain handle. */
	VkImage *images; /**< Images owned by @p swapchain. */
	uint32_t image_count; /**< Number of entries in @p images. */
	uint32_t image_index; /**< Image acquired for the current frame. */

	VkFormat image_format; /**< Format of swapchain images. */
	VkColorSpaceKHR color_space; /**< Presentation color space. */
	VkPresentModeKHR present_mode; /**< Presentation scheduling mode. */
	VkExtent2D extent; /**< Image extent in pixels. */
	VkSemaphore image_available; /**< Semaphore for the acquired image. */
};

/**
 * @brief Creates a Vulkan swapchain for a window and render format.
 * @param window Window receiving the swapchain.
 * @param width Initial buffer width in pixels.
 * @param height Initial buffer height in pixels.
 * @param format Requested render format.
 * @return Newly allocated generic swapchain, or NULL on failure.
 */
struct wlf_vk_swapchain *wlf_vk_swapchain_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);

/**
 * @brief Checks whether a swapchain is Vulkan-backed.
 * @param swapchain Generic swapchain to inspect.
 * @return true when @p swapchain is Vulkan-backed, false otherwise.
 */
bool wlf_swapchain_is_vk(const struct wlf_swapchain *swapchain);

/**
 * @brief Casts a generic swapchain to a Vulkan swapchain.
 * @param swapchain Swapchain known to be Vulkan-backed.
 * @return Enclosing Vulkan swapchain, or NULL when the type does not match.
 */
struct wlf_vk_swapchain *wlf_vk_swapchain_from_swapchain(struct wlf_swapchain *swapchain);

#endif // VULKAN_SWAPCHAIN_H
