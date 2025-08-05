#include "wlf/render/vulkan/vk_render.h"
#include "wlf/render/vulkan/vk_device.h"
#include "wlf/render/vulkan/vk_instance.h"
#include "wlf/utils/wlf_env.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

struct wlf_render *wlf_vk_render_create_from_backend(
		struct wlf_backend *backend) {
	wlf_log(WLF_INFO, "Run with VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation "
		"to enable the validation layer");
	struct wlf_vk_instance *ini = wlf_vk_instance_create(
		wlf_env_parse_bool("WLR_RENDER_DEBUG)"));
	if (!ini) {
		wlf_log(WLF_ERROR, "creating vulkan instance for render failed");
		return NULL;
	}

	VkPhysicalDevice phdev = wlf_vk_find_phdev(ini);
	if (phdev == VK_NULL_HANDLE) {
		wlf_log(WLF_ERROR, "finding physical device for render failed");
		goto cleanup;
	}

	struct wlf_vk_device *device = wlf_vk_device_create(ini, phdev);
	if (!device) {
		wlf_log(WLF_ERROR, "Failed to create vulkan device");
		goto cleanup;
	}

	return wlr_vk_render_create_for_device(device);

cleanup:
	wlf_vk_instance_destroy(ini);
	return NULL;
}

void wlf_vk_render_destroy(struct wlf_vk_render *vk_render) {

}

bool check_extension(const VkExtensionProperties *avail,
		uint32_t avail_len, const char *name) {
	for (size_t i = 0; i < avail_len; i++) {
		if (strcmp(avail[i].extensionName, name) == 0) {
			return true;
		}
	}

	return false;
}

struct wlf_render *wlr_vk_render_create_for_device(struct wlf_vk_device *dev) {
	VkResult res;
	struct wlf_vk_render *render = calloc(1, sizeof(*render));
	if (!render) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlr_vk_render");
		wlf_vk_device_destroy(dev);
		return NULL;
	}

	render->dev = dev;
	VkPhysicalDeviceProperties phdev_props;
	vkGetPhysicalDeviceProperties(dev->phdev, &phdev_props);
	if (phdev_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
		render->base.type = CPU;
	} else {
		render->base.type = GPU;
	}

	// if (!init_static_render_data(render)) {
	// 	goto error;
	// }

	VkCommandPoolCreateInfo cpool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = dev->queue_family,
	};
	res = vkCreateCommandPool(dev->dev, &cpool_info, NULL,
		&render->command_pool);
	if (res != VK_SUCCESS) {
		wlf_vk_error("vkCreateCommandPool", res);
		goto error;
	}

	VkSemaphoreTypeCreateInfoKHR semaphore_type_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR,
		.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE_KHR,
		.initialValue = 0,
	};
	VkSemaphoreCreateInfo semaphore_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = &semaphore_type_info,
	};
	res = vkCreateSemaphore(dev->dev, &semaphore_info, NULL,
		&render->timeline_semaphore);
	if (res != VK_SUCCESS) {
		wlf_vk_error("vkCreateSemaphore failed", res);
		goto error;
	}

	return &render->base;

error:
	wlf_vk_render_destroy(render);
	return NULL;
}

const char *wlf_vulkan_strerror(VkResult err) {
#define ERR_STR(r) case VK_ ##r: return #r
	switch (err) {
	ERR_STR(SUCCESS);
	ERR_STR(NOT_READY);
	ERR_STR(TIMEOUT);
	ERR_STR(EVENT_SET);
	ERR_STR(EVENT_RESET);
	ERR_STR(INCOMPLETE);
	ERR_STR(ERROR_OUT_OF_HOST_MEMORY);
	ERR_STR(ERROR_OUT_OF_DEVICE_MEMORY);
	ERR_STR(ERROR_INITIALIZATION_FAILED);
	ERR_STR(ERROR_DEVICE_LOST);
	ERR_STR(ERROR_MEMORY_MAP_FAILED);
	ERR_STR(ERROR_LAYER_NOT_PRESENT);
	ERR_STR(ERROR_EXTENSION_NOT_PRESENT);
	ERR_STR(ERROR_FEATURE_NOT_PRESENT);
	ERR_STR(ERROR_INCOMPATIBLE_DRIVER);
	ERR_STR(ERROR_TOO_MANY_OBJECTS);
	ERR_STR(ERROR_FORMAT_NOT_SUPPORTED);
	ERR_STR(ERROR_FRAGMENTED_POOL);
	ERR_STR(ERROR_UNKNOWN);
	ERR_STR(ERROR_OUT_OF_POOL_MEMORY);
	ERR_STR(ERROR_INVALID_EXTERNAL_HANDLE);
	ERR_STR(ERROR_FRAGMENTATION);
	ERR_STR(ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
	ERR_STR(PIPELINE_COMPILE_REQUIRED);
	ERR_STR(ERROR_SURFACE_LOST_KHR);
	ERR_STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
	ERR_STR(SUBOPTIMAL_KHR);
	ERR_STR(ERROR_OUT_OF_DATE_KHR);
	ERR_STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
	ERR_STR(ERROR_VALIDATION_FAILED_EXT);
	ERR_STR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
	default:
		return "<unknown>";
	}
#undef ERR_STR
}
