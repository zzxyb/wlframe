/**
 * @file        wlf_pass.h
 * @brief       Common rendering pass enums and definitions in wlframe.
 * @details     This file contains global enumerations used across different rendering 
 *              passes, specifically defining blending types and texture scaling filters.
 * @author      YaoBing Xiao
 * @date        2026-05-16
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-16, initial version\n
 */

#ifndef PASS_WLF_PASS_H
#define PASS_WLF_PASS_H

/**
 * @brief Supported color blending modes for rendering operations.
 */
enum wlf_render_blend_mode {
	WLF_RENDER_BLEND_MODE_PREMULTIPLIED, /**< Pre-multiplied alpha blending (default) */
	WLF_RENDER_BLEND_MODE_NONE,          /**< Blending is disabled, source overwrites destination */
};

/**
 * @brief Texture sampling and scaling filter modes.
 */
enum wlf_scale_filter_mode {
	WLF_SCALE_FILTER_BILINEAR,           /**< Bilinear texture filtering for smooth scaling (default) */
	WLF_SCALE_FILTER_NEAREST,            /**< Nearest-neighbor texture filtering for pixelated scaling */
};

#endif // PASS_WLF_PASS_H
