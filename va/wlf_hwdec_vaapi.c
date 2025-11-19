#include "wlf/va/wlf_hwdec.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <va/va.h>
#include <va/va_wayland.h>

struct vaapi_hwdec_priv {
	VADisplay va_display;
	VAConfigID config_id;
	VAContextID context_id;
	VASurfaceID *surfaces;
	uint32_t num_surfaces;
	VASurfaceID current_surface;  /* Currently decoded surface */
};

static bool vaapi_init(struct wlf_hwdec_device *device) {
	struct vaapi_hwdec_priv *priv = calloc(1, sizeof(struct vaapi_hwdec_priv));
	if (!priv) {
		return false;
	}

	device->priv = priv;

	/* Initialize VA-API - simplified for now */
	wlf_log(WLF_DEBUG, "VA-API hwdec backend initialized");
	return true;
}

static void vaapi_destroy(struct wlf_hwdec_device *device) {
	struct vaapi_hwdec_priv *priv = device->priv;
	if (!priv) {
		return;
	}

	if (priv->surfaces) {
		/* Destroy VA surfaces */
		free(priv->surfaces);
	}

	if (priv->context_id) {
		vaDestroyContext(priv->va_display, priv->context_id);
	}

	if (priv->config_id) {
		vaDestroyConfig(priv->va_display, priv->config_id);
	}

	free(priv);
	device->priv = NULL;
}

static bool vaapi_supports_codec(struct wlf_hwdec_device *device,
	enum wlf_video_codec codec) {

	/* VA-API supports most codecs */
	switch (codec) {
		case WLF_VIDEO_CODEC_H264:
		case WLF_VIDEO_CODEC_H265:
		case WLF_VIDEO_CODEC_AV1:
		case WLF_VIDEO_CODEC_VP9:
			return true;
		default:
			return false;
	}
}

static bool vaapi_supports_format(struct wlf_hwdec_device *device, uint32_t format) {
	/* Check if format is supported - simplified */
	return true;
}

static bool vaapi_decode_frame(struct wlf_hwdec_device *device,
	const uint8_t *bitstream, size_t size,
	struct wlf_video_image *output) {

	struct vaapi_hwdec_priv *priv = device->priv;
	if (!priv) {
		return false;
	}

	/* Actual VA-API decode implementation would go here */
	wlf_log(WLF_DEBUG, "VA-API decode frame: %zu bytes", size);

	/* Store the decoded surface ID in output for later export */
	if (output && priv->current_surface != VA_INVALID_SURFACE) {
		/* Store VA surface ID in the image structure */
		output->image = (VkImage)(uintptr_t)priv->current_surface;
	.export_to_wl_buffer = vaapi_export_to_wl_buffer,
	}

	return true;
}

static struct wl_buffer *vaapi_export_to_wl_buffer(
	struct wlf_hwdec_device *device,
	struct wlf_video_image *image,
	struct wl_display *wl_display) {

	struct vaapi_hwdec_priv *priv = device->priv;
	if (!priv || !image) {
		return NULL;
	}

	/* Get VA surface ID from image */
	VASurfaceID va_surface = (VASurfaceID)(uintptr_t)image->image;
	if (va_surface == VA_INVALID_SURFACE) {
		wlf_log(WLF_ERROR, "Invalid VA surface");
		return NULL;
	}

	/* Get wl_buffer from VA surface */
	struct wl_buffer *buffer = NULL;
	VAStatus status = vaGetSurfaceBufferWl(
		priv->va_display,
		va_surface,
		VA_FRAME_PICTURE,
		&buffer
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to get wl_buffer from VA surface: %d", status);
		return NULL;
	}

	wlf_log(WLF_DEBUG, "Exported VA surface to wl_buffer");
	return buffer;
}

const struct wlf_hwdec_device_impl wlf_hwdec_vaapi_impl = {
	.name = "vaapi",
	.init = vaapi_init,
	.destroy = vaapi_destroy,
	.supports_codec = vaapi_supports_codec,
	.supports_format = vaapi_supports_format,
	.decode_frame = vaapi_decode_frame,
};
