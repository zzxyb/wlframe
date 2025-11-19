/**
 * @file        wlf_vk_video_decoder.c
 * @brief       Vulkan video decoder implementation
 * @author      YaoBing Xiao
 * @date        2026-01-30
 */

#include "wlf/va/wlf_vk_video_decoder.h"
#include "wlf/renderer/vulkan/renderer.h"
#include "wlf/renderer/vulkan/device.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* H.264 decoder data */
struct wlf_h264_decoder_data {
	VkVideoDecodeH264ProfileInfoKHR profile;
	VkVideoDecodeH264CapabilitiesKHR capabilities;
	StdVideoH264SequenceParameterSet sps;
	StdVideoH264PictureParameterSet pps;
	bool sps_valid;
	bool pps_valid;
};

/* H.265 decoder data */
struct wlf_h265_decoder_data {
	VkVideoDecodeH265ProfileInfoKHR profile;
	VkVideoDecodeH265CapabilitiesKHR capabilities;
	StdVideoH265VideoParameterSet vps;
	StdVideoH265SequenceParameterSet sps;
	StdVideoH265PictureParameterSet pps;
	bool vps_valid;
	bool sps_valid;
	bool pps_valid;
};

/* AV1 decoder data */
struct wlf_av1_decoder_data {
	VkVideoDecodeAV1ProfileInfoKHR profile;
	VkVideoDecodeAV1CapabilitiesKHR capabilities;
	StdVideoAV1SequenceHeader seq_header;
	bool seq_header_valid;
};

/* Helper functions */
static VkVideoCodecOperationFlagBitsKHR codec_to_vk_operation(enum wlf_video_codec codec) {
	switch (codec) {
		case WLF_VIDEO_CODEC_H264:
			return VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR;
		case WLF_VIDEO_CODEC_H265:
			return VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR;
		case WLF_VIDEO_CODEC_AV1:
			return VK_VIDEO_CODEC_OPERATION_DECODE_AV1_BIT_KHR;
		case WLF_VIDEO_CODEC_VP9:
			return VK_VIDEO_CODEC_OPERATION_DECODE_VP9_BIT_KHR;
		default:
			return 0;
	}
}

static bool vk_decoder_create_session(struct wlf_vk_video_decoder *vk_decoder) {
	assert(vk_decoder);

	VkVideoCodecOperationFlagBitsKHR codec_op =
		codec_to_vk_operation(vk_decoder->base.config.codec);

	VkVideoProfileInfoKHR profile = {
		.sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
		.videoCodecOperation = codec_op,
		.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR,
		.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
		.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
	};

	VkExtent2D max_extent = {
		.width = vk_decoder->base.config.max_width,
		.height = vk_decoder->base.config.max_height,
	};

	VkVideoSessionCreateInfoKHR session_create_info = {
		.sType = VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR,
		.queueFamilyIndex = vk_decoder->queue_family_index,
		.pVideoProfile = &profile,
		.pictureFormat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
		.maxCodedExtent = max_extent,
		.referencePictureFormat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
		.maxDpbSlots = vk_decoder->base.config.max_dpb_slots,
		.maxActiveReferencePictures = vk_decoder->base.config.max_active_references,
	};

	VkResult result = vkCreateVideoSessionKHR(vk_decoder->device,
		&session_create_info, NULL, &vk_decoder->video_session);

	if (result != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create video session: %d", result);
		return false;
	}

	wlf_log(WLF_DEBUG, "Created Vulkan video session for %s decoder",
		wlf_video_codec_to_string(vk_decoder->base.config.codec));

	return true;
}

static bool vk_decoder_create_dpb(struct wlf_vk_video_decoder *vk_decoder) {
	assert(vk_decoder);

	vk_decoder->dpb_count = vk_decoder->base.config.max_dpb_slots;
	vk_decoder->dpb_images = calloc(vk_decoder->dpb_count,
		sizeof(struct wlf_video_image *));

	if (!vk_decoder->dpb_images) {
		wlf_log(WLF_ERROR, "Failed to allocate DPB image array");
		return false;
	}

	for (uint32_t i = 0; i < vk_decoder->dpb_count; i++) {
		vk_decoder->dpb_images[i] = calloc(1, sizeof(struct wlf_video_image));
		if (!vk_decoder->dpb_images[i]) {
			wlf_log(WLF_ERROR, "Failed to allocate DPB image %u", i);
			return false;
		}

		/* Create image for DPB slot - simplified */
		vk_decoder->dpb_images[i]->width = vk_decoder->base.config.max_width;
		vk_decoder->dpb_images[i]->height = vk_decoder->base.config.max_height;
		vk_decoder->dpb_images[i]->format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
		vk_decoder->dpb_images[i]->ref_count = 1;
	}

	wlf_log(WLF_DEBUG, "Created DPB with %u slots", vk_decoder->dpb_count);
	return true;
}

static void vk_decoder_destroy_dpb(struct wlf_vk_video_decoder *vk_decoder) {
	if (!vk_decoder || !vk_decoder->dpb_images) {
		return;
	}

	for (uint32_t i = 0; i < vk_decoder->dpb_count; i++) {
		if (vk_decoder->dpb_images[i]) {
			/* Destroy Vulkan resources */
			if (vk_decoder->dpb_images[i]->image_view != VK_NULL_HANDLE) {
				vkDestroyImageView(vk_decoder->device,
					vk_decoder->dpb_images[i]->image_view, NULL);
			}
			if (vk_decoder->dpb_images[i]->image != VK_NULL_HANDLE) {
				vkDestroyImage(vk_decoder->device,
					vk_decoder->dpb_images[i]->image, NULL);
			}
			if (vk_decoder->dpb_images[i]->memory != VK_NULL_HANDLE) {
				vkFreeMemory(vk_decoder->device,
					vk_decoder->dpb_images[i]->memory, NULL);
			}
			free(vk_decoder->dpb_images[i]);
		}
	}

	free(vk_decoder->dpb_images);
	vk_decoder->dpb_images = NULL;
	vk_decoder->dpb_count = 0;
}

/* Implementation functions */
static bool vk_decoder_decode_frame_impl(struct wlf_video_decoder *decoder,
	const uint8_t *bitstream_data, size_t bitstream_size,
	struct wlf_video_image *output_image) {

	struct wlf_vk_video_decoder *vk_decoder =
		wlf_vk_video_decoder_from_decoder(decoder);

	if (!vk_decoder || !bitstream_data || !output_image) {
		wlf_log(WLF_ERROR, "Invalid parameters for decode_frame");
		return false;
	}

	/* Copy bitstream to GPU buffer */
	void *mapped = wlf_video_buffer_map(vk_decoder->bitstream_buffer,
		vk_decoder->device);
	if (!mapped) {
		wlf_log(WLF_ERROR, "Failed to map bitstream buffer");
		return false;
	}

	memcpy(mapped, bitstream_data, bitstream_size);
	wlf_video_buffer_unmap(vk_decoder->bitstream_buffer, vk_decoder->device);

	/* TODO: Implement actual decode operation with command buffers */

	wlf_signal_emit_mutable(&decoder->events.frame_decoded, decoder);
	return true;
}

static void vk_decoder_flush_impl(struct wlf_video_decoder *decoder) {
	struct wlf_vk_video_decoder *vk_decoder =
		wlf_vk_video_decoder_from_decoder(decoder);

	if (!vk_decoder) {
		return;
	}

	/* Wait for all decode operations to complete */
	vkDeviceWaitIdle(vk_decoder->device);

	wlf_log(WLF_DEBUG, "Vulkan video decoder flushed");
}

static void vk_decoder_destroy_impl(struct wlf_video_decoder *decoder) {
	struct wlf_vk_video_decoder *vk_decoder =
		wlf_vk_video_decoder_from_decoder(decoder);

	if (!vk_decoder) {
		return;
	}

	wlf_signal_emit_mutable(&decoder->events.destroy, decoder);

	/* Destroy video session */
	if (vk_decoder->video_session != VK_NULL_HANDLE) {
		vkDestroyVideoSessionKHR(vk_decoder->device,
			vk_decoder->video_session, NULL);
	}

	if (vk_decoder->session_params != VK_NULL_HANDLE) {
		vkDestroyVideoSessionParametersKHR(vk_decoder->device,
			vk_decoder->session_params, NULL);
	}

	/* Destroy DPB */
	vk_decoder_destroy_dpb(vk_decoder);

	/* Destroy bitstream buffer */
	if (vk_decoder->bitstream_buffer) {
		wlf_video_buffer_destroy(vk_decoder->bitstream_buffer, vk_decoder->device);
		free(vk_decoder->bitstream_buffer);
	}

	/* Free codec-specific data */
	if (vk_decoder->codec_data) {
		free(vk_decoder->codec_data);
	}

	/* Clean up VkDevice if we own it */
	if (vk_decoder->owns_vk_device && vk_decoder->vk_device) {
		wlf_vk_device_destroy(vk_decoder->vk_device);
	}

	free(vk_decoder);
}

static const struct wlf_video_decoder_impl vk_decoder_impl = {
	.decode_frame = vk_decoder_decode_frame_impl,
	.flush = vk_decoder_flush_impl,
	.destroy = vk_decoder_destroy_impl,
};

/* Public API */

struct wlf_video_decoder *wlf_vk_video_decoder_create(
	const struct wlf_vk_video_decoder_config *config) {

	if (!config) {
		wlf_log(WLF_ERROR, "Invalid Vulkan decoder configuration");
		return NULL;
	}

	struct wlf_vk_video_decoder *vk_decoder =
		calloc(1, sizeof(struct wlf_vk_video_decoder));
	if (!vk_decoder) {
		wlf_log(WLF_ERROR, "Failed to allocate Vulkan decoder");
		return NULL;
	}

	/* Initialize base decoder */
	vk_decoder->base.impl = &vk_decoder_impl;
	vk_decoder->base.config = config->base;
	wlf_signal_init(&vk_decoder->base.events.frame_decoded);
	wlf_signal_init(&vk_decoder->base.events.destroy);

	/* Setup Vulkan resources */
	if (config->renderer) {
		/* Option 1: Use renderer's device */
		vk_decoder->vk_device = config->renderer->dev;
		vk_decoder->device = vk_decoder->vk_device->base;
		vk_decoder->physical_device = vk_decoder->vk_device->phdev;
		vk_decoder->queue_family_index = vk_decoder->vk_device->queue_family;
		vk_decoder->decode_queue = vk_decoder->vk_device->queue;
		vk_decoder->owns_vk_device = false;
	} else if (config->vk_device) {
		/* Option 2: Use provided VkDevice wrapper */
		vk_decoder->vk_device = config->vk_device;
		vk_decoder->device = vk_decoder->vk_device->base;
		vk_decoder->physical_device = vk_decoder->vk_device->phdev;
		vk_decoder->queue_family_index = vk_decoder->vk_device->queue_family;
		vk_decoder->decode_queue = vk_decoder->vk_device->queue;
		vk_decoder->owns_vk_device = false;
	} else if (config->device && config->physical_device) {
		/* Option 3: Use custom Vulkan objects */
		vk_decoder->device = config->device;
		vk_decoder->physical_device = config->physical_device;
		vk_decoder->decode_queue = config->decode_queue;
		vk_decoder->queue_family_index = config->queue_family_index;
		vk_decoder->vk_device = NULL;
		vk_decoder->owns_vk_device = false;
	} else {
		wlf_log(WLF_ERROR, "No valid Vulkan device source provided");
		free(vk_decoder);
		return NULL;
	}

	/* Allocate codec-specific data */
	switch (config->base.codec) {
		case WLF_VIDEO_CODEC_H264:
			vk_decoder->codec_data = calloc(1, sizeof(struct wlf_h264_decoder_data));
			break;
		case WLF_VIDEO_CODEC_H265:
			vk_decoder->codec_data = calloc(1, sizeof(struct wlf_h265_decoder_data));
			break;
		case WLF_VIDEO_CODEC_AV1:
			vk_decoder->codec_data = calloc(1, sizeof(struct wlf_av1_decoder_data));
			break;
		default:
			wlf_log(WLF_ERROR, "Unsupported codec: %d", config->base.codec);
			free(vk_decoder);
			return NULL;
	}

	if (!vk_decoder->codec_data) {
		wlf_log(WLF_ERROR, "Failed to allocate codec data");
		free(vk_decoder);
		return NULL;
	}

	/* Create video session */
	if (!vk_decoder_create_session(vk_decoder)) {
		wlf_video_decoder_destroy(&vk_decoder->base);
		return NULL;
	}

	/* Create DPB */
	if (!vk_decoder_create_dpb(vk_decoder)) {
		wlf_video_decoder_destroy(&vk_decoder->base);
		return NULL;
	}

	/* Create bitstream buffer */
	vk_decoder->bitstream_buffer = calloc(1, sizeof(struct wlf_video_buffer));
	if (!vk_decoder->bitstream_buffer) {
		wlf_log(WLF_ERROR, "Failed to allocate bitstream buffer");
		wlf_video_decoder_destroy(&vk_decoder->base);
		return NULL;
	}

	if (!wlf_video_buffer_init(vk_decoder->bitstream_buffer, vk_decoder->device,
		4 * 1024 * 1024, VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR)) {
		wlf_log(WLF_ERROR, "Failed to create bitstream buffer");
		wlf_video_decoder_destroy(&vk_decoder->base);
		return NULL;
	}

	wlf_log(WLF_INFO, "Created Vulkan %s video decoder (%ux%u)",
		wlf_video_codec_to_string(config->base.codec),
		config->base.max_width, config->base.max_height);

	return &vk_decoder->base;
}

struct wlf_video_decoder *wlf_vk_video_decoder_create_from_renderer(
	struct wlf_vk_renderer *renderer,
	const struct wlf_video_decoder_config *config) {

	struct wlf_vk_video_decoder_config vk_config = {
		.base = *config,
		.renderer = renderer,
	};

	return wlf_vk_video_decoder_create(&vk_config);
}

struct wlf_video_decoder *wlf_vk_video_decoder_create_from_device(
	struct wlf_vk_device *vk_device,
	const struct wlf_video_decoder_config *config) {

	struct wlf_vk_video_decoder_config vk_config = {
		.base = *config,
		.vk_device = vk_device,
	};

	return wlf_vk_video_decoder_create(&vk_config);
}

bool wlf_video_decoder_is_vk(struct wlf_video_decoder *decoder) {
	return decoder && decoder->impl == &vk_decoder_impl;
}

struct wlf_vk_video_decoder *wlf_vk_video_decoder_from_decoder(
	struct wlf_video_decoder *decoder) {

	if (!wlf_video_decoder_is_vk(decoder)) {
		return NULL;
	}

	return (struct wlf_vk_video_decoder *)decoder;
}

bool wlf_vk_video_decoder_query_capabilities(VkPhysicalDevice physical_device,
	enum wlf_video_codec codec, VkVideoCapabilitiesKHR *capabilities) {

	if (!physical_device || !capabilities) {
		return false;
	}

	VkVideoCodecOperationFlagBitsKHR codec_op = codec_to_vk_operation(codec);
	if (codec_op == 0) {
		wlf_log(WLF_ERROR, "Unsupported codec: %d", codec);
		return false;
	}

	VkVideoProfileInfoKHR profile = {
		.sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
		.videoCodecOperation = codec_op,
		.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR,
		.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
		.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
	};

	capabilities->sType = VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR;
	capabilities->pNext = NULL;

	VkResult result = vkGetPhysicalDeviceVideoCapabilitiesKHR(
		physical_device, &profile, capabilities);

	if (result != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to query video capabilities: %d", result);
		return false;
	}

	wlf_log(WLF_INFO, "Vulkan video decoder capabilities for %s:",
		wlf_video_codec_to_string(codec));
	wlf_log(WLF_INFO, "  Max coded extent: %ux%u",
		capabilities->maxCodedExtent.width, capabilities->maxCodedExtent.height);
	wlf_log(WLF_INFO, "  Max DPB slots: %u", capabilities->maxDpbSlots);
	wlf_log(WLF_INFO, "  Max active references: %u",
		capabilities->maxActiveReferencePictures);

	return true;
}
