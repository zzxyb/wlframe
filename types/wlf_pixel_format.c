#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"

static const struct wlf_pixel_format_info wlf_pixel_formats[] = {
	{ WLF_FORMAT_XRGB8888, 0, 4, 1, 1 },
	{ WLF_FORMAT_ARGB8888, WLF_FORMAT_XRGB8888, 4, 1, 1 },

	{ WLF_FORMAT_XBGR8888, 0, 4, 1, 1 },
	{ WLF_FORMAT_ABGR8888, WLF_FORMAT_XBGR8888, 4, 1, 1 },

	{ WLF_FORMAT_RGBX8888, 0, 4, 1, 1 },
	{ WLF_FORMAT_RGBA8888, WLF_FORMAT_RGBX8888, 4, 1, 1 },

	{ WLF_FORMAT_BGRX8888, 0, 4, 1, 1 },
	{ WLF_FORMAT_BGRA8888, WLF_FORMAT_BGRX8888, 4, 1, 1 },

	{ WLF_FORMAT_RGB888, 0, 3, 1, 1 },
	{ WLF_FORMAT_BGR888, 0, 3, 1, 1 },

	{ WLF_FORMAT_RGB565, 0, 2, 1, 1 },
	{ WLF_FORMAT_BGR565, 0, 2, 1, 1 },

	{ WLF_FORMAT_ARGB1555, WLF_FORMAT_XRGB1555, 2, 1, 1 },
	{ WLF_FORMAT_XRGB1555, 0, 2, 1, 1 },

	{ WLF_FORMAT_RGBA4444, WLF_FORMAT_RGBX4444, 2, 1, 1 },
	{ WLF_FORMAT_RGBX4444, 0, 2, 1, 1 },

	{ WLF_FORMAT_ARGB2101010, WLF_FORMAT_XRGB2101010, 4, 1, 1 },
	{ WLF_FORMAT_XRGB2101010, 0, 4, 1, 1 },

	{ WLF_FORMAT_ABGR2101010, WLF_FORMAT_XBGR2101010, 4, 1, 1 },
	{ WLF_FORMAT_XBGR2101010, 0, 4, 1, 1 },

	{ WLF_FORMAT_R8, 0, 1, 1, 1 },

	{ WLF_FORMAT_RG88, 0, 2, 1, 1 },

	{ WLF_FORMAT_R16F, 0, 2, 1, 1 },
	{ WLF_FORMAT_R32F, 0, 4, 1, 1 },

	{ WLF_FORMAT_RG1616F, 0, 4, 1, 1 },
	{ WLF_FORMAT_RG3232F, 0, 8, 1, 1 },

	{ WLF_FORMAT_RGBA16161616F, 0, 8, 1, 1 },
	{ WLF_FORMAT_RGBA32323232F, 0, 16, 1, 1 },

	{ WLF_FORMAT_YVYU, 0, 4, 2, 1 },
	{ WLF_FORMAT_VYUY, 0, 4, 2, 1 },

	{ WLF_FORMAT_NV12, 0, 1, 2, 2 },
	{ WLF_FORMAT_P010, 0, 2, 2, 2 },

	{ WLF_FORMAT_YUV420, 0, 1, 2, 2 },
	{ WLF_FORMAT_YVU420, 0, 1, 2, 2 },
};

static const uint32_t opaque_pixel_formats[] = {
	WLF_FORMAT_XRGB8888,
	WLF_FORMAT_XBGR8888,
	WLF_FORMAT_RGBX8888,
	WLF_FORMAT_BGRX8888,
	WLF_FORMAT_RGB888,
	WLF_FORMAT_BGR888,
	WLF_FORMAT_RGB565,
	WLF_FORMAT_BGR565,
	WLF_FORMAT_XRGB1555,
	WLF_FORMAT_RGBX4444,
	WLF_FORMAT_XRGB2101010,
	WLF_FORMAT_XBGR2101010,
	WLF_FORMAT_R8,
	WLF_FORMAT_RG88,
	WLF_FORMAT_R16F,
	WLF_FORMAT_R32F,
	WLF_FORMAT_RG1616F,
	WLF_FORMAT_RG3232F,
	WLF_FORMAT_YVYU,
	WLF_FORMAT_VYUY,
	WLF_FORMAT_NV12,
	WLF_FORMAT_P010,
	WLF_FORMAT_YUV420,
	WLF_FORMAT_YVU420,
};

static const size_t pixel_format_info_size =
	sizeof(wlf_pixel_formats) / sizeof(wlf_pixel_formats[0]);

static const size_t opaque_pixel_formats_size =
	sizeof(opaque_pixel_formats) / sizeof(opaque_pixel_formats[0]);

static int32_t div_round_up(int32_t dividend, int32_t divisor) {
	int32_t quotient = dividend / divisor;
	if (dividend % divisor != 0) {
		quotient++;
	}

	return quotient;
}

const struct wlf_pixel_format_info *wlf_get_pixel_format_info(uint32_t format) {
	for (size_t i = 0; i < pixel_format_info_size; i++) {
		if (wlf_pixel_formats[i].format == format) {
			return &wlf_pixel_formats[i];
		}
	}

	return NULL;
}

uint32_t pixel_format_info_pixels_per_block(const struct wlf_pixel_format_info *info) {
	uint32_t pixels = info->block_width * info->block_height;

	return pixels > 0 ? pixels : 1;
}

int32_t pixel_format_info_min_stride(const struct wlf_pixel_format_info *info, int32_t width) {
	int32_t pixels_per_block = (int32_t)pixel_format_info_pixels_per_block(info);
	int32_t bytes_per_block = (int32_t)info->bytes_per_block;
	if (width > INT32_MAX / bytes_per_block) {
		wlf_log(WLF_DEBUG, "Invalid width %d (overflow)", width);

		return 0;
	}

	return div_round_up(width * bytes_per_block, pixels_per_block);
}
bool pixel_format_info_check_stride(const struct wlf_pixel_format_info *info,
		int32_t stride, int32_t width) {
	int32_t bytes_per_block = (int32_t)info->bytes_per_block;
	if (stride % bytes_per_block != 0) {
		wlf_log(WLF_DEBUG, "Invalid stride %d (incompatible with %d "
			"bytes-per-block)", stride, bytes_per_block);
		return false;
	}

	int32_t min_stride = pixel_format_info_min_stride(info, width);
	if (min_stride <= 0) {
		return false;
	} else if (stride < min_stride) {
		wlf_log(WLF_DEBUG, "Invalid stride %d (too small for %d "
			"bytes-per-block and width %d)", stride, bytes_per_block, width);
		return false;
	}

	return true;
}

#if WLF_HAS_LINUX_PLATFORM
uint32_t convert_wl_shm_format_to_wlf(enum wl_shm_format fmt) {
	switch (fmt) {
	case WL_SHM_FORMAT_XRGB8888:
		return WLF_FORMAT_XRGB8888;
	case WL_SHM_FORMAT_ARGB8888:
		return WLF_FORMAT_ARGB8888;
	default:
		return WLF_FORMAT_INVALID;
	}
}

enum wl_shm_format convert_wlf_format_to_wl_shm(uint32_t fmt) {
	switch (fmt) {
	case WLF_FORMAT_XRGB8888:
		return WL_SHM_FORMAT_XRGB8888;
	case WLF_FORMAT_ARGB8888:
		return WL_SHM_FORMAT_ARGB8888;
	default:
		return (enum wl_shm_format)fmt;
	}
}
#endif

bool pixel_format_has_alpha(uint32_t fmt) {
	for (size_t i = 0; i < opaque_pixel_formats_size; i++) {
		if (fmt == opaque_pixel_formats[i]) {
			return false;
		}
	}

	return true;
}

bool pixel_format_is_ycbcr(uint32_t fmt) {
	switch (fmt) {
	case WLF_FORMAT_YVYU:
	case WLF_FORMAT_VYUY:
	case WLF_FORMAT_NV12:
	case WLF_FORMAT_P010:
	case WLF_FORMAT_YUV420:
	case WLF_FORMAT_YVU420:
		return true;
	}

	return false;
}