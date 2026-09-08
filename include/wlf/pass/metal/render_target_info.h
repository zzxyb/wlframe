/**
 * @file render_target_info.h
 * @brief Metal render-target state.
 */

#ifndef WLF_PASS_METAL_RENDER_TARGET_INFO_H
#define WLF_PASS_METAL_RENDER_TARGET_INFO_H

#include "wlf/pass/wlf_render_target_info.h"

#include <stdbool.h>

struct wlf_mtl_buffer;
struct wlf_mtl_renderer;

struct wlf_mtl_render_target_info {
	struct wlf_render_target_info base;
	struct wlf_mtl_buffer *buffer;
	struct wlf_mtl_renderer *renderer;
	void *command_buffer; /**< Retained id<MTLCommandBuffer>. */
	void *encoder; /**< Retained id<MTLRenderCommandEncoder>. */
};

struct wlf_mtl_render_target_info *wlf_mtl_begin_buffer_render_pass(
	struct wlf_mtl_buffer *buffer, struct wlf_mtl_renderer *renderer);

bool wlf_render_target_info_is_mtl(
	const struct wlf_render_target_info *render_target);

struct wlf_mtl_render_target_info *wlf_mtl_render_target_info_from_info(
	struct wlf_render_target_info *render_target);

void *wlf_mtl_render_target_get_encoder(
	const struct wlf_mtl_render_target_info *target);

#endif /* WLF_PASS_METAL_RENDER_TARGET_INFO_H */
