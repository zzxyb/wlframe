#include <wlf/va/wlf_hwdec.h>
#include <wlf/utils/wlf_log.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	/* Initialize logging */
	wlf_log_init(WLF_DEBUG);

	wlf_log(WLF_INFO, "=== wlframe Hardware Decode Backend Test ===\n");

	/* Create hwdec context with auto backend selection */
	struct wlf_hwdec_context *ctx = wlf_hwdec_context_create("auto", true);
	if (!ctx) {
		wlf_log(WLF_ERROR, "Failed to create hwdec context");
		return 1;
	}

	/* Test different codecs */
	enum wlf_video_codec codecs[] = {
		WLF_VIDEO_CODEC_H264,
		WLF_VIDEO_CODEC_H265,
		WLF_VIDEO_CODEC_AV1,
		WLF_VIDEO_CODEC_VP9,
	};

	const char *codec_names[] = {
		"H.264",
		"H.265/HEVC",
		"AV1",
		"VP9",
	};

	for (size_t i = 0; i < sizeof(codecs) / sizeof(codecs[0]); i++) {
		wlf_log(WLF_INFO, "\nTesting %s decode support:", codec_names[i]);

		struct wlf_hwdec_device *device = wlf_hwdec_context_select_device(
			ctx, codecs[i], 0);

		if (device) {
			wlf_log(WLF_INFO, "  ✓ Supported by backend: %s", device->impl->name);
		} else {
			wlf_log(WLF_INFO, "  ✗ Not supported by any backend");
		}
	}

	/* Test specific backend types */
	wlf_log(WLF_INFO, "\n=== Testing Individual Backends ===\n");

	const char *backends[] = {
		"vulkan",
		"vaapi",
		"software",
		NULL,
	};

	const char *backend_descriptions[] = {
		"Vulkan",
		"VA-API",
		"Software (FFmpeg)",
	};

	for (size_t i = 0; backends[i]; i++) {
		wlf_log(WLF_INFO, "Testing %s backend:", backend_descriptions[i]);

		struct wlf_hwdec_context *test_ctx = wlf_hwdec_context_create(
			backends[i], false);

			/* Test H.264 support */
			struct wlf_hwdec_device *device = wlf_hwdec_context_select_device(
				test_ctx, WLF_VIDEO_CODEC_H264, 0);

			if (device) {
				wlf_log(WLF_INFO, "  ✓ H.264 decode supported");
			} else {
				wlf_log(WLF_INFO, "  ✗ H.264 decode not supported");
			}

			wlf_hwdec_context_destroy(test_ctx);
		} else {
			wlf_log(WLF_INFO, "  ✗ Backend not available");
		}
	}

	/* Test decode with actual data (mock) */
	wlf_log(WLF_INFO, "\n=== Testing Decode Operation ===\n");

	struct wlf_hwdec_device *device = wlf_hwdec_context_select_device(
		ctx, WLF_VIDEO_CODEC_H264, 0);

	if (device) {
		wlf_log(WLF_INFO, "Using backend: %s", device->impl->name);

		/* Mock bitstream data */
		uint8_t mock_bitstream[] = {0x00, 0x00, 0x00, 0x01, 0x67};
		struct wlf_video_image output = {0};

		bool result = wlf_hwdec_device_decode(device, mock_bitstream,
			sizeof(mock_bitstream), &output);

		if (result) {
			wlf_log(WLF_INFO, "✓ Decode operation successful");
		} else {
			wlf_log(WLF_WARNING, "✗ Decode operation failed (expected with mock data)");
		}
	}

	/* Cleanup */
	wlf_hwdec_context_destroy(ctx);

	wlf_log(WLF_INFO, "\n=== Test Complete ===");
	return 0;
}
