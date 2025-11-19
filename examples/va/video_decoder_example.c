/**
 * @file        video_decoder_example.c
 * @brief       Example demonstrating Vulkan video decoder usage in wlframe.
 * @details     This example shows how to:
 *              - Initialize Vulkan device and queues for video decoding
 *              - Create and configure a video decoder
 *              - Decode H.264/H.265/AV1 bitstream
 *              - Handle decoded frames
 *
 * @author      YaoBing Xiao
 * @date        2026-01-23
 * @version     v1.0
 */

#include "wlf/video/wlf_video_decoder.h"
#include "wlf/video/wlf_video_common.h"
#include "wlf/utils/wlf_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simplified Vulkan initialization */
static bool init_vulkan(VkInstance *instance, VkPhysicalDevice *physical_device,
	VkDevice *device, uint32_t *queue_family_index) {

	/* Create Vulkan instance */
	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Video Decoder Example",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "wlframe",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3,
	};

	const char *extensions[] = {
		VK_KHR_VIDEO_QUEUE_EXTENSION_NAME,
		VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME,
		VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME,
		VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME,
	};

	VkInstanceCreateInfo instance_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]),
		.ppEnabledExtensionNames = extensions,
	};

	if (vkCreateInstance(&instance_info, NULL, instance) != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create Vulkan instance");
		return false;
	}

	/* Enumerate physical devices */
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(*instance, &device_count, NULL);

	if (device_count == 0) {
		wlf_log(WLF_ERROR, "No Vulkan devices found");
		return false;
	}

	VkPhysicalDevice *devices = malloc(device_count * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(*instance, &device_count, devices);
	*physical_device = devices[0]; /* Use first device */
	free(devices);

	/* Find video decode queue family */
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*physical_device, &queue_family_count, NULL);

	VkQueueFamilyProperties *queue_families =
		malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(*physical_device, &queue_family_count, queue_families);

	*queue_family_index = UINT32_MAX;
	for (uint32_t i = 0; i < queue_family_count; i++) {
		if (queue_families[i].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
			*queue_family_index = i;
			break;
		}
	}
	free(queue_families);

	if (*queue_family_index == UINT32_MAX) {
		wlf_log(WLF_ERROR, "No video decode queue family found");
		return false;
	}

	/* Create logical device */
	float queue_priority = 1.0f;
	VkDeviceQueueCreateInfo queue_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = *queue_family_index,
		.queueCount = 1,
		.pQueuePriorities = &queue_priority,
	};

	const char *device_extensions[] = {
		VK_KHR_VIDEO_QUEUE_EXTENSION_NAME,
		VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME,
		VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME,
		VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME,
	};

	VkDeviceCreateInfo device_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queue_info,
		.enabledExtensionCount = sizeof(device_extensions) / sizeof(device_extensions[0]),
		.ppEnabledExtensionNames = device_extensions,
	};

	if (vkCreateDevice(*physical_device, &device_info, NULL, device) != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create Vulkan device");
		return false;
	}

	return true;
}

/* Frame decoded callback */
static void on_frame_decoded(struct wlf_signal *signal, void *data) {
	struct wlf_video_decoder *decoder = data;
	wlf_log(WLF_INFO, "Frame decoded successfully");
}

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL, NULL);

	wlf_log(WLF_INFO, "=== wlframe Video Decoder Example ===");

	/* Initialize Vulkan */
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkDevice device;
	uint32_t queue_family_index;

	if (!init_vulkan(&instance, &physical_device, &device, &queue_family_index)) {
		wlf_log(WLF_ERROR, "Failed to initialize Vulkan");
		return 1;
	}

	wlf_log(WLF_INFO, "Vulkan initialized successfully");

	/* Query decoder capabilities */
	VkVideoCapabilitiesKHR capabilities;
	if (!wlf_video_decoder_query_capabilities(physical_device,
		WLF_VIDEO_CODEC_H264, &capabilities)) {
		wlf_log(WLF_ERROR, "H.264 decode not supported");
		vkDestroyDevice(device, NULL);
		vkDestroyInstance(instance, NULL);
		return 1;
	}

	/* Configure decoder */
	struct wlf_video_decoder_config config = {
		.codec = WLF_VIDEO_CODEC_H264,
		.max_width = 1920,
		.max_height = 1080,
		.max_dpb_slots = 16,
		.max_active_references = 16,
		.chroma = WLF_VIDEO_CHROMA_420,
		.bit_depth = 8,
		.enable_film_grain = false,
	};

	/* Create decoder */
	struct wlf_video_decoder *decoder = wlf_video_decoder_create(
		device, physical_device, &config);

	if (!decoder) {
		wlf_log(WLF_ERROR, "Failed to create video decoder");
		vkDestroyDevice(device, NULL);
		vkDestroyInstance(instance, NULL);
		return 1;
	}

	/* Register frame decoded callback */
	wlf_signal_add(&decoder->events.frame_decoded, on_frame_decoded);

	wlf_log(WLF_INFO, "Decoder created successfully");
	wlf_log(WLF_INFO, "Codec: %s", wlf_video_codec_to_string(config.codec));
	wlf_log(WLF_INFO, "Resolution: %ux%u", config.max_width, config.max_height);
	wlf_log(WLF_INFO, "Chroma: %s", wlf_video_chroma_to_string(config.chroma));

	/* In a real application, you would:
	 * 1. Read compressed bitstream from file
	 * 2. Parse into NAL units
	 * 3. Call wlf_video_decoder_decode_frame() for each frame
	 * 4. Handle decoded frames
	 */

	wlf_log(WLF_INFO, "Decoder ready for use");
	wlf_log(WLF_INFO, "To decode frames, call wlf_video_decoder_decode_frame()");

	/* Cleanup */
	wlf_video_decoder_destroy(decoder);
	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);

	wlf_log(WLF_INFO, "Example completed successfully");
	return 0;
}
