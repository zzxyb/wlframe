/**
 * @file        video_encoder_example.c
 * @brief       Example demonstrating Vulkan video encoder usage in wlframe.
 * @details     This example shows how to:
 *              - Initialize Vulkan device and queues for video encoding
 *              - Create and configure a video encoder
 *              - Encode YUV frames to H.264/H.265/AV1
 *              - Handle encoded bitstream output
 *
 * @author      YaoBing Xiao
 * @date        2026-01-23
 * @version     v1.0
 */

#include "wlf/video/wlf_video_encoder.h"
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
		.pApplicationName = "Video Encoder Example",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "wlframe",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_3,
	};

	const char *extensions[] = {
		VK_KHR_VIDEO_QUEUE_EXTENSION_NAME,
		VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME,
		VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME,
		VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME,
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
	*physical_device = devices[0];
	free(devices);

	/* Find video encode queue family */
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*physical_device, &queue_family_count, NULL);

	VkQueueFamilyProperties *queue_families =
		malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(*physical_device, &queue_family_count, queue_families);

	*queue_family_index = UINT32_MAX;
	for (uint32_t i = 0; i < queue_family_count; i++) {
		if (queue_families[i].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
			*queue_family_index = i;
			break;
		}
	}
	free(queue_families);

	if (*queue_family_index == UINT32_MAX) {
		wlf_log(WLF_ERROR, "No video encode queue family found");
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
		VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME,
		VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME,
		VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME,
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

/* Frame encoded callback */
static void on_frame_encoded(struct wlf_signal *signal, void *data) {
	struct wlf_video_encoder *encoder = data;
	wlf_log(WLF_INFO, "Frame %lu encoded", encoder->frame_count);
}

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL, NULL);

	wlf_log(WLF_INFO, "=== wlframe Video Encoder Example ===");

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

	/* Query encoder capabilities */
	VkVideoCapabilitiesKHR capabilities;
	if (!wlf_video_encoder_query_capabilities(physical_device,
		WLF_VIDEO_CODEC_H264, &capabilities)) {
		wlf_log(WLF_ERROR, "H.264 encode not supported");
		vkDestroyDevice(device, NULL);
		vkDestroyInstance(instance, NULL);
		return 1;
	}

	/* Configure encoder */
	struct wlf_video_encoder_config config = {
		.codec = WLF_VIDEO_CODEC_H264,
		.width = 1920,
		.height = 1080,
		.framerate_num = 30,
		.framerate_den = 1,
		.chroma = WLF_VIDEO_CHROMA_420,
		.bit_depth = 8,

		/* Rate control */
		.rate_control_mode = WLF_VIDEO_RATE_CONTROL_CBR,
		.target_bitrate = 5000000, /* 5 Mbps */
		.max_bitrate = 6000000,    /* 6 Mbps */

		/* GOP structure */
		.gop_size = 60,            /* I-frame every 2 seconds at 30fps */
		.num_b_frames = 2,
		.use_open_gop = false,

		/* Profile/level */
		.profile = 100,            /* High profile */
		.level = 41,               /* Level 4.1 */
	};

	/* Create encoder */
	struct wlf_video_encoder *encoder = wlf_video_encoder_create(
		device, physical_device, &config);

	if (!encoder) {
		wlf_log(WLF_ERROR, "Failed to create video encoder");
		vkDestroyDevice(device, NULL);
		vkDestroyInstance(instance, NULL);
		return 1;
	}

	/* Register frame encoded callback */
	wlf_signal_add(&encoder->events.frame_encoded, on_frame_encoded);

	wlf_log(WLF_INFO, "Encoder created successfully");
	wlf_log(WLF_INFO, "Codec: %s", wlf_video_codec_to_string(config.codec));
	wlf_log(WLF_INFO, "Resolution: %ux%u @ %u/%u fps",
		config.width, config.height,
		config.framerate_num, config.framerate_den);
	wlf_log(WLF_INFO, "Bitrate: %u kbps (CBR)", config.target_bitrate / 1000);
	wlf_log(WLF_INFO, "GOP: %u frames, %u B-frames", config.gop_size, config.num_b_frames);

	/* In a real application, you would:
	 * 1. Read YUV frames from file or camera
	 * 2. Upload to Vulkan images
	 * 3. Call wlf_video_encoder_encode_frame() for each frame
	 * 4. Write encoded bitstream to file
	 */

	wlf_log(WLF_INFO, "Encoder ready for use");
	wlf_log(WLF_INFO, "To encode frames, call wlf_video_encoder_encode_frame()");

	/* Cleanup */
	wlf_video_encoder_destroy(encoder);
	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);

	wlf_log(WLF_INFO, "Example completed successfully");
	return 0;
}
