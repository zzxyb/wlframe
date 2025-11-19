#include "wlf/va/wlf_video_encoder.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>

/* Base encoder implementation */

const char *wlf_video_rate_control_mode_to_string(enum wlf_video_rate_control_mode mode) {
	switch (mode) {
		case WLF_VIDEO_RATE_CONTROL_DISABLED: return "Disabled";
		case WLF_VIDEO_RATE_CONTROL_CBR: return "CBR";
		case WLF_VIDEO_RATE_CONTROL_VBR: return "VBR";
		case WLF_VIDEO_RATE_CONTROL_CQP: return "CQP";
		default: return "Unknown";
	}
}

struct wlf_video_encoder *wlf_video_encoder_create(
	const struct wlf_video_encoder_config *config) {

	if (!config) {
		wlf_log(WLF_ERROR, "Invalid encoder configuration");
		return NULL;
	}

	/* This is a base interface - should be implemented by specific backends */
	wlf_log(WLF_ERROR, "Base encoder cannot be instantiated directly. "
		"Use wlf_vk_video_encoder_create(), wlf_va_video_encoder_create(), "
		"or wlf_sw_video_encoder_create() instead.");

	return NULL;
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
		wlf_log(WLF_ERROR, "Invalid encoder or implementation");
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
