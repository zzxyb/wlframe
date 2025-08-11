#include "wlf/renderer/vulkan/vk_device.h"
#include "wlf/renderer/vulkan/vk_renderer.h"
#include "wlf/renderer/vulkan/vk_instance.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"

#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>

static void log_phdev(const VkPhysicalDeviceProperties *props) {
	uint32_t vv_major = VK_VERSION_MAJOR(props->apiVersion);
	uint32_t vv_minor = VK_VERSION_MINOR(props->apiVersion);
	uint32_t vv_patch = VK_VERSION_PATCH(props->apiVersion);

	uint32_t dv_major = VK_VERSION_MAJOR(props->driverVersion);
	uint32_t dv_minor = VK_VERSION_MINOR(props->driverVersion);
	uint32_t dv_patch = VK_VERSION_PATCH(props->driverVersion);

	const char *dev_type = "unknown";
	switch (props->deviceType) {
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		dev_type = "integrated";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		dev_type = "discrete";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		dev_type = "cpu";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		dev_type = "vgpu";
		break;
	default:
		break;
	}

	wlf_log(WLF_INFO, "Vulkan device: '%s'", props->deviceName);
	wlf_log(WLF_INFO, "  Device type: '%s'", dev_type);
	wlf_log(WLF_INFO, "  Supported API version: %u.%u.%u", vv_major, vv_minor, vv_patch);
	wlf_log(WLF_INFO, "  Driver version: %u.%u.%u", dv_major, dv_minor, dv_patch);
}

struct wlf_vk_device *wlf_vk_device_create(struct wlf_vk_instance *instance,
		VkPhysicalDevice phdev) {
	VkResult res;

	uint32_t avail_extc = 0;
	res = vkEnumerateDeviceExtensionProperties(phdev, NULL,
		&avail_extc, NULL);
	if (res != VK_SUCCESS || avail_extc == 0) {
		wlf_vk_error("Could not enumerate device extensions (1)", res);
		return NULL;
	}

	VkExtensionProperties avail_ext_props[avail_extc + 1];
	res = vkEnumerateDeviceExtensionProperties(phdev, NULL,
		&avail_extc, avail_ext_props);
	if (res != VK_SUCCESS) {
		wlf_vk_error("Could not enumerate device extensions (2)", res);
		return NULL;
	}

	for (size_t j = 0; j < avail_extc; ++j) {
		wlf_log(WLF_DEBUG, "Vulkan device extension %s v%"PRIu32,
			avail_ext_props[j].extensionName, avail_ext_props[j].specVersion);
	}

	struct wlf_vk_device *dev = calloc(1, sizeof(*dev));
	if (!dev) {
		wlf_log_errno(WLF_ERROR, "allocation wlf_vk_device failed");
		return NULL;
	}

	dev->phdev = phdev;
	dev->instance = instance;

	const char *extensions[32] = {0};
	size_t extensions_len = 0;
	extensions[extensions_len++] = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
	extensions[extensions_len++] = VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME; // or vulkan 1.2
	extensions[extensions_len++] = VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME;
	extensions[extensions_len++] = VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME;
	extensions[extensions_len++] = VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME;
	extensions[extensions_len++] = VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME; // or vulkan 1.2
	extensions[extensions_len++] = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME; // or vulkan 1.3

	for (size_t i = 0; i < extensions_len; i++) {
		if (!check_extension(avail_ext_props, avail_extc, extensions[i])) {
			wlf_log(WLF_ERROR, "vulkan: required device extension %s not found",
				extensions[i]);
			goto error;
		}
	}

	{
		uint32_t qfam_count;
		vkGetPhysicalDeviceQueueFamilyProperties(phdev, &qfam_count, NULL);
		assert(qfam_count > 0);
		VkQueueFamilyProperties queue_props[qfam_count];
		vkGetPhysicalDeviceQueueFamilyProperties(phdev, &qfam_count,
			queue_props);

		bool graphics_found = false;
		for (unsigned i = 0u; i < qfam_count; ++i) {
			graphics_found = queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
			if (graphics_found) {
				dev->queue_family = i;
				break;
			}
		}
		assert(graphics_found);
	}

	bool exportable_semaphore = false, importable_semaphore = false;
	bool has_external_semaphore_fd =
		check_extension(avail_ext_props, avail_extc, VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
	if (has_external_semaphore_fd) {
		const VkPhysicalDeviceExternalSemaphoreInfo ext_semaphore_info = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO,
			.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT,
		};
		VkExternalSemaphoreProperties ext_semaphore_props = {
			.sType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES,
		};
		vkGetPhysicalDeviceExternalSemaphoreProperties(phdev,
			&ext_semaphore_info, &ext_semaphore_props);
		exportable_semaphore = ext_semaphore_props.externalSemaphoreFeatures &
			VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT;
		importable_semaphore = ext_semaphore_props.externalSemaphoreFeatures &
			VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT;
		extensions[extensions_len++] = VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME;
	}
	if (!exportable_semaphore) {
		wlf_log(WLF_DEBUG, "VkSemaphore is not exportable to a sync_file");
	}
	if (!importable_semaphore) {
		wlf_log(WLF_DEBUG, "VkSemaphore is not importable from a sync_file");
	}

	// bool dmabuf_sync_file_import_export = dmabuf_check_sync_file_import_export();
	// if (!dmabuf_sync_file_import_export) {
	// 	wlf_log(WLF_DEBUG, "DMA-BUF sync_file import/export not supported");
	// }

	// dev->sync_file_import_export = exportable_semaphore && importable_semaphore;
	// dev->implicit_sync_interop =
	// 	exportable_semaphore && importable_semaphore && dmabuf_sync_file_import_export;
	// if (dev->implicit_sync_interop) {
	// 	wlf_log(WLF_DEBUG, "Implicit sync interop supported");
	// } else {
	// 	wlf_log(WLF_INFO, "Implicit sync interop not supported, "
	// 		"falling back to blocking");
	// }

	VkPhysicalDeviceSamplerYcbcrConversionFeatures phdev_sampler_ycbcr_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES,
	};
	VkPhysicalDeviceFeatures2 phdev_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &phdev_sampler_ycbcr_features,
	};
	vkGetPhysicalDeviceFeatures2(phdev, &phdev_features);

	dev->sampler_ycbcr_conversion = phdev_sampler_ycbcr_features.samplerYcbcrConversion;
	wlf_log(WLF_DEBUG, "Sampler YCbCr conversion %s",
		dev->sampler_ycbcr_conversion ? "supported" : "not supported");

	const float prio = 1.f;
	VkDeviceQueueCreateInfo qinfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = dev->queue_family,
		.queueCount = 1,
		.pQueuePriorities = &prio,
	};

	VkDeviceQueueGlobalPriorityCreateInfoKHR global_priority;
	bool has_global_priority = check_extension(avail_ext_props, avail_extc,
		VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME);
	if (has_global_priority) {
		// If global priorities are supported, request a high-priority context
		global_priority = (VkDeviceQueueGlobalPriorityCreateInfoKHR){
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_KHR,
			.globalPriority = VK_QUEUE_GLOBAL_PRIORITY_HIGH_KHR,
		};
		qinfo.pNext = &global_priority;
		extensions[extensions_len++] = VK_KHR_GLOBAL_PRIORITY_EXTENSION_NAME;
		wlf_log(WLF_DEBUG, "Requesting a high-priority device queue");
	} else {
		wlf_log(WLF_DEBUG, "Global priorities are not supported, "
			"falling back to regular queue priority");
	}

	VkPhysicalDeviceSamplerYcbcrConversionFeatures sampler_ycbcr_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES,
		.samplerYcbcrConversion = dev->sampler_ycbcr_conversion,
	};
	VkPhysicalDeviceSynchronization2FeaturesKHR sync2_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
		.pNext = &sampler_ycbcr_features,
		.synchronization2 = VK_TRUE,
	};
	VkPhysicalDeviceTimelineSemaphoreFeaturesKHR timeline_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR,
		.pNext = &sync2_features,
		.timelineSemaphore = VK_TRUE,
	};
	VkDeviceCreateInfo dev_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &timeline_features,
		.queueCreateInfoCount = 1u,
		.pQueueCreateInfos = &qinfo,
		.enabledExtensionCount = extensions_len,
		.ppEnabledExtensionNames = extensions,
	};

	assert(extensions_len <= sizeof(extensions) / sizeof(extensions[0]));

	res = vkCreateDevice(phdev, &dev_info, NULL, &dev->base);

	if (has_global_priority && (res == VK_ERROR_NOT_PERMITTED_EXT ||
			res == VK_ERROR_INITIALIZATION_FAILED)) {
		// Try to recover from the driver denying a global priority queue
		wlf_log(WLF_DEBUG, "Failed to obtain a high-priority device queue, "
			"falling back to regular queue priority");
		qinfo.pNext = NULL;
		res = vkCreateDevice(phdev, &dev_info, NULL, &dev->base);
	}

	if (res != VK_SUCCESS) {
		wlf_vk_error("Failed to create vulkan device", res);
		goto error;
	}

	vkGetDeviceQueue(dev->base, dev->queue_family, 0, &dev->queue);

	load_device_proc(dev, "vkGetMemoryFdPropertiesKHR",
		&dev->api.vkGetMemoryFdPropertiesKHR);
	load_device_proc(dev, "vkWaitSemaphoresKHR", &dev->api.vkWaitSemaphoresKHR);
	load_device_proc(dev, "vkGetSemaphoreCounterValueKHR",
		&dev->api.vkGetSemaphoreCounterValueKHR);
	load_device_proc(dev, "vkQueueSubmit2KHR", &dev->api.vkQueueSubmit2KHR);

	if (has_external_semaphore_fd) {
		load_device_proc(dev, "vkGetSemaphoreFdKHR", &dev->api.vkGetSemaphoreFdKHR);
		load_device_proc(dev, "vkImportSemaphoreFdKHR", &dev->api.vkImportSemaphoreFdKHR);
	}

	return dev;

error:
	wlf_vk_device_destroy(dev);
	return NULL;
}

void wlf_vk_device_destroy(struct wlf_vk_device *device) {
	if (!device) {
		return;
	}

	if (device->instance) {
		wlf_vk_instance_destroy(device->instance);
	}

	if (device->base) {
		vkDestroyDevice(device->base, NULL);
	}

	free(device);
}

void load_device_proc(struct wlf_vk_device *device, const char *name,
		void *proc_ptr) {
	void *proc = (void *)vkGetDeviceProcAddr(device->base, name);
	if (proc == NULL) {
		abort();
	}

	*(void **)proc_ptr = proc;
}

VkPhysicalDevice wlf_vk_find_phdev(struct wlf_vk_instance *instance) {
	VkResult res;
	uint32_t num_phdevs;

	res = vkEnumeratePhysicalDevices(instance->base, &num_phdevs, NULL);
	if (res != VK_SUCCESS) {
		wlf_vk_error("Could not retrieve physical devices", res);
		return VK_NULL_HANDLE;
	}

	VkPhysicalDevice phdevs[1 + num_phdevs];
	res = vkEnumeratePhysicalDevices(instance->base, &num_phdevs, phdevs);
	if (res != VK_SUCCESS) {
		wlf_vk_error("Could not retrieve physical devices", res);
		return VK_NULL_HANDLE;
	}

	bool force_sw = wlf_env_parse_bool("WSM_RENDER_FORCE_SOFTWARE");
	bool force_discrete_gpu = wlf_env_parse_bool("WSM_RENDER_FORCE_DISCRETE_GPU");
	for (uint32_t i = 0; i < num_phdevs; ++i) {
		VkPhysicalDevice phdev = phdevs[i];
		VkPhysicalDeviceProperties phdev_props;
		vkGetPhysicalDeviceProperties(phdev, &phdev_props);

		log_phdev(&phdev_props);

		if (phdev_props.apiVersion < VK_API_VERSION_1_1) {
			// NOTE: we could additionaly check whether the
			// VkPhysicalDeviceProperties2KHR extension is supported but
			// implementations not supporting 1.1 are unlikely in future
			continue;
		}

		// check for extensions
		uint32_t avail_extc = 0;
		res = vkEnumerateDeviceExtensionProperties(phdev, NULL,
			&avail_extc, NULL);
		if ((res != VK_SUCCESS) || (avail_extc == 0)) {
			wlf_vk_error("  Could not enumerate device extensions", res);
			continue;
		}

		VkExtensionProperties avail_ext_props[avail_extc + 1];
		res = vkEnumerateDeviceExtensionProperties(phdev, NULL,
			&avail_extc, avail_ext_props);
		if (res != VK_SUCCESS) {
			wlf_vk_error("  Could not enumerate device extensions", res);
			continue;
		}

		bool has_drm_props = check_extension(avail_ext_props, avail_extc,
			VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME);
		bool has_driver_props = check_extension(avail_ext_props, avail_extc,
			VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME);

		VkPhysicalDeviceProperties2 props = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		};

		VkPhysicalDeviceDrmPropertiesEXT drm_props = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT,
		};
		if (has_drm_props) {
			drm_props.pNext = props.pNext;
			props.pNext = &drm_props;
		}

		VkPhysicalDeviceDriverPropertiesKHR driver_props = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES,
		};
		if (has_driver_props) {
			driver_props.pNext = props.pNext;
			props.pNext = &driver_props;
		}

		vkGetPhysicalDeviceProperties2(phdev, &props);

		if (has_driver_props) {
			wlf_log(WLF_INFO, "  Driver name: %s (%s)", driver_props.driverName, driver_props.driverInfo);
		}

		bool found;

		if (force_sw) {
			wlf_log(WLF_INFO, "WSM_RENDER_FORCE_SOFTWARE env variable is set, "
				"forcing software render");
			found = phdev_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU;
		} else {
			if (force_discrete_gpu) {
				wlf_log(WLF_INFO, "WSM_RENDER_FORCE_DISCRETE_GPU env variable is set, "
					"forcing discrete GPU");
				found = phdev_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
			} else {
				found = phdev_props.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU;
			}
		}

		if (found) {
			wlf_log(WLF_INFO, "Found matching Vulkan physical device: %s",
				phdev_props.deviceName);
			return phdev;
		}
	}

	return VK_NULL_HANDLE;
}
