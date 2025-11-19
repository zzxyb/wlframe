#include "wlf/va/wlf_video_decoder.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>

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
