#include "wlf/swapchain/vulkan/swapchain.h"

#include "wlf/config.h"
#include "wlf/renderer/vulkan/device.h"
#include "wlf/renderer/vulkan/instance.h"
#include "wlf/renderer/vulkan/renderer.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wlf_window.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/platform/wayland/backend.h"
#endif

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if WLF_HAS_LINUX_PLATFORM
#include <wayland-client-core.h>
#endif

static VkFormat format_to_vk(uint32_t format) {
	switch (format) {
	case WLF_FORMAT_ARGB8888:
	case WLF_FORMAT_XRGB8888:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case WLF_FORMAT_ABGR8888:
	case WLF_FORMAT_XBGR8888:
		return VK_FORMAT_R8G8B8A8_UNORM;
	default:
		return VK_FORMAT_B8G8R8A8_UNORM;
	}
}

static VkSurfaceFormatKHR choose_surface_format(
		const VkSurfaceFormatKHR *formats, uint32_t format_count,
		const struct wlf_render_format *format) {
	VkFormat preferred = format_to_vk(format->format);
	if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		return (VkSurfaceFormatKHR){
			.format = preferred,
			.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		};
	}

	for (uint32_t i = 0; i < format_count; i++) {
		if (formats[i].format == preferred &&
				formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return formats[i];
		}
	}

	return formats[0];
}

static VkPresentModeKHR choose_present_mode(
		const VkPresentModeKHR *modes, uint32_t mode_count) {
	for (uint32_t i = 0; i < mode_count; i++) {
		if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return modes[i];
		}
	}
	for (uint32_t i = 0; i < mode_count; i++) {
		if (modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			return modes[i];
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkCompositeAlphaFlagBitsKHR choose_composite_alpha(
		VkCompositeAlphaFlagsKHR supported) {
	const VkCompositeAlphaFlagBitsKHR choices[] = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

	for (size_t i = 0; i < sizeof(choices) / sizeof(choices[0]); i++) {
		if (supported & choices[i]) {
			return choices[i];
		}
	}

	return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

static uint32_t clamp_image_count(const VkSurfaceCapabilitiesKHR *caps) {
	uint32_t image_count = caps->minImageCount + 1;
	if (caps->maxImageCount > 0 && image_count > caps->maxImageCount) {
		image_count = caps->maxImageCount;
	}

	return image_count;
}

static void destroy_vk_swapchain_objects(struct wlf_vk_swapchain *swapchain) {
	struct wlf_vk_renderer *renderer =
		wlf_vk_renderer_from_renderer(swapchain->base.window->state.renderer);
	struct wlf_vk_device *dev = renderer->dev;

	if (swapchain->swapchain != VK_NULL_HANDLE) {
		dev->api.vkDestroySwapchainKHR(dev->base, swapchain->swapchain, NULL);
		swapchain->swapchain = VK_NULL_HANDLE;
	}

	free(swapchain->images);
	swapchain->images = NULL;
	swapchain->image_count = 0;
}

static bool create_vk_swapchain_objects(struct wlf_vk_swapchain *swapchain) {
	struct wlf_vk_renderer *renderer =
		wlf_vk_renderer_from_renderer(swapchain->base.window->state.renderer);
	struct wlf_vk_device *dev = renderer->dev;
	VkResult res;

	VkBool32 supported = VK_FALSE;
	res = vkGetPhysicalDeviceSurfaceSupportKHR(dev->phdev, dev->queue_family,
		swapchain->surface, &supported);
	if (res != VK_SUCCESS || !supported) {
		wlf_vk_error("vkGetPhysicalDeviceSurfaceSupportKHR", res);
		return false;
	}

	VkSurfaceCapabilitiesKHR caps;
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev->phdev,
		swapchain->surface, &caps);
	if (res != VK_SUCCESS) {
		wlf_vk_error("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", res);
		return false;
	}

	uint32_t format_count = 0;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(dev->phdev, swapchain->surface,
		&format_count, NULL);
	if (res != VK_SUCCESS || format_count == 0) {
		wlf_vk_error("vkGetPhysicalDeviceSurfaceFormatsKHR", res);
		return false;
	}

	VkSurfaceFormatKHR surface_formats[format_count];
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(dev->phdev, swapchain->surface,
		&format_count, surface_formats);
	if (res != VK_SUCCESS) {
		wlf_vk_error("vkGetPhysicalDeviceSurfaceFormatsKHR", res);
		return false;
	}

	uint32_t present_mode_count = 0;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(dev->phdev,
		swapchain->surface, &present_mode_count, NULL);
	if (res != VK_SUCCESS || present_mode_count == 0) {
		wlf_vk_error("vkGetPhysicalDeviceSurfacePresentModesKHR", res);
		return false;
	}

	VkPresentModeKHR present_modes[present_mode_count];
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(dev->phdev,
		swapchain->surface, &present_mode_count, present_modes);
	if (res != VK_SUCCESS) {
		wlf_vk_error("vkGetPhysicalDeviceSurfacePresentModesKHR", res);
		return false;
	}

	VkSurfaceFormatKHR surface_format =
		choose_surface_format(surface_formats, format_count,
			&swapchain->base.format);
	swapchain->image_format = surface_format.format;
	swapchain->color_space = surface_format.colorSpace;
	swapchain->present_mode =
		choose_present_mode(present_modes, present_mode_count);
	swapchain->extent = (VkExtent2D){
		.width = (uint32_t)swapchain->base.width,
		.height = (uint32_t)swapchain->base.height,
	};
	if (caps.currentExtent.width != UINT32_MAX) {
		swapchain->extent = caps.currentExtent;
	}

	VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VkSwapchainCreateInfoKHR swapchain_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = swapchain->surface,
		.minImageCount = clamp_image_count(&caps),
		.imageFormat = swapchain->image_format,
		.imageColorSpace = swapchain->color_space,
		.imageExtent = swapchain->extent,
		.imageArrayLayers = 1,
		.imageUsage = usage,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = caps.currentTransform,
		.compositeAlpha = choose_composite_alpha(caps.supportedCompositeAlpha),
		.presentMode = swapchain->present_mode,
		.clipped = VK_TRUE,
	};

	res = dev->api.vkCreateSwapchainKHR(dev->base, &swapchain_info, NULL,
		&swapchain->swapchain);
	if (res != VK_SUCCESS) {
		wlf_vk_error("vkCreateSwapchainKHR", res);
		return false;
	}

	res = dev->api.vkGetSwapchainImagesKHR(dev->base, swapchain->swapchain,
		&swapchain->image_count, NULL);
	if (res != VK_SUCCESS || swapchain->image_count == 0) {
		wlf_vk_error("vkGetSwapchainImagesKHR", res);
		destroy_vk_swapchain_objects(swapchain);
		return false;
	}

	swapchain->images = calloc(swapchain->image_count,
		sizeof(swapchain->images[0]));
	if (swapchain->images == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate Vulkan swapchain images");
		destroy_vk_swapchain_objects(swapchain);
		return false;
	}

	res = dev->api.vkGetSwapchainImagesKHR(dev->base, swapchain->swapchain,
		&swapchain->image_count, swapchain->images);
	if (res != VK_SUCCESS) {
		wlf_vk_error("vkGetSwapchainImagesKHR", res);
		destroy_vk_swapchain_objects(swapchain);
		return false;
	}

	return true;
}

static void swapchain_destroy(struct wlf_swapchain *base) {
	struct wlf_vk_swapchain *swapchain =
		wlf_vk_swapchain_from_swapchain(base);
	struct wlf_vk_renderer *renderer =
		wlf_vk_renderer_from_renderer(base->window->state.renderer);
	struct wlf_vk_device *dev = renderer->dev;
	struct wlf_vk_instance *instance = dev->instance;

	if (dev->base != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(dev->base);
	}

	destroy_vk_swapchain_objects(swapchain);

	if (swapchain->image_available != VK_NULL_HANDLE) {
		vkDestroySemaphore(dev->base, swapchain->image_available, NULL);
	}
	if (swapchain->surface != VK_NULL_HANDLE) {
		instance->api.destroySurfaceKHR(instance->base, swapchain->surface,
			NULL);
	}

	free(swapchain);
}

static bool swapchain_resize(struct wlf_swapchain *base, int width,
		int height) {
	struct wlf_vk_swapchain *swapchain =
		wlf_vk_swapchain_from_swapchain(base);
	struct wlf_vk_renderer *renderer =
		wlf_vk_renderer_from_renderer(base->window->state.renderer);
	struct wlf_vk_device *dev = renderer->dev;

	VkResult res = vkDeviceWaitIdle(dev->base);
	if (res != VK_SUCCESS) {
		wlf_vk_error("vkDeviceWaitIdle", res);
		return false;
	}

	base->width = width;
	base->height = height;
	destroy_vk_swapchain_objects(swapchain);

	return create_vk_swapchain_objects(swapchain);
}

static void swapchain_present(struct wlf_swapchain *base,
		const pixman_region32_t *damage) {
	(void)damage;
	struct wlf_vk_swapchain *swapchain =
		wlf_vk_swapchain_from_swapchain(base);
	struct wlf_vk_renderer *renderer =
		wlf_vk_renderer_from_renderer(base->window->state.renderer);
	struct wlf_vk_device *dev = renderer->dev;

	VkResult res = dev->api.vkAcquireNextImageKHR(dev->base,
		swapchain->swapchain, UINT64_MAX, swapchain->image_available,
		VK_NULL_HANDLE, &swapchain->image_index);
	if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		wlf_vk_error("vkAcquireNextImageKHR", res);
		return;
	}

	int nrects = 0;
	pixman_box32_t *damage_rects = damage != NULL ?
		pixman_region32_rectangles((pixman_region32_t *)damage, &nrects) : NULL;
	uint32_t rect_count = nrects > 0 ? (uint32_t)nrects : 0;

	VkPresentRegionKHR present_region = {0};
	VkPresentRegionsKHR present_regions = {0};
	VkRectLayerKHR rects[rect_count > 0 ? rect_count : 1];
	if (rect_count > 0 && dev->incremental_present) {
		for (uint32_t i = 0; i < rect_count; i++) {
			const pixman_box32_t *rect = &damage_rects[i];
			rects[i] = (VkRectLayerKHR){
				.offset = { rect->x1, rect->y1 },
				.extent = {
					(uint32_t)(rect->x2 - rect->x1),
					(uint32_t)(rect->y2 - rect->y1),
				},
				.layer = 0,
			};
		}

		present_region = (VkPresentRegionKHR){
			.rectangleCount = rect_count,
			.pRectangles = rects,
		};
		present_regions = (VkPresentRegionsKHR){
			.sType = VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR,
			.swapchainCount = 1,
			.pRegions = &present_region,
		};
	}

	VkSemaphore wait_semaphores[] = {
		swapchain->image_available,
	};
	VkSwapchainKHR swapchains[] = {
		swapchain->swapchain,
	};
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = present_regions.pRegions != NULL ? &present_regions : NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait_semaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &swapchain->image_index,
	};

	res = dev->api.vkQueuePresentKHR(dev->queue, &present_info);
	if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		wlf_vk_error("vkQueuePresentKHR", res);
	}
}

static const struct wlf_swapchain_impl swapchain_impl = {
	.destroy = swapchain_destroy,
	.resize = swapchain_resize,
	.present = swapchain_present,
};

struct wlf_swapchain *wlf_vk_swapchain_create(struct wlf_window *window,
		int width, int height, const struct wlf_render_format *format) {
	struct wlf_vk_renderer *renderer =
		wlf_vk_renderer_from_renderer(window->state.renderer);
	if (renderer == NULL) {
		return NULL;
	}

	struct wlf_vk_swapchain *swapchain = calloc(1, sizeof(*swapchain));
	if (swapchain == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_vk_swapchain");
		return NULL;
	}

	wlf_swapchain_init(&swapchain->base, NULL, &swapchain_impl, width, height);
	swapchain->base.window = window;
	if (!wlf_render_format_copy(&swapchain->base.format, format)) {
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}

#if WLF_HAS_LINUX_PLATFORM
	if (wlf_backend_is_wayland(window->state.backend)) {
		struct wl_surface *surface = wlf_window_native_handle(window);
		if (surface == NULL) {
			wlf_log(WLF_ERROR, "Vulkan Wayland swapchain requires wl_surface");
			wlf_swapchain_destroy(&swapchain->base);
			return NULL;
		}

		struct wlf_vk_instance *instance = renderer->dev->instance;
		VkWaylandSurfaceCreateInfoKHR surface_info = {
			.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
			.display = window->state.backend->impl->native_display(window->state.backend),
			.surface = surface,
		};
		VkResult res = instance->api.createWaylandSurfaceKHR(instance->base,
			&surface_info, NULL, &swapchain->surface);
		if (res != VK_SUCCESS) {
			wlf_vk_error("vkCreateWaylandSurfaceKHR", res);
			wlf_swapchain_destroy(&swapchain->base);
			return NULL;
		}
	} else
#endif
	{
		wlf_log(WLF_ERROR, "Vulkan swapchain unsupported for this platform");
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}

	VkSemaphoreCreateInfo semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	VkResult res = vkCreateSemaphore(renderer->dev->base, &semaphore_info,
		NULL, &swapchain->image_available);
	if (res != VK_SUCCESS) {
		wlf_vk_error("vkCreateSemaphore", res);
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}

	if (!create_vk_swapchain_objects(swapchain)) {
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}

	return &swapchain->base;
}

bool wlf_swapchain_is_vk(const struct wlf_swapchain *swapchain) {
	return swapchain->impl == &swapchain_impl;
}

struct wlf_vk_swapchain *wlf_vk_swapchain_from_swapchain(
		struct wlf_swapchain *swapchain) {
	assert(swapchain->impl == &swapchain_impl);

	struct wlf_vk_swapchain *vk_swapchain =
		wlf_container_of(swapchain, vk_swapchain, base);

	return vk_swapchain;
}
