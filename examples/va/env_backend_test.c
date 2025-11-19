#include <wlf/va/wlf_hwdec.h>
#include <wlf/utils/wlf_log.h>
#include <wlf/utils/wlf_env.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	/* Initialize logging */
	wlf_log_init(WLF_DEBUG);

	wlf_log(WLF_INFO, "=== wlframe Environment Variable Backend Test ===\n");

	/* Check current environment variable */
	const char *env_backend = wlf_get_env("WLF_HWDEC_BACKEND");
	if (env_backend) {
		wlf_log(WLF_INFO, "WLF_HWDEC_BACKEND is set to: %s", env_backend);
	} else {
		wlf_log(WLF_INFO, "WLF_HWDEC_BACKEND not set, will use auto mode (prefers VA-API)");
	}

	/* Create context - environment variable will override */
	wlf_log(WLF_INFO, "\nCreating hwdec context with auto mode...");
	struct wlf_hwdec_context *ctx = wlf_hwdec_context_create("auto", true);
	if (!ctx) {
		wlf_log(WLF_ERROR, "Failed to create hwdec context");
		return 1;
	}

	/* Test H.264 decode */
	wlf_log(WLF_INFO, "\nTesting H.264 codec support:");
	struct wlf_hwdec_device *device = wlf_hwdec_get_device(ctx, WLF_VIDEO_CODEC_H264);

	if (device) {
		wlf_log(WLF_INFO, "✓ Selected backend: %s", device->impl->name);
	} else {
		wlf_log(WLF_ERROR, "✗ No backend available for H.264");
	}

	/* Test different codecs */
	wlf_log(WLF_INFO, "\n=== Testing Different Codecs ===");

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
		device = wlf_hwdec_get_device(ctx, codecs[i]);
		if (device) {
			wlf_log(WLF_INFO, "%s: %s", codec_names[i], device->impl->name);
		} else {
			wlf_log(WLF_INFO, "%s: not supported", codec_names[i]);
		}
	}

	/* Show how to use environment variable */
	wlf_log(WLF_INFO, "\n=== Environment Variable Usage ===");
	wlf_log(WLF_INFO, "You can control the backend by setting WLF_HWDEC_BACKEND:");
	wlf_log(WLF_INFO, "  export WLF_HWDEC_BACKEND=vaapi   # Use VA-API");
	wlf_log(WLF_INFO, "  export WLF_HWDEC_BACKEND=vulkan  # Use Vulkan");
	wlf_log(WLF_INFO, "  export WLF_HWDEC_BACKEND=software # Use Software");
	wlf_log(WLF_INFO, "  export WLF_HWDEC_BACKEND=auto    # Auto (prefers VA-API)");

	/* Test specific backend selection */
	wlf_log(WLF_INFO, "\n=== Testing Manual Backend Selection ===");

	const char *test_backends[] = {"vulkan", "vaapi", "software", NULL};

	for (int i = 0; test_backends[i]; i++) {
		wlf_log(WLF_INFO, "\nTrying to create %s backend:", test_backends[i]);

		struct wlf_hwdec_context *test_ctx = wlf_hwdec_context_create(
			test_backends[i], false);

		if (test_ctx) {
			struct wlf_hwdec_device *test_dev = wlf_hwdec_get_device(
				test_ctx, WLF_VIDEO_CODEC_H264);

			if (test_dev) {
				wlf_log(WLF_INFO, "  ✓ Backend available: %s", test_dev->impl->name);
			} else {
				wlf_log(WLF_INFO, "  ✗ Backend created but H.264 not supported");
			}

			wlf_hwdec_context_destroy(test_ctx);
		} else {
			wlf_log(WLF_INFO, "  ✗ Backend not available");
		}
	}

	/* Cleanup */
	wlf_hwdec_context_destroy(ctx);

	wlf_log(WLF_INFO, "\n=== Test Complete ===");
	return 0;
}
