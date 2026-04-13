/**
 * @file        render_target_info.h
 * @brief       Pixman render target info definitions in wlframe.
 * @details     This file provides the pixman-specific render target info type and
 *              helper functions used by pixman render passes.
 * @author      YaoBing Xiao
 * @date        2026-04-13
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-13, initial version\n
 */

#ifndef PIXMAN_RENDER_TARGET_INFO_H
#define PIXMAN_RENDER_TARGET_INFO_H

#include "wlf/pass/wlf_render_target_info.h"
#include "wlf/buffer/pixman/buffer.h"

#include <stdbool.h>

/**
 * @brief Pixman-specific render target info object.
 */
struct wlf_pixman_render_target_info {
	struct wlf_render_target_info base; /**< Base render target info */
	struct wlf_pixman_buffer *buffer;   /**< Target pixman buffer */
};

/**
 * @brief Begins a pixman render pass for the given buffer.
 *
 * @param buffer Pixman buffer used as render target.
 * @return Pixman render target info for the started pass, or NULL on failure.
 */
struct wlf_pixman_render_target_info *wlf_pixman_begin_pixman_render_pass(
	struct wlf_pixman_buffer *buffer);

/**
 * @brief Begins data pointer access for a pixman-compatible buffer.
 *
 * @param buffer Buffer to access.
 * @param image_ptr Output image pointer for pixman operations.
 * @param flags Access flags.
 * @return true on success, false otherwise.
 */
bool begin_pixman_data_ptr_access(struct wlf_buffer *buffer, pixman_image_t **image_ptr,
	uint32_t flags);

/**
 * @brief Checks whether a render target info is a pixman implementation.
 *
 * @param render_target Render target info to test.
 * @return true if render target is pixman-based, false otherwise.
 */
bool wlf_render_target_info_is_pixman(const struct wlf_render_target_info *render_target);

/**
 * @brief Casts base render target info to pixman render target info.
 *
 * @param render_target Base render target info object.
 * @return Pixman render target info on success, or NULL if type does not match.
 */
struct wlf_pixman_render_target_info *wlf_pixman_render_target_info_from_info(
	struct wlf_render_target_info *render_target);

#endif // PIXMAN_RENDER_TARGET_INFO_H
