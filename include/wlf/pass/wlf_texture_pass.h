
/**
 * @file        wlf_texture_pass.h
 * @brief       Texture rendering pass interface in wlframe.
 * @details     This file defines the options, structures, and functions required
 *              to perform texture drawing operations within a rendering pass.
 * @author      YaoBing Xiao
 * @date        2026-06-02
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-02, initial version\n
 */

#ifndef PASS_WLF_RECT_PASS_H
#define PASS_WLF_RECT_PASS_H

#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_region.h"
#include "wlf/pass/wlf_pass.h"
#include "wlf/math/wlf_frect.h"
#include "wlf/types/wlf_output.h"
#include "wlf/pass/wlf_render_target_info.h"

struct wlf_texture;

/**
 * @brief Configuration options for rendering a texture.
 */
struct wlf_render_texture_options {
	struct wlf_texture *texture;           /**< Source texture to render */
	struct wlf_frect src_box;              /**< Source region sampled from the texture */
	struct wlf_frect dst_box;              /**< Destination region on the render target */
	const float *alpha;                    /**< Optional alpha multiplier, leave NULL to use full opacity */
	const struct wlf_region *clip;         /**< Clip region, leave NULL to disable clipping */
	enum wlf_output_transform transform;   /**< Texture transform applied during rendering */
	enum wlf_scale_filter_mode filter_mode; /**< Sampling filter used when scaling the texture */
	enum wlf_render_blend_mode blend_mode; /**< Texture blend mode */
};

struct wlf_render_texture_pass;

/**
 * @brief Virtual method table for texture rendering pass implementations.
 */
struct wlf_render_texture_pass_impl {
	/**
	 * @brief Destroys texture pass specific resources.
	 *
	 * @param pass Texture pass instance.
	 */
	void (*destroy)(struct wlf_render_texture_pass *pass);

	/**
	 * @brief Renders a texture onto the specified render target.
	 *
	 * @param pass Texture pass instance.
	 * @param render_target_info Target surface metadata and context.
	 * @param options Configurations including source, destination, and clipping.
	 */
	void (*render)(struct wlf_render_texture_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_texture_options *options);
};

/**
 * @brief Base texture rendering pass.
 */
struct wlf_render_texture_pass {
	const struct wlf_render_texture_pass_impl *impl; /**< Implementation vtable */
	struct {
		struct wlf_signal destroy;         /**< Emitted before the texture pass is destroyed */
	} events;
};

/**
 * @brief Initializes a texture rendering pass object.
 *
 * @param pass Texture pass object to initialize.
 * @param impl Implementation vtable.
 */
void wlf_render_texture_pass_init(struct wlf_render_texture_pass *pass,
	const struct wlf_render_texture_pass_impl *impl);

/**
 * @brief Destroys a texture rendering pass object.
 *
 * Calls implementation-specific destroy callback and emits destroy signal.
 *
 * @param pass Texture pass object to destroy.
 */
void wlf_render_texture_pass_destroy(struct wlf_render_texture_pass *pass);

/**
 * @brief Executes a texture draw operation within the pass.
 *
 * @param pass Texture pass instance.
 * @param render_target_info Target surface metadata and context.
 * @param options Configurations including source, destination, and clipping.
 */
void wlf_render_texture_pass_add_texture(struct wlf_render_texture_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const struct wlf_render_texture_options *options);

/**
 * @brief Gets the effective source box used for texture rendering.
 *
 * @param options Texture rendering options.
 * @param box Output source rectangle.
 */
void wlf_render_texture_options_get_src_box(const struct wlf_render_texture_options *options,
	struct wlf_frect *box);

/**
 * @brief Gets the effective destination box used for texture rendering.
 *
 * @param options Texture rendering options.
 * @param box Output destination rectangle.
 */
void wlf_render_texture_options_get_dst_box(const struct wlf_render_texture_options *options,
	struct wlf_frect *box);

/**
 * @brief Gets the effective alpha value used for texture rendering.
 *
 * @param options Texture rendering options.
 * @return Effective alpha multiplier.
 */
float wlf_render_texture_options_get_alpha(const struct wlf_render_texture_options *options);

#endif // PASS_WLF_RECT_PASS_H
