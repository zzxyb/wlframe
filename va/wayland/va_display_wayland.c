#include "wlf/va/wlf_va_display_wayland.h"
#include "wlf/va/wlf_va_display.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <va/va.h>
#include <va/va_wayland.h>

/**
 * Create VA-API display from Wayland display
 */
VADisplay wlf_va_display_create_wayland(struct wl_display *wl_display) {
	if (!wl_display) {
		wlf_log(WLF_ERROR, "Invalid Wayland display");
		return NULL;
	}

	VADisplay va_display = vaGetDisplayWl(wl_display);
	if (!va_display) {
		wlf_log(WLF_ERROR, "Failed to get VA display from Wayland");
		return NULL;
	}

	int major, minor;
	VAStatus status = vaInitialize(va_display, &major, &minor);
	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to initialize VA-API: %d", status);
		return NULL;
	}

	wlf_log(WLF_INFO, "VA-API initialized: version %d.%d", major, minor);

	/* Query vendor string */
	const char *vendor = vaQueryVendorString(va_display);
	if (vendor) {
		wlf_log(WLF_INFO, "VA-API vendor: %s", vendor);
	}

	return va_display;
}

// /**
//  * Destroy VA-API display
//  */
// void wlf_va_display_destroy(VADisplay va_display) {
// 	if (!va_display) {
// 		return;
// 	}

// 	vaTerminate(va_display);
// }

/**
 * Query VA-API capabilities
 */
bool wlf_va_query_codec_support(VADisplay va_display,
	enum wlf_video_codec codec, bool *decode, bool *encode) {

	if (!va_display) {
		return false;
	}

	/* Map wlframe codec to VA profile */
	VAProfile profile;
	switch (codec) {
		case WLF_VIDEO_CODEC_H264:
			profile = VAProfileH264High;
			break;
		case WLF_VIDEO_CODEC_H265:
			profile = VAProfileHEVCMain;
			break;
		case WLF_VIDEO_CODEC_AV1:
			profile = VAProfileAV1Profile0;
			break;
		case WLF_VIDEO_CODEC_VP9:
			profile = VAProfileVP9Profile0;
			break;
		default:
			return false;
	}

	/* Query entrypoints */
	int num_entrypoints;
	VAEntrypoint *entrypoints = calloc(vaMaxNumEntrypoints(va_display),
		sizeof(VAEntrypoint));

	VAStatus status = vaQueryConfigEntrypoints(va_display, profile,
		entrypoints, &num_entrypoints);

	if (status != VA_STATUS_SUCCESS) {
		free(entrypoints);
		return false;
	}

	/* Check for decode/encode support */
	if (decode) {
		*decode = false;
		for (int i = 0; i < num_entrypoints; i++) {
			if (entrypoints[i] == VAEntrypointVLD) {
				*decode = true;
				break;
			}
		}
	}

	if (encode) {
		*encode = false;
		for (int i = 0; i < num_entrypoints; i++) {
			if (entrypoints[i] == VAEntrypointEncSlice ||
				entrypoints[i] == VAEntrypointEncSliceLP) {
				*encode = true;
				break;
			}
		}
	}

	free(entrypoints);
	return true;
}
