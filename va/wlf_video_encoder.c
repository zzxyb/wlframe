#include "wlf/video/wlf_video_encoder.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* H.264 encoder implementation */
struct wlf_h264_encoder_data {
	VkVideoEncodeH264ProfileInfoKHR profile;
	VkVideoEncodeH264CapabilitiesKHR capabilities;
	VkVideoEncodeH264RateControlInfoKHR rate_control;
	StdVideoH264SequenceParameterSet sps;
	StdVideoH264PictureParameterSet pps;
	uint32_t idr_pic_id;
	uint32_t frame_num;
};

/* H.265 encoder implementation */
struct wlf_h265_encoder_data {
	VkVideoEncodeH265ProfileInfoKHR profile;
	VkVideoEncodeH265CapabilitiesKHR capabilities;
	VkVideoEncodeH265RateControlInfoKHR rate_control;
	StdVideoH265VideoParameterSet vps;
	StdVideoH265SequenceParameterSet sps;
	StdVideoH265PictureParameterSet pps;
	uint32_t poc;
};

/* AV1 encoder implementation */
struct wlf_av1_encoder_data {
	VkVideoEncodeAV1ProfileInfoKHR profile;
	VkVideoEncodeAV1CapabilitiesKHR capabilities;
	VkVideoEncodeAV1RateControlInfoKHR rate_control;
	StdVideoAV1SequenceHeader seq_header;
	uint32_t order_hint;
	uint32_t frame_id;
};

static bool encoder_create_session(struct wlf_video_encoder *encoder);
static bool encoder_create_dpb(struct wlf_video_encoder *encoder);
static void encoder_destroy_dpb(struct wlf_video_encoder *encoder);
static bool encoder_init_rate_control(struct wlf_video_encoder *encoder);

static bool encoder_encode_frame_impl(struct wlf_video_encoder *encoder,
	const struct wlf_video_image *input_image,
	struct wlf_video_encoded_frame *output_frame) {

	if (!encoder || !input_image || !output_frame) {
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

	/* Prepare encode command buffer */
	VkCommandBuffer cmd_buffer;
	VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	/* Encode operation would go here */
	/* This is a simplified implementation - full implementation would handle:
	 * - Rate control updates
	 * - Reference frame management
	 * - Slice encoding
	 * - Bitstream generation
	 */

	/* Update output frame info */
	output_frame->type = frame_type;
	output_frame->is_keyframe = is_keyframe;
	output_frame->pts = encoder->current_pts;
	output_frame->dts = encoder->current_pts; /* Simplified - would need reordering for B-frames */

	/* Increment counters */
	encoder->frame_count++;
	encoder->current_pts += (encoder->config.framerate_den * 1000000000ULL) / encoder->config.framerate_num;

	wlf_signal_emit_mutable(&encoder->events.frame_encoded, encoder);

	wlf_log(WLF_DEBUG, "Encoded frame %lu as %s (size: %zu bytes)",
		encoder->frame_count,
		frame_type == WLF_VIDEO_FRAME_TYPE_IDR ? "IDR" :
		frame_type == WLF_VIDEO_FRAME_TYPE_P ? "P" : "B",
		output_frame->size);

	return true;
}

static void encoder_flush_impl(struct wlf_video_encoder *encoder) {
	if (!encoder) {
		return;
	}

	/* Wait for all encode operations to complete */
	vkDeviceWaitIdle(encoder->device);

	wlf_log(WLF_DEBUG, "Video encoder flushed");
}

static void encoder_destroy_impl(struct wlf_video_encoder *encoder) {
	if (!encoder) {
		return;
	}

	wlf_signal_emit_mutable(&encoder->events.destroy, encoder);

	/* Destroy video session */
	if (encoder->video_session != VK_NULL_HANDLE) {
		vkDestroyVideoSessionKHR(encoder->device, encoder->video_session, NULL);
	}

	if (encoder->session_params != VK_NULL_HANDLE) {
		vkDestroyVideoSessionParametersKHR(encoder->device, encoder->session_params, NULL);
	}

	/* Destroy DPB */
	encoder_destroy_dpb(encoder);

	/* Destroy output buffer */
	if (encoder->output_buffer) {
		wlf_video_buffer_destroy(encoder->output_buffer, encoder->device);
		free(encoder->output_buffer);
	}

	/* Free codec-specific data */
	if (encoder->codec_data) {
		free(encoder->codec_data);
	}

	free(encoder);
}

static const struct wlf_video_encoder_impl encoder_impl = {
	.encode_frame = encoder_encode_frame_impl,
	.flush = encoder_flush_impl,
	.destroy = encoder_destroy_impl,
};

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

bool wlf_video_encoder_query_capabilities(VkPhysicalDevice physical_device,
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

	wlf_log(WLF_INFO, "Video encoder capabilities for %s:",
		wlf_video_codec_to_string(codec));
	wlf_log(WLF_INFO, "  Max coded extent: %ux%u",
		capabilities->maxCodedExtent.width, capabilities->maxCodedExtent.height);
	wlf_log(WLF_INFO, "  Max DPB slots: %u", capabilities->maxDpbSlots);
	wlf_log(WLF_INFO, "  Max active references: %u", capabilities->maxActiveReferencePictures);

	return true;
}

static bool encoder_create_session(struct wlf_video_encoder *encoder) {
	assert(encoder);

	VkVideoCodecOperationFlagBitsKHR codec_op = codec_to_vk_operation(encoder->config.codec);

	VkVideoProfileInfoKHR profile = {
		.sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
		.videoCodecOperation = codec_op,
		.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR,
		.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
		.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
	};

	VkExtent2D max_extent = {
		.width = encoder->config.width,
		.height = encoder->config.height,
	};

	uint32_t max_dpb_slots = encoder->config.num_b_frames + 2; /* Simplified calculation */

	VkVideoSessionCreateInfoKHR session_create_info = {
		.sType = VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR,
		.queueFamilyIndex = encoder->queue_family_index,
		.pVideoProfile = &profile,
		.pictureFormat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
		.maxCodedExtent = max_extent,
		.referencePictureFormat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
		.maxDpbSlots = max_dpb_slots,
		.maxActiveReferencePictures = max_dpb_slots,
	};

	VkResult result = vkCreateVideoSessionKHR(encoder->device,
		&session_create_info, NULL, &encoder->video_session);

	if (result != VK_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to create video session: %d", result);
		return false;
	}

	wlf_log(WLF_DEBUG, "Created video session for %s encoder",
		wlf_video_codec_to_string(encoder->config.codec));

	return true;
}

static bool encoder_create_dpb(struct wlf_video_encoder *encoder) {
	assert(encoder);

	encoder->dpb_count = encoder->config.num_b_frames + 2;
	encoder->dpb_images = calloc(encoder->dpb_count, sizeof(struct wlf_video_image *));

	if (!encoder->dpb_images) {
		wlf_log(WLF_ERROR, "Failed to allocate DPB image array");
		return false;
	}

	for (uint32_t i = 0; i < encoder->dpb_count; i++) {
		encoder->dpb_images[i] = calloc(1, sizeof(struct wlf_video_image));
		if (!encoder->dpb_images[i]) {
			wlf_log(WLF_ERROR, "Failed to allocate DPB image %u", i);
			return false;
		}

		/* Create image for DPB slot - simplified */
		encoder->dpb_images[i]->width = encoder->config.width;
		encoder->dpb_images[i]->height = encoder->config.height;
		encoder->dpb_images[i]->format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
		encoder->dpb_images[i]->ref_count = 1;
	}

	wlf_log(WLF_DEBUG, "Created DPB with %u slots", encoder->dpb_count);
	return true;
}

static void encoder_destroy_dpb(struct wlf_video_encoder *encoder) {
	if (!encoder || !encoder->dpb_images) {
		return;
	}

	for (uint32_t i = 0; i < encoder->dpb_count; i++) {
		if (encoder->dpb_images[i]) {
			/* Destroy Vulkan resources */
			if (encoder->dpb_images[i]->image_view != VK_NULL_HANDLE) {
				vkDestroyImageView(encoder->device, encoder->dpb_images[i]->image_view, NULL);
			}
			if (encoder->dpb_images[i]->image != VK_NULL_HANDLE) {
				vkDestroyImage(encoder->device, encoder->dpb_images[i]->image, NULL);
			}
			if (encoder->dpb_images[i]->memory != VK_NULL_HANDLE) {
				vkFreeMemory(encoder->device, encoder->dpb_images[i]->memory, NULL);
			}
			free(encoder->dpb_images[i]);
		}
	}

	free(encoder->dpb_images);
	encoder->dpb_images = NULL;
	encoder->dpb_count = 0;
}

static bool encoder_init_rate_control(struct wlf_video_encoder *encoder) {
	assert(encoder);

	/* Rate control would be initialized here based on config */
	switch (encoder->config.rate_control_mode) {
		case WLF_VIDEO_RATE_CONTROL_CBR:
			wlf_log(WLF_DEBUG, "Initializing CBR rate control: %u kbps",
				encoder->config.target_bitrate / 1000);
			break;
		case WLF_VIDEO_RATE_CONTROL_VBR:
			wlf_log(WLF_DEBUG, "Initializing VBR rate control: %u-%u kbps",
				encoder->config.target_bitrate / 1000,
				encoder->config.max_bitrate / 1000);
			break;
		case WLF_VIDEO_RATE_CONTROL_CQP:
			wlf_log(WLF_DEBUG, "Initializing CQP rate control: QP I=%u P=%u B=%u",
				encoder->config.qp_i, encoder->config.qp_p, encoder->config.qp_b);
			break;
		case WLF_VIDEO_RATE_CONTROL_DISABLED:
			wlf_log(WLF_DEBUG, "Rate control disabled");
			break;
	}

	return true;
}

struct wlf_video_encoder *wlf_video_encoder_create(
	VkDevice device,
	VkPhysicalDevice physical_device,
	const struct wlf_video_encoder_config *config) {

	if (!device || !physical_device || !config) {
		wlf_log(WLF_ERROR, "Invalid parameters for encoder creation");
		return NULL;
	}

	/* Validate configuration */
	if (config->width == 0 || config->height == 0) {
		wlf_log(WLF_ERROR, "Invalid dimensions: %ux%u", config->width, config->height);
		return NULL;
	}

	if (config->framerate_num == 0 || config->framerate_den == 0) {
		wlf_log(WLF_ERROR, "Invalid framerate: %u/%u",
			config->framerate_num, config->framerate_den);
		return NULL;
	}

	struct wlf_video_encoder *encoder = calloc(1, sizeof(struct wlf_video_encoder));
	if (!encoder) {
		wlf_log(WLF_ERROR, "Failed to allocate encoder");
		return NULL;
	}

	encoder->impl = &encoder_impl;
	encoder->device = device;
	encoder->physical_device = physical_device;
	encoder->config = *config;
	encoder->frame_count = 0;
	encoder->current_pts = 0;

	wlf_signal_init(&encoder->events.frame_encoded);
	wlf_signal_init(&encoder->events.destroy);

	/* Allocate codec-specific data */
	switch (config->codec) {
		case WLF_VIDEO_CODEC_H264:
			encoder->codec_data = calloc(1, sizeof(struct wlf_h264_encoder_data));
			break;
		case WLF_VIDEO_CODEC_H265:
			encoder->codec_data = calloc(1, sizeof(struct wlf_h265_encoder_data));
			break;
		case WLF_VIDEO_CODEC_AV1:
			encoder->codec_data = calloc(1, sizeof(struct wlf_av1_encoder_data));
			break;
		default:
			wlf_log(WLF_ERROR, "Unsupported codec: %d", config->codec);
			free(encoder);
			return NULL;
	}

	if (!encoder->codec_data) {
		wlf_log(WLF_ERROR, "Failed to allocate codec data");
		free(encoder);
		return NULL;
	}

	/* Create video session */
	if (!encoder_create_session(encoder)) {
		wlf_video_encoder_destroy(encoder);
		return NULL;
	}

	/* Create DPB */
	if (!encoder_create_dpb(encoder)) {
		wlf_video_encoder_destroy(encoder);
		return NULL;
	}

	/* Initialize rate control */
	if (!encoder_init_rate_control(encoder)) {
		wlf_video_encoder_destroy(encoder);
		return NULL;
	}

	/* Create output buffer */
	encoder->output_buffer = calloc(1, sizeof(struct wlf_video_buffer));
	if (!encoder->output_buffer) {
		wlf_log(WLF_ERROR, "Failed to allocate output buffer");
		wlf_video_encoder_destroy(encoder);
		return NULL;
	}

	if (!wlf_video_buffer_init(encoder->output_buffer, device,
		4 * 1024 * 1024, VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR)) {
		wlf_log(WLF_ERROR, "Failed to create output buffer");
		wlf_video_encoder_destroy(encoder);
		return NULL;
	}

	wlf_log(WLF_INFO, "Created %s video encoder (%ux%u @ %u/%u fps)",
		wlf_video_codec_to_string(config->codec),
		config->width, config->height,
		config->framerate_num, config->framerate_den);

	return encoder;
}

void wlf_video_encoder_destroy(struct wlf_video_encoder *encoder) {
	if (!encoder || !encoder->impl || !encoder->impl->destroy) {
		return;
	}

	encoder->impl->destroy(encoder);
}

bool wlf_video_encoder_encode_frame(struct wlf_video_encoder *encoder,
	const struct wlf_video_image *input_image,
	struct wlf_video_encoded_frame *output_frame) {

	if (!encoder || !encoder->impl || !encoder->impl->encode_frame) {
		return false;
	}

	return encoder->impl->encode_frame(encoder, input_image, output_frame);
}

void wlf_video_encoder_flush(struct wlf_video_encoder *encoder) {
	if (!encoder || !encoder->impl || !encoder->impl->flush) {
		return;
	}

	encoder->impl->flush(encoder);
}
