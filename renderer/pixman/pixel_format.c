#include "wlf/buffer/pixman/buffer.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"

#include <inttypes.h>
#include <drm_fourcc.h>

static const struct wlf_pixman_pixel_format formats[] = {
	{
		.drm_format = WLF_FORMAT_ARGB8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_b8g8r8a8,
#else
		.pixman_format = PIXMAN_a8r8g8b8,
#endif
	},
	{
		.drm_format = WLF_FORMAT_XBGR8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_r8g8b8x8,
#else
		.pixman_format = PIXMAN_x8b8g8r8,
#endif
	},
	{
		.drm_format = WLF_FORMAT_XRGB8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_b8g8r8x8,
#else
		.pixman_format = PIXMAN_x8r8g8b8,
#endif
	},
	{
		.drm_format = WLF_FORMAT_ABGR8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_r8g8b8a8,
#else
		.pixman_format = PIXMAN_a8b8g8r8,
#endif
	},
	{
		.drm_format = WLF_FORMAT_RGBA8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_a8b8g8r8,
#else
		.pixman_format = PIXMAN_r8g8b8a8,
#endif
	},
	{
		.drm_format = WLF_FORMAT_RGBX8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_x8b8g8r8,
#else
		.pixman_format = PIXMAN_r8g8b8x8,
#endif
	},
	{
		.drm_format = WLF_FORMAT_BGRA8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_a8r8g8b8,
#else
		.pixman_format = PIXMAN_b8g8r8a8,
#endif
	},
	{
		.drm_format = WLF_FORMAT_BGRX8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_x8r8g8b8,
#else
		.pixman_format = PIXMAN_b8g8r8x8,
#endif
	},
	{
		.drm_format = WLF_FORMAT_RGB888,
		.pixman_format = PIXMAN_r8g8b8,
	},
	{
		/* RGB byte data is represented by BGR888 in DRM notation. */
		.drm_format = WLF_FORMAT_BGR888,
		.pixman_format = PIXMAN_b8g8r8,
	},
	{
		.drm_format = WLF_FORMAT_R8,
		.pixman_format = PIXMAN_g8,
	},
#if WLF_LITTLE_ENDIAN
	{
		.drm_format = WLF_FORMAT_RGB565,
		.pixman_format = PIXMAN_r5g6b5,
	},
	{
		.drm_format = WLF_FORMAT_BGR565,
		.pixman_format = PIXMAN_b5g6r5,
	},
	{
		.drm_format = WLF_FORMAT_ARGB2101010,
		.pixman_format = PIXMAN_a2r10g10b10,
	},
	{
		.drm_format = WLF_FORMAT_XRGB2101010,
		.pixman_format = PIXMAN_x2r10g10b10,
	},
	{
		.drm_format = WLF_FORMAT_ABGR2101010,
		.pixman_format = PIXMAN_a2b10g10r10,
	},
	{
		.drm_format = WLF_FORMAT_XBGR2101010,
		.pixman_format = PIXMAN_x2b10g10r10,
	},
	{
		.drm_format = DRM_FORMAT_ABGR16161616,
		.pixman_format = PIXMAN_a16b16g16r16,
	},
#endif
};

pixman_format_code_t get_pixman_format_from_drm(uint32_t format) {
	for (size_t i = 0; i < sizeof(formats) / sizeof(*formats); i++) {
		if (formats[i].drm_format == format) {
			return formats[i].pixman_format;
		}
	}

	wlf_log(WLF_ERROR, "DRM format 0x%"PRIX32" has no pixman equivalent", format);
	return 0;
}

uint32_t get_drm_format_from_pixman(pixman_format_code_t format) {
	for (size_t i = 0; i < sizeof(formats) / sizeof(*formats); i++) {
		if (formats[i].pixman_format == format) {
			return formats[i].drm_format;
		}
	}

	wlf_log(WLF_ERROR, "pixman format 0x%"PRIX32" has no DRM equivalent", format);
	return WLF_FORMAT_INVALID;
}

const uint32_t *get_pixman_drm_formats(size_t *length) {
	static uint32_t drm_formats[sizeof(formats) / sizeof(*formats)];
	*length = sizeof(formats) / sizeof(*formats);
	for (size_t i = 0; i < *length; i++) {
		drm_formats[i] = formats[i].drm_format;
	}

	return drm_formats;
}
