#include "wlf/renderer/vulkan/vk_renderer.h"
#include "wlf/renderer/vulkan/vk_device.h"
#include "wlf/renderer/vulkan/vk_instance.h"
#include "wlf/utils/wlf_env.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

struct wlf_renderer *wlf_vk_renderer_create_from_backend(
		struct wlf_backend *backend) {
	wlf_log(WLF_INFO, "Run with VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation "
		"to enable the validation layer");
	struct wlf_vk_instance *ini = wlf_vk_instance_create(
		wlf_env_parse_bool("WSM_RENDER_DEBUG)"));
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

void wlf_vk_renderer_destroy(struct wlf_vk_renderer *vk_render) {

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

struct wlf_renderer *wlr_vk_render_create_for_device(struct wlf_vk_device *device) {
	VkResult res;
	struct wlf_vk_renderer *render = calloc(1, sizeof(*render));
	if (!render) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlr_vk_render");
		wlf_vk_device_destroy(device);
		return NULL;
	}

	render->dev = device;
	VkPhysicalDeviceProperties phdev_props;
	vkGetPhysicalDeviceProperties(device->phdev, &phdev_props);
	if (phdev_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
		render->base.type = CPU;
	} else {
		render->base.type = GPU;
	}

	VkCommandPoolCreateInfo cpool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = device->queue_family,
	};
	res = vkCreateCommandPool(device->base, &cpool_info, NULL,
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
	res = vkCreateSemaphore(device->base, &semaphore_info, NULL,
		&render->timeline_semaphore);
	if (res != VK_SUCCESS) {
		wlf_vk_error("vkCreateSemaphore failed", res);
		goto error;
	}

	return &render->base;

error:
	wlf_vk_renderer_destroy(render);
	return NULL;
}

const char *wlf_vulkan_strerror(VkResult err) {
	switch (err) {
	case VK_SUCCESS:
		return "SUCCESS";
	case VK_NOT_READY:
		return "NOT_READY";
	case VK_TIMEOUT:
		return "TIMEOUT";
	case VK_EVENT_SET:
		return "EVENT_SET";
	case VK_EVENT_RESET:
		return "EVENT_RESET";
	case VK_INCOMPLETE:
		return "INCOMPLETE";
	case VK_SUBOPTIMAL_KHR:
		return "SUBOPTIMAL_KHR";

	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_FRAGMENTED_POOL:
		return "ERROR_FRAGMENTED_POOL";
	case VK_ERROR_FRAGMENTATION:
		return "ERROR_FRAGMENTATION";

	case VK_ERROR_INITIALIZATION_FAILED:
		return "ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "ERROR_DEVICE_LOST";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "ERROR_TOO_MANY_OBJECTS";

	case VK_ERROR_LAYER_NOT_PRESENT:
		return "ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "ERROR_FORMAT_NOT_SUPPORTED";

	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
		return "ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";

	case VK_PIPELINE_COMPILE_REQUIRED:
		return "PIPELINE_COMPILE_REQUIRED";

	case VK_ERROR_SURFACE_LOST_KHR:
		return "ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "ERROR_INCOMPATIBLE_DISPLAY_KHR";

	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		return "ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";

	case VK_ERROR_UNKNOWN:
		return "ERROR_UNKNOWN";

	default:
		return "UNKNOWN_ERROR";
	}
}
