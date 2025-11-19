#include "wlf/va/wlf_video_decoder.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Base decoder implementation */

const char *wlf_video_codec_to_string(enum wlf_video_codec codec) {
	switch (codec) {
		case WLF_VIDEO_CODEC_H264: return "H.264";
		case WLF_VIDEO_CODEC_H265: return "H.265";
		case WLF_VIDEO_CODEC_AV1: return "AV1";
		case WLF_VIDEO_CODEC_VP9: return "VP9";
		default: return "Unknown";
	}
}

const char *wlf_video_chroma_to_string(enum wlf_video_chroma_format chroma) {
	switch (chroma) {
		case WLF_VIDEO_CHROMA_MONOCHROME: return "4:0:0";
		case WLF_VIDEO_CHROMA_420: return "4:2:0";
		case WLF_VIDEO_CHROMA_422: return "4:2:2";
		case WLF_VIDEO_CHROMA_444: return "4:4:4";
		default: return "Unknown";
	}
}

struct wlf_video_decoder *wlf_video_decoder_create(
	const struct wlf_video_decoder_config *config) {

	if (!config) {
		wlf_log(WLF_ERROR, "Invalid decoder configuration");
		return NULL;
	}

	/* This is a base interface - should be implemented by specific backends */
	wlf_log(WLF_ERROR, "Base decoder cannot be instantiated directly. "
		"Use wlf_vk_video_decoder_create(), wlf_va_video_decoder_create(), "
		"or wlf_sw_video_decoder_create() instead.");

	return NULL;
}

void wlf_video_decoder_destroy(struct wlf_video_decoder *decoder) {
	if (!decoder || !decoder->impl || !decoder->impl->destroy) {
		return;
	}

	decoder->impl->destroy(decoder);
}

bool wlf_video_decoder_decode_frame(struct wlf_video_decoder *decoder,
	const uint8_t *bitstream_data, size_t bitstream_size,
	struct wlf_video_image *output_image) {

	if (!decoder || !decoder->impl || !decoder->impl->decode_frame) {
		wlf_log(WLF_ERROR, "Invalid decoder or implementation");
		return false;
	}

	return decoder->impl->decode_frame(decoder, bitstream_data, bitstream_size, output_image);
}

void wlf_video_decoder_flush(struct wlf_video_decoder *decoder) {
	if (!decoder || !decoder->impl || !decoder->impl->flush) {
		return;
	}

	decoder->impl->flush(decoder);
}
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

bool wlf_video_decoder_query_capabilities(VkPhysicalDevice physical_device,
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

	wlf_log(WLF_INFO, "Video decoder capabilities for %s:",
		wlf_video_codec_to_string(codec));
	wlf_log(WLF_INFO, "  Max coded extent: %ux%u",
		capabilities->maxCodedExtent.width, capabilities->maxCodedExtent.height);
	wlf_log(WLF_INFO, "  Max DPB slots: %u", capabilities->maxDpbSlots);
	wlf_log(WLF_INFO, "  Max active references: %u", capabilities->maxActiveReferencePictures);

	return true;
}

static bool decoder_create_session(struct wlf_video_decoder *decoder) {
	assert(decoder);

	VkVideoCodecOperationFlagBitsKHR codec_op = codec_to_vk_operation(decoder->config.codec);

	VkVideoProfileInfoKHR profile = {
		.sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
		.videoCodecOperation = codec_op,
		.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR,
		.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
		.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
	};

	VkExtent2D max_extent = {
		.width = decoder->config.max_width,
		.height = decoder->config.max_height,
	};

	VkVideoSessionCreateInfoKHR session_create_info = {
		.sType = VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR,
		.queueFamilyIndex = decoder->queue_family_index,
		.pVideoProfile = &profile,
		.pictureFormat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
void wlf_video_decoder_flush(struct wlf_video_decoder *decoder) {
	if (!decoder || !decoder->impl || !decoder->impl->flush) {
		return;
	}

	decoder->impl->flush(decoder);
}
