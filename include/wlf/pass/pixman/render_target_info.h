/**
 * @file        render_target_info.h
 * @brief       Pixman render target info for wlframe render passes.
 * @details     This file defines the pixman-specific render target info type
 *              used while a pixman render pass is active. It extends
 *              @ref wlf_render_target_info and carries the bound
 *              @ref wlf_pixman_render_buffer.
 * @author      YaoBing Xiao
 * @date        2026-03-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-20, initial version\n
 */

#ifndef PIXMAN_RENDER_TARGET_INFO_H
#define PIXMAN_RENDER_TARGET_INFO_H

#include "wlf/pass/wlf_render_target_info.h"
#include "wlf/buffer/pixman/render_buffer.h"

/**
 * @brief Pixman-specific render target info.
 *
 * This structure is created when beginning a pixman render pass and destroyed
 * when the pass ends. It embeds the generic base object and keeps a reference
 * to the render buffer being targeted.
 */
struct wlf_pixman_render_target_info {
	struct wlf_render_target_info base; /**< Base render target info object. */
	struct wlf_pixman_render_buffer *buffer; /**< Target pixman render buffer. */
};

/**
 * @brief Begins a pixman render pass for the given render buffer.
 *
 * Allocates and initializes a @ref wlf_pixman_render_target_info object,
 * prepares data access for the underlying buffer, and returns pass-local
 * state used by pixman rendering code.
 *
 * @param buffer Target pixman render buffer.
 * @return New pixman render target info, or NULL on failure.
 */
struct wlf_pixman_render_target_info *begin_pixman_render_pass(
	struct wlf_pixman_render_buffer *buffer);

/**
 * @brief Checks whether a render target info object is pixman-based.
 *
 * @param info Render target info object to check.
 * @return true if @p info is a pixman render target info, false otherwise.
 */
bool wlf_pixman_render_target_info_is_pixman(const struct wlf_render_target_info *info);

/**
 * @brief Converts a base render target info pointer to pixman type.
 *
 * The caller must ensure @p base is pixman-based (for example by calling
 * @ref wlf_pixman_render_target_info_is_pixman first).
 *
 * @param base Base render target info pointer.
 * @return Pixman render target info pointer.
 */
struct wlf_pixman_render_target_info *wlf_pixman_render_target_info_from_render_target_info(
	struct wlf_render_target_info *base);

#endif // PIXMAN_RENDER_TARGET_INFO_H
