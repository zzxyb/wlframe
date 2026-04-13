/**
 * @file        wlf_render_target_info.h
 * @brief       Render target information interface in wlframe.
 * @details     This file defines the generic render target info abstraction used by
 *              rendering passes. Implementations provide lifecycle and renderer access.
 * @author      YaoBing Xiao
 * @date        2026-04-13
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-13, initial version\n
 */

#ifndef PASS_WLF_RENDER_TARGET_INFO_H
#define PASS_WLF_RENDER_TARGET_INFO_H

#include "wlf/renderer/wlf_renderer.h"
#include "wlf/utils/wlf_signal.h"

struct wlf_render_target_info;

/**
 * @brief Virtual method table for render target info implementations.
 */
struct wlf_render_target_info_impl {
	/**
	 * @brief Destroys render target specific resources.
	 *
	 * @param render_target Render target info instance.
	 */
	void (*destroy)(struct wlf_render_target_info *render_target);

	/**
	 * @brief Gets renderer associated with this render target.
	 *
	 * @param render_target Render target info instance.
	 * @return Renderer bound to this render target, or NULL when unavailable.
	 */
	struct wlf_renderer *(*get_renderer)(struct wlf_render_target_info *render_target);
};

/**
 * @brief Base render target info object.
 */
struct wlf_render_target_info {
	const struct wlf_render_target_info_impl *impl; /**< Implementation vtable */
	struct {
		struct wlf_signal destroy; /**< Emitted before target info is destroyed */
	} events;
};

/**
 * @brief Initializes a render target info object.
 *
 * @param render_target Render target info object to initialize.
 * @param impl Implementation vtable.
 */
void wlf_render_target_info_init(struct wlf_render_target_info *render_target,
	const struct wlf_render_target_info_impl *impl);

/**
 * @brief Destroys a render target info object.
 *
 * Calls implementation-specific destroy callback and emits destroy signal.
 *
 * @param render_target Render target info object to destroy.
 */
void wlf_render_target_info_destroy(struct wlf_render_target_info *render_target);

#endif // PASS_WLF_RENDER_TARGET_INFO_H
