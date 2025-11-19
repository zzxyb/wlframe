/**
 * @file        wlf_vk_video_encoder.c
 * @brief       Vulkan video encoder implementation
 * @author      YaoBing Xiao
 * @date        2026-01-30
 */

#include "wlf/va/wlf_vk_video_encoder.h"
#include "wlf/renderer/vulkan/renderer.h"
#include "wlf/renderer/vulkan/device.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* H.264 encoder data */
struct wlf_h264_encoder_data {
	VkVideoEncodeH264ProfileInfoKHR profile;
	VkVideoEncodeH264CapabilitiesKHR capabilities;
	VkVideoEncodeH264RateControlInfoKHR rate_control;
	StdVideoH264SequenceParameterSet sps;
	StdVideoH264PictureParameterSet pps;
	uint32_t idr_pic_id;
	uint32_t frame_num;
};

/* H.265 encoder data */
struct wlf_h265_encoder_data {
	VkVideoEncodeH265ProfileInfoKHR profile;
	VkVideoEncodeH265CapabilitiesKHR capabilities;
	VkVideoEncodeH265RateControlInfoKHR rate_control;
	StdVideoH265VideoParameterSet vps;
	StdVideoH265SequenceParameterSet sps;
	StdVideoH265PictureParameterSet pps;
	uint32_t poc;
};

/* AV1 encoder data */
struct wlf_av1_encoder_data {
	VkVideoEncodeAV1ProfileInfoKHR profile;
	VkVideoEncodeAV1CapabilitiesKHR capabilities;
	VkVideoEncodeAV1RateControlInfoKHR rate_control;
	StdVideoAV1SequenceHeader seq_header;
	uint32_t order_hint;
	uint32_t frame_id;
};

/* Helper functions */
static VkVideoCodecOperationFlagBitsKHR codec_to_vk_operation(enum wlf_video_codec codec) {
	switch (codec) {
		case WLF_VIDEO_CODEC_H264:
			return VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR;
		case WLF_VIDEO_CODEC_H265:
			return VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR;
		case WLF_VIDEO_CODEC_AV1:
			return VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR;
		default:
			return 0;
	}
}

static bool vk_encoder_create_session(struct wlf_vk_video_encoder *vk_encoder) {
	assert(vk_encoder);

	VkVideoCodecOperationFlagBitsKHR codec_op =
		codec_to_vk_operation(vk_encoder->base.config.codec);

	VkVideoProfileInfoKHR profile = {
		.sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
		.videoCodecOperation = codec_op,
		.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR,
		.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
		.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
	};

	VkExtent2D max_extent = {
		.width = vk_encoder->base.config.width,
		.height = vk_encoder->base.config.height,
	};

	VkVideoSessionCreateInfoKHR session_create_info = {
		.sType = VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR,
		.queueFamilyIndex = vk_encoder->queue_family_index,
		.pVideoProfile = &profile,
		.pictureFormat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
		.maxCodedExtent = max_extent,
		.referencePictureFormat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
		.maxDpbSlots = vk_encoder->base.config.gop_size > 0 ?
			vk_encoder->base.config.gop_size : 16,
		.maxActiveReferencePictures = vk_encoder->base.config.num_b_frames + 2,
	};

	VkResult result = vkCreateVideoSessionKHR(vk_encoder->device,
		&session_create_info, NULL, &vk_encoder->video_session);

	if (result != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create video session: %d", result);
		return false;
	}

	wlf_log(WLF_DEBUG, "Created Vulkan video session for %s encoder",
		wlf_video_codec_to_string(vk_encoder->base.config.codec));

	return true;
}

static bool vk_encoder_create_dpb(struct wlf_vk_video_encoder *vk_encoder) {
	assert(vk_encoder);

	vk_encoder->dpb_count = vk_encoder->base.config.gop_size > 0 ?
		vk_encoder->base.config.gop_size : 16;
	vk_encoder->dpb_images = calloc(vk_encoder->dpb_count,
		sizeof(struct wlf_video_image *));

	if (!vk_encoder->dpb_images) {
		wlf_log(WLF_ERROR, "Failed to allocate DPB image array");
		return false;
	}

	for (uint32_t i = 0; i < vk_encoder->dpb_count; i++) {
		vk_encoder->dpb_images[i] = calloc(1, sizeof(struct wlf_video_image));
		if (!vk_encoder->dpb_images[i]) {
			wlf_log(WLF_ERROR, "Failed to allocate DPB image %u", i);
			return false;
		}

		/* Create image for DPB slot - simplified */
		vk_encoder->dpb_images[i]->width = vk_encoder->base.config.width;
		vk_encoder->dpb_images[i]->height = vk_encoder->base.config.height;
		vk_encoder->dpb_images[i]->format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
		vk_encoder->dpb_images[i]->ref_count = 1;
	}

	wlf_log(WLF_DEBUG, "Created DPB with %u slots", vk_encoder->dpb_count);
	return true;
}

static void vk_encoder_destroy_dpb(struct wlf_vk_video_encoder *vk_encoder) {
	if (!vk_encoder || !vk_encoder->dpb_images) {
		return;
	}

	for (uint32_t i = 0; i < vk_encoder->dpb_count; i++) {
		if (vk_encoder->dpb_images[i]) {
			/* Destroy Vulkan resources */
			if (vk_encoder->dpb_images[i]->image_view != VK_NULL_HANDLE) {
				vkDestroyImageView(vk_encoder->device,
					vk_encoder->dpb_images[i]->image_view, NULL);
			}
			if (vk_encoder->dpb_images[i]->image != VK_NULL_HANDLE) {
				vkDestroyImage(vk_encoder->device,
					vk_encoder->dpb_images[i]->image, NULL);
			}
			if (vk_encoder->dpb_images[i]->memory != VK_NULL_HANDLE) {
				vkFreeMemory(vk_encoder->device,
					vk_encoder->dpb_images[i]->memory, NULL);
			}
			free(vk_encoder->dpb_images[i]);
		}
	}

	free(vk_encoder->dpb_images);
	vk_encoder->dpb_images = NULL;
	vk_encoder->dpb_count = 0;
}

/* Implementation functions */
static bool vk_encoder_encode_frame_impl(struct wlf_video_encoder *encoder,
	const struct wlf_video_image *input_image,
	struct wlf_video_encoded_frame *output_frame) {

	struct wlf_vk_video_encoder *vk_encoder =
		wlf_vk_video_encoder_from_encoder(encoder);

	if (!vk_encoder || !input_image || !output_frame) {
		wlf_log(WLF_ERROR, "Invalid parameters for encode_frame");
		return false;
	}

	/* Validate input image dimensions */
	if (input_image->width != encoder->config.width ||
	    input_image->height != encoder->config.height) {
		wlf_log(WLF_ERROR, "Input image size mismatch: got %ux%u, expected %ux%u",
			input_image->width, input_image->height,
			encoder->config.width, encoder->config.height);
		return false;
	}

	/* Determine frame type based on GOP structure */
	enum wlf_video_frame_type frame_type;
	bool is_keyframe = false;

	if (encoder->frame_count % encoder->config.gop_size == 0) {
		frame_type = WLF_VIDEO_FRAME_TYPE_IDR;
		is_keyframe = true;
	} else if (encoder->config.num_b_frames > 0 &&
	           (encoder->frame_count % (encoder->config.num_b_frames + 1)) != 0) {
		frame_type = WLF_VIDEO_FRAME_TYPE_B;
	} else {
		frame_type = WLF_VIDEO_FRAME_TYPE_P;
	}

	/* TODO: Implement actual encode operation with command buffers */

	/* Update output frame info */
	output_frame->type = frame_type;
	output_frame->is_keyframe = is_keyframe;
	output_frame->pts = encoder->current_pts;
	output_frame->dts = encoder->current_pts;

	/* Increment counters */
	encoder->frame_count++;
	encoder->current_pts += (encoder->config.framerate_den * 1000000000ULL) /
		encoder->config.framerate_num;

	wlf_signal_emit_mutable(&encoder->events.frame_encoded, encoder);

	wlf_log(WLF_DEBUG, "Encoded frame %lu as %s",
		encoder->frame_count,
		frame_type == WLF_VIDEO_FRAME_TYPE_IDR ? "IDR" :
		frame_type == WLF_VIDEO_FRAME_TYPE_P ? "P" : "B");

	return true;
}

static void vk_encoder_flush_impl(struct wlf_video_encoder *encoder) {
	struct wlf_vk_video_encoder *vk_encoder =
		wlf_vk_video_encoder_from_encoder(encoder);

	if (!vk_encoder) {
		return;
	}

	/* Wait for all encode operations to complete */
	vkDeviceWaitIdle(vk_encoder->device);

	wlf_log(WLF_DEBUG, "Vulkan video encoder flushed");
}

static void vk_encoder_destroy_impl(struct wlf_video_encoder *encoder) {
	struct wlf_vk_video_encoder *vk_encoder =
		wlf_vk_video_encoder_from_encoder(encoder);

	if (!vk_encoder) {
		return;
	}

	wlf_signal_emit_mutable(&encoder->events.destroy, encoder);

	/* Destroy video session */
	if (vk_encoder->video_session != VK_NULL_HANDLE) {
		vkDestroyVideoSessionKHR(vk_encoder->device,
			vk_encoder->video_session, NULL);
	}

	if (vk_encoder->session_params != VK_NULL_HANDLE) {
		vkDestroyVideoSessionParametersKHR(vk_encoder->device,
			vk_encoder->session_params, NULL);
	}

	/* Destroy DPB */
	vk_encoder_destroy_dpb(vk_encoder);

	/* Destroy output buffer */
	if (vk_encoder->output_buffer) {
		wlf_video_buffer_destroy(vk_encoder->output_buffer, vk_encoder->device);
		free(vk_encoder->output_buffer);
	}

	/* Free codec-specific data */
	if (vk_encoder->codec_data) {
		free(vk_encoder->codec_data);
	}

	/* Clean up VkDevice if we own it */
	if (vk_encoder->owns_vk_device && vk_encoder->vk_device) {
		wlf_vk_device_destroy(vk_encoder->vk_device);
	}

	free(vk_encoder);
}

static const struct wlf_video_encoder_impl vk_encoder_impl = {
	.encode_frame = vk_encoder_encode_frame_impl,
	.flush = vk_encoder_flush_impl,
	.destroy = vk_encoder_destroy_impl,
};

/* Public API */

struct wlf_video_encoder *wlf_vk_video_encoder_create(
	const struct wlf_vk_video_encoder_config *config) {

	if (!config) {
		wlf_log(WLF_ERROR, "Invalid Vulkan encoder configuration");
		return NULL;
	}

	struct wlf_vk_video_encoder *vk_encoder =
		calloc(1, sizeof(struct wlf_vk_video_encoder));
	if (!vk_encoder) {
		wlf_log(WLF_ERROR, "Failed to allocate Vulkan encoder");
		return NULL;
	}

	/* Initialize base encoder */
	vk_encoder->base.impl = &vk_encoder_impl;
	vk_encoder->base.config = config->base;
	vk_encoder->base.frame_count = 0;
	vk_encoder->base.current_pts = 0;
	wlf_signal_init(&vk_encoder->base.events.frame_encoded);
	wlf_signal_init(&vk_encoder->base.events.destroy);

	/* Setup Vulkan resources */
	if (config->renderer) {
		/* Option 1: Use renderer's device */
		vk_encoder->vk_device = config->renderer->dev;
		vk_encoder->device = vk_encoder->vk_device->base;
		vk_encoder->physical_device = vk_encoder->vk_device->phdev;
		vk_encoder->queue_family_index = vk_encoder->vk_device->queue_family;
		vk_encoder->encode_queue = vk_encoder->vk_device->queue;
		vk_encoder->owns_vk_device = false;
	} else if (config->vk_device) {
		/* Option 2: Use provided VkDevice wrapper */
		vk_encoder->vk_device = config->vk_device;
		vk_encoder->device = vk_encoder->vk_device->base;
		vk_encoder->physical_device = vk_encoder->vk_device->phdev;
		vk_encoder->queue_family_index = vk_encoder->vk_device->queue_family;
		vk_encoder->encode_queue = vk_encoder->vk_device->queue;
		vk_encoder->owns_vk_device = false;
	} else if (config->device && config->physical_device) {
		/* Option 3: Use custom Vulkan objects */
		vk_encoder->device = config->device;
		vk_encoder->physical_device = config->physical_device;
		vk_encoder->encode_queue = config->encode_queue;
		vk_encoder->queue_family_index = config->queue_family_index;
		vk_encoder->vk_device = NULL;
		vk_encoder->owns_vk_device = false;
	} else {
		wlf_log(WLF_ERROR, "No valid Vulkan device source provided");
		free(vk_encoder);
		return NULL;
	}

	/* Allocate codec-specific data */
	switch (config->base.codec) {
		case WLF_VIDEO_CODEC_H264:
			vk_encoder->codec_data = calloc(1, sizeof(struct wlf_h264_encoder_data));
			break;
		case WLF_VIDEO_CODEC_H265:
			vk_encoder->codec_data = calloc(1, sizeof(struct wlf_h265_encoder_data));
			break;
		case WLF_VIDEO_CODEC_AV1:
			vk_encoder->codec_data = calloc(1, sizeof(struct wlf_av1_encoder_data));
			break;
		default:
			wlf_log(WLF_ERROR, "Unsupported codec: %d", config->base.codec);
			free(vk_encoder);
			return NULL;
	}

	if (!vk_encoder->codec_data) {
		wlf_log(WLF_ERROR, "Failed to allocate codec data");
		free(vk_encoder);
		return NULL;
	}

	/* Create video session */
	if (!vk_encoder_create_session(vk_encoder)) {
		wlf_video_encoder_destroy(&vk_encoder->base);
		return NULL;
	}

	/* Create DPB */
	if (!vk_encoder_create_dpb(vk_encoder)) {
		wlf_video_encoder_destroy(&vk_encoder->base);
		return NULL;
	}

	/* Create output buffer */
	vk_encoder->output_buffer = calloc(1, sizeof(struct wlf_video_buffer));
	if (!vk_encoder->output_buffer) {
		wlf_log(WLF_ERROR, "Failed to allocate output buffer");
		wlf_video_encoder_destroy(&vk_encoder->base);
		return NULL;
	}

	if (!wlf_video_buffer_init(vk_encoder->output_buffer, vk_encoder->device,
		8 * 1024 * 1024, VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR)) {
		wlf_log(WLF_ERROR, "Failed to create output buffer");
		wlf_video_encoder_destroy(&vk_encoder->base);
		return NULL;
	}

	wlf_log(WLF_INFO, "Created Vulkan %s video encoder (%ux%u, %s)",
		wlf_video_codec_to_string(config->base.codec),
		config->base.width, config->base.height,
		wlf_video_rate_control_mode_to_string(config->base.rate_control_mode));

	return &vk_encoder->base;
}

struct wlf_video_encoder *wlf_vk_video_encoder_create_from_renderer(
	struct wlf_vk_renderer *renderer,
	const struct wlf_video_encoder_config *config) {

	struct wlf_vk_video_encoder_config vk_config = {
		.base = *config,
		.renderer = renderer,
	};

	return wlf_vk_video_encoder_create(&vk_config);
}

struct wlf_video_encoder *wlf_vk_video_encoder_create_from_device(
	struct wlf_vk_device *vk_device,
	const struct wlf_video_encoder_config *config) {

	struct wlf_vk_video_encoder_config vk_config = {
		.base = *config,
		.vk_device = vk_device,
	};

	return wlf_vk_video_encoder_create(&vk_config);
}

bool wlf_video_encoder_is_vk(struct wlf_video_encoder *encoder) {
	return encoder && encoder->impl == &vk_encoder_impl;
}

struct wlf_vk_video_encoder *wlf_vk_video_encoder_from_encoder(
	struct wlf_video_encoder *encoder) {

	if (!wlf_video_encoder_is_vk(encoder)) {
		return NULL;
	}

	return (struct wlf_vk_video_encoder *)encoder;
}

bool wlf_vk_video_encoder_query_capabilities(VkPhysicalDevice physical_device,
	enum wlf_video_codec codec, VkVideoCapabilitiesKHR *capabilities) {

	if (!physical_device || !capabilities) {
		return false;
	}

	VkVideoCodecOperationFlagBitsKHR codec_op = codec_to_vk_operation(codec);
	if (codec_op == 0) {
		wlf_log(WLF_ERROR, "Unsupported codec for encoding: %d", codec);
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

	wlf_log(WLF_INFO, "Vulkan video encoder capabilities for %s:",
		wlf_video_codec_to_string(codec));
	wlf_log(WLF_INFO, "  Max coded extent: %ux%u",
		capabilities->maxCodedExtent.width, capabilities->maxCodedExtent.height);
	wlf_log(WLF_INFO, "  Max DPB slots: %u", capabilities->maxDpbSlots);
	wlf_log(WLF_INFO, "  Max active references: %u",
		capabilities->maxActiveReferencePictures);

	return true;
}
