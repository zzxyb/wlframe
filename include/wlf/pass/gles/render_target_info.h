/**
 * @file        render_target_info.h
 * @brief       GLES render target info definitions.
 */

#ifndef PASS_GLES_RENDER_TARGET_INFO_H
#define PASS_GLES_RENDER_TARGET_INFO_H

#include "wlf/pass/wlf_render_target_info.h"
#include "wlf/swapchain/egl/swapchain.h"

struct wlf_gles_renderer;

/**
 * @brief GLES render target backed by an EGL window surface.
 */
struct wlf_gles_render_target_info {
	struct wlf_render_target_info base;
	struct wlf_egl_swapchain *swapchain;
	struct wlf_gles_renderer *renderer;
};

/**
 * @brief Begins rendering to an EGL swapchain surface.
 */
struct wlf_gles_render_target_info *wlf_gles_begin_egl_render_pass(
	struct wlf_egl_swapchain *swapchain);

/**
 * @brief Checks whether a render target is GLES-backed.
 */
bool wlf_render_target_info_is_gles(const struct wlf_render_target_info *render_target);

/**
 * @brief Casts a render target to GLES target info.
 */
struct wlf_gles_render_target_info *wlf_gles_render_target_info_from_info(
	struct wlf_render_target_info *render_target);

#endif // PASS_GLES_RENDER_TARGET_INFO_H
