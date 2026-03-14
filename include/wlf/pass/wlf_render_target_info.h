/**
 * @file        wlf_render_target_info.h
 * @brief       Render target info base object for wlframe render passes.
 * @details     This file defines a small polymorphic base type used to carry
 *              backend-specific render-target metadata across render-pass code.
 *              Backends can attach custom payload in @ref wlf_render_target_info.data
 *              and provide a custom destroy callback via
 *              @ref wlf_render_target_info_impl.
 * @author      YaoBing Xiao
 * @date        2026-03-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-20, initial version\n
 */

#ifndef PASS_WLF_RENDER_TARGET_INFO_H
#define PASS_WLF_RENDER_TARGET_INFO_H

#include "wlf/utils/wlf_signal.h"

struct wlf_render_target_info;

/**
 * @brief Virtual methods for render target info objects.
 */
struct wlf_render_target_info_impl {
	/**
	 * @brief Destroys a render target info object.
	 *
	 * Implementations are responsible for releasing backend-specific resources
	 * and freeing the enclosing allocation when needed.
	 *
	 * @param info Render target info object to destroy.
	 */
	void (*destroy)(struct wlf_render_target_info *info);
};

/**
 * @brief Base object describing an active render target.
 *
 * This structure is backend-agnostic and can be embedded by concrete backend
 * types (for example, pixman or vulkan). Users can subscribe to
 * @ref events.destroy to perform cleanup before backend resources are released.
 */
struct wlf_render_target_info {
	const struct wlf_render_target_info_impl *impl; /**< Virtual method table. */

	void *data; /**< Backend-specific user data (opaque pointer). */

	struct {
		struct wlf_signal destroy; /**< Emitted before the object is destroyed. */
	} events;
};

/**
 * @brief Initializes a render target info object.
 *
 * Initializes the object fields and its destroy signal. The object must later
 * be released with @ref wlf_render_target_info_destroy.
 *
 * @param info Render target info object to initialize.
 * @param impl Implementation callbacks for the object.
 */
void wlf_render_target_info_init(struct wlf_render_target_info *info,
	const struct wlf_render_target_info_impl *impl);

/**
 * @brief Destroys a render target info object.
 *
 * Emits the destroy signal and calls the implementation destroy callback when
 * provided.
 *
 * @param info Render target info object to destroy. NULL is allowed.
 */
void wlf_render_target_info_destroy(struct wlf_render_target_info *info);

#endif // PASS_WLF_RENDER_TARGET_INFO_H
