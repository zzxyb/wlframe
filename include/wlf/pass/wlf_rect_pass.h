/**
 * @file        wlf_render_rect_pass.h
 * @brief       Rectangle rendering pass interface in wlframe.
 * @details     This file defines the options, structures, and functions required
 *              to perform rectangle drawing operations within a rendering pass.
 * @author      YaoBing Xiao
 * @date        2026-05-16
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-16, initial version\n
 */

#ifndef PASS_WLF_RECT_PASS_H
#define PASS_WLF_RECT_PASS_H

#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_region.h"
#include "wlf/pass/wlf_pass.h"
#include "wlf/math/wlf_frect.h"
#include "wlf/pass/wlf_render_target_info.h"

/**
 * @brief Configuration options for rendering a rectangle.
 */
struct wlf_render_rect_options {
	struct wlf_frect box;                  /**< Rectangle coordinates and dimensions */
	struct wlf_color color;                /**< Source fill color */
	const struct wlf_region *clip;         /**< Clip region, leave NULL to disable clipping */
	enum wlf_render_blend_mode blend_mode; /**< Color blend mode */
};

struct wlf_render_rect_pass;

/**
 * @brief Virtual method table for rectangle rendering pass implementations.
 */
struct wlf_render_rect_pass_impl {
	/**
	 * @brief Destroys rectangle pass specific resources.
	 *
	 * @param pass Rectangle pass instance.
	 */
	void (*destroy)(struct wlf_render_rect_pass *pass);

	/**
	 * @brief Renders a rectangle onto the specified render target.
	 *
	 * @param pass Rectangle pass instance.
	 * @param render_target_info Target surface metadata and context.
	 * @param options Configurations including dimensions, color, and clipping.
	 */
	void (*render)(struct wlf_render_rect_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_rect_options *options);
};

/**
 * @brief Base rectangle rendering pass.
 */
struct wlf_render_rect_pass {
	const struct wlf_render_rect_pass_impl *impl; /**< Implementation vtable */
	struct {
		struct wlf_signal destroy;        /**< Emitted before the rectangle pass is destroyed */
	} events;
};

/**
 * @brief Initializes a rectangle rendering pass object.
 *
 * @param pass Rectangle pass object to initialize.
 * @param impl Implementation vtable.
 */
void wlf_render_rect_pass_init(struct wlf_render_rect_pass *pass,
	const struct wlf_render_rect_pass_impl *impl);

/**
 * @brief Destroys a rectangle rendering pass object.
 *
 * Calls implementation-specific destroy callback and emits destroy signal.
 *
 * @param pass Rectangle pass object to destroy.
 */
void wlf_render_rect_pass_destroy(struct wlf_render_rect_pass *pass);

/**
 * @brief Eexecutes a rectangle draw operation within the pass.
 *
 * @param pass Rectangle pass instance.
 * @param render_target_info Target surface metadata and context.
 * @param options Configurations including dimensions, color, and clipping.
 */
void wlf_render_pass_add_rect(struct wlf_render_rect_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const struct wlf_render_rect_options *options);

#endif // PASS_WLF_RECT_PASS_H
