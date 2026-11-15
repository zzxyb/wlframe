/**
 * @file        render_target_info.h
 * @brief       GLES render target info definitions.
 * @details     This file defines the GLES-specific render-target information
 *              used while presenting through an EGL swapchain.
 * @author      YaoBing Xiao
 * @date        2026-08-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-09, initial version\n
 */

#ifndef PASS_GLES_RENDER_TARGET_INFO_H
#define PASS_GLES_RENDER_TARGET_INFO_H

#include "wlf/pass/wlf_render_target_info.h"
#include "wlf/buffer/wlf_buffer.h"

struct wlf_gles_renderer;

/**
 * @brief GLES render target backed by an EGL window surface.
 */
struct wlf_gles_render_target_info {
	struct wlf_render_target_info base; /**< Generic wlf_render_target_info interface. */
	struct wlf_buffer *buffer; /**< Buffer used as the render target. */
	struct wlf_gles_renderer *renderer; /**< Renderer used by this object. */
};

/**
 * @brief Begins rendering to an EGL-backed buffer.
 *
 * @param buffer Buffer to inspect or render into.
 * @param renderer GLES renderer whose EGL context owns the buffer.
 * @return New GLES render target, or NULL for an unsupported buffer or EGL failure.
 */
struct wlf_gles_render_target_info *wlf_gles_begin_egl_render_pass(
	struct wlf_buffer *buffer, struct wlf_gles_renderer *renderer);

/**
 * @brief Checks whether a render target is GLES-backed.
 *
 * @param render_target Render target to inspect or convert.
 * @return true for a GLES render target, false otherwise.
 */
bool wlf_render_target_info_is_gles(const struct wlf_render_target_info *render_target);

/**
 * @brief Casts a render target to GLES target info.
 *
 * @param render_target Render target to inspect or convert.
 * @return Enclosing GLES render target; asserts on an incompatible type.
 */
struct wlf_gles_render_target_info *wlf_gles_render_target_info_from_info(
	struct wlf_render_target_info *render_target);

#endif // PASS_GLES_RENDER_TARGET_INFO_H
