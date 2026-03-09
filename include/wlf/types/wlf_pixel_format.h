/**
 * @file        wlf_pixel_format.h
 * @brief       Pixel format definitions and utilities for wlframe.
 * @details     This file defines the pixel format enumeration, format information
 *              structures, and utility functions for querying pixel format properties
 *              such as stride calculation, alpha channel detection, and YCbCr detection.
 * @author      YaoBing Xiao
 * @date        2026-03-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-09, initial version\n
 */

#ifndef TYPES_WLF_PIXEL_FORMAT_H
#define TYPES_WLF_PIXEL_FORMAT_H

#include "wlf/config.h"
#if WLF_HAS_LINUX_PLATFORM
#include <wayland-client-protocol.h>
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Constructs a fourcc pixel format code from four ASCII characters.
 */
#define WLF_FOURCC(a,b,c,d) \
	((uint32_t)(a) | ((uint32_t)(b)<<8) | ((uint32_t)(c)<<16) | ((uint32_t)(d)<<24))

/**
 * @brief Pixel format identifiers used throughout wlframe.
 *
 * Values are fourcc codes compatible with DRM/KMS format definitions,
 * except for WLF_FORMAT_INVALID which is 0.
 */
enum wlf_pixel_format {
	WLF_FORMAT_INVALID = 0,            /**< Invalid / unspecified format */

	/* 32-bit RGBA */
	WLF_FORMAT_ARGB8888 = WLF_FOURCC('A','R','2','4'),  /**< 32-bit ARGB, 8 bits per channel */
	WLF_FORMAT_XRGB8888 = WLF_FOURCC('X','R','2','4'),  /**< 32-bit XRGB, no alpha */
	WLF_FORMAT_ABGR8888 = WLF_FOURCC('A','B','2','4'),  /**< 32-bit ABGR, 8 bits per channel */
	WLF_FORMAT_XBGR8888 = WLF_FOURCC('X','B','2','4'),  /**< 32-bit XBGR, no alpha */

	WLF_FORMAT_RGBA8888 = WLF_FOURCC('R','A','2','4'),  /**< 32-bit RGBA, 8 bits per channel */
	WLF_FORMAT_RGBX8888 = WLF_FOURCC('R','X','2','4'),  /**< 32-bit RGBX, no alpha */
	WLF_FORMAT_BGRA8888 = WLF_FOURCC('B','A','2','4'),  /**< 32-bit BGRA, 8 bits per channel */
	WLF_FORMAT_BGRX8888 = WLF_FOURCC('B','X','2','4'),  /**< 32-bit BGRX, no alpha */

	/* 24-bit */
	WLF_FORMAT_RGB888 = WLF_FOURCC('R','G','2','4'),    /**< 24-bit RGB, 8 bits per channel */
	WLF_FORMAT_BGR888 = WLF_FOURCC('B','G','2','4'),    /**< 24-bit BGR, 8 bits per channel */

	/* 16-bit */
	WLF_FORMAT_RGB565 = WLF_FOURCC('R','G','1','6'),    /**< 16-bit RGB, 5-6-5 */
	WLF_FORMAT_BGR565 = WLF_FOURCC('B','G','1','6'),    /**< 16-bit BGR, 5-6-5 */

	WLF_FORMAT_ARGB1555 = WLF_FOURCC('A','R','1','5'),  /**< 16-bit ARGB, 1-5-5-5 */
	WLF_FORMAT_XRGB1555 = WLF_FOURCC('X','R','1','5'),  /**< 16-bit XRGB, no alpha, 1-5-5-5 */

	WLF_FORMAT_RGBA4444 = WLF_FOURCC('R','A','1','2'),  /**< 16-bit RGBA, 4 bits per channel */
	WLF_FORMAT_RGBX4444 = WLF_FOURCC('R','X','1','2'),  /**< 16-bit RGBX, no alpha, 4 bits per channel */

	/* 10-bit HDR */
	WLF_FORMAT_ARGB2101010 = WLF_FOURCC('A','R','3','0'),  /**< 32-bit ARGB, 2-10-10-10 HDR */
	WLF_FORMAT_XRGB2101010 = WLF_FOURCC('X','R','3','0'),  /**< 32-bit XRGB, 2-10-10-10 HDR, no alpha */

	WLF_FORMAT_ABGR2101010 = WLF_FOURCC('A','B','3','0'),  /**< 32-bit ABGR, 2-10-10-10 HDR */
	WLF_FORMAT_XBGR2101010 = WLF_FOURCC('X','B','3','0'),  /**< 32-bit XBGR, 2-10-10-10 HDR, no alpha */

	/* Floating point */
	WLF_FORMAT_R16F = WLF_FOURCC('R','1','6','F'),      /**< 16-bit float, single red channel */
	WLF_FORMAT_R32F = WLF_FOURCC('R','3','2','F'),      /**< 32-bit float, single red channel */

	WLF_FORMAT_RG88 = WLF_FOURCC('G','R','0','8'),      /**< 16-bit RG, 8 bits per channel */
	WLF_FORMAT_RG1616F = WLF_FOURCC('G','R','1','6'),   /**< 32-bit float RG, 16 bits per channel */
	WLF_FORMAT_RG3232F = WLF_FOURCC('G','R','3','2'),   /**< 64-bit float RG, 32 bits per channel */

	WLF_FORMAT_RGBA16161616F = WLF_FOURCC('A','B','4','H'),  /**< 64-bit float RGBA, 16 bits per channel */
	WLF_FORMAT_RGBA32323232F = WLF_FOURCC('A','B','4','F'),  /**< 128-bit float RGBA, 32 bits per channel */

	/* Single channel */
	WLF_FORMAT_R8 = WLF_FOURCC('R','0','0','8'),        /**< 8-bit single red channel */

	/* YUV packed */
	WLF_FORMAT_YVYU = WLF_FOURCC('Y','V','Y','U'),      /**< Packed YUV 4:2:2, YVYU order */
	WLF_FORMAT_VYUY = WLF_FOURCC('V','Y','U','Y'),      /**< Packed YUV 4:2:2, VYUY order */

	/* YUV planar */
	WLF_FORMAT_NV12 = WLF_FOURCC('N','V','1','2'),      /**< Semi-planar YUV 4:2:0, Y + interleaved UV */
	WLF_FORMAT_P010 = WLF_FOURCC('P','0','1','0'),      /**< Semi-planar YUV 4:2:0, 10-bit */

	WLF_FORMAT_YUV420 = WLF_FOURCC('Y','U','1','2'),    /**< Planar YUV 4:2:0, Y + U + V */
	WLF_FORMAT_YVU420 = WLF_FOURCC('Y','V','1','2'),    /**< Planar YUV 4:2:0, Y + V + U */
};

/**
 * @brief Describes the memory layout of a pixel format.
 */
struct wlf_pixel_format_info {
	uint32_t format;             /**< Pixel format identifier (enum wlf_pixel_format) */

	uint32_t opaque_substitute;  /**< Opaque variant of this format (0 if none or already opaque) */

	uint32_t bytes_per_block;    /**< Number of bytes per pixel block */

	uint32_t block_width;        /**< Number of pixels per block horizontally (1 for non-subsampled) */
	uint32_t block_height;       /**< Number of pixels per block vertically (1 for non-subsampled) */
};

/**
 * @brief Looks up format information by format identifier.
 * @param format Pixel format identifier (enum wlf_pixel_format).
 * @return Pointer to the format info, or NULL if not found.
 */
const struct wlf_pixel_format_info *wlf_get_pixel_format_info(uint32_t format);

/**
 * @brief Returns the number of pixels per block for the given format.
 * @param info Pixel format info.
 * @return Number of pixels per block (at least 1).
 */
uint32_t pixel_format_info_pixels_per_block(const struct wlf_pixel_format_info *info);

/**
 * @brief Calculates the minimum row stride in bytes for the given format and width.
 * @param info Pixel format info.
 * @param width Image width in pixels.
 * @return Minimum stride in bytes, or 0 on overflow.
 */
int32_t pixel_format_info_min_stride(const struct wlf_pixel_format_info *info, int32_t width);

/**
 * @brief Checks whether a stride value is valid for the given format and width.
 * @param info Pixel format info.
 * @param stride Row stride in bytes to validate.
 * @param width Image width in pixels.
 * @return true if the stride is valid, false otherwise.
 */
bool pixel_format_info_check_stride(const struct wlf_pixel_format_info *info,
	int32_t stride, int32_t width);

#if WLF_HAS_LINUX_PLATFORM
/**
 * @brief Converts a Wayland shared memory format to a wlf pixel format.
 *
 * WL_SHM_FORMAT_ARGB8888 and WL_SHM_FORMAT_XRGB8888 are special cases
 * with values 0 and 1 that do not follow the fourcc convention.
 * All other wl_shm_format values share the same numeric value as
 * their corresponding wlf format.
 *
 * @param fmt Wayland shm format.
 * @return Corresponding wlf pixel format, or WLF_FORMAT_INVALID if unknown.
 */
uint32_t convert_wl_shm_format_to_wlf(enum wl_shm_format fmt);

/**
 * @brief Converts a wlf pixel format to a Wayland shared memory format.
 *
 * @param fmt wlf pixel format.
 * @return Corresponding wl_shm_format value.
 */
enum wl_shm_format convert_wlf_format_to_wl_shm(uint32_t fmt);
#endif

/**
 * @brief Returns whether the given pixel format has an alpha channel.
 * @param fmt Pixel format identifier.
 * @return true if the format has alpha, false if it is fully opaque.
 */
bool pixel_format_has_alpha(uint32_t fmt);

/**
 * @brief Returns whether the given pixel format is a YCbCr (YUV) format.
 * @param fmt Pixel format identifier.
 * @return true if the format is YCbCr, false otherwise.
 */
bool pixel_format_is_ycbcr(uint32_t fmt);

#endif // TYPES_WLF_PIXEL_FORMAT_H
