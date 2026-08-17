/**
 * @file        wlf_texture_pass.h
 * @brief       Renderer-independent texture pass interface.
 * @details     A texture pass translates a source texture and its sampling
 *              options into draw operations on a render target.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef PASS_WLF_TEXTURE_PASS_H
#define PASS_WLF_TEXTURE_PASS_H

#include "wlf/math/wlf_frect.h"
#include "wlf/pass/wlf_pass.h"
#include "wlf/pass/wlf_render_target_info.h"
#include "wlf/texture/wlf_texture.h"
#include "wlf/utils/wlf_signal.h"

#include <pixman.h>

/**
 * @brief Rendering options for one textured rectangle.
 *
 * The source rectangle is sampled from the texture and mapped to the
 * destination rectangle on the render target.
 */
struct wlf_render_texture_options {
	struct wlf_texture *texture; /**< Texture to sample. */
	struct wlf_frect src_box; /**< Source rectangle in texture coordinates. */
	struct wlf_frect dst_box; /**< Destination rectangle in logical coordinates. */
	float opacity; /**< Opacity in the inclusive range 0..1. */
	const pixman_region32_t *clip; /**< Optional logical clip region. */
	enum wlf_scale_filter_mode filter_mode; /**< Texture filtering mode. */
	enum wlf_render_blend_mode blend_mode; /**< Compositing mode. */
};

/**
 * @brief Renderer-independent texture pass object.
 *
 * Backend-specific passes embed this base structure.
 */
struct wlf_texture_pass;

/**
 * @brief Virtual methods implemented by a texture pass backend.
 *
 * Backend implementations provide resource destruction and textured drawing
 * through this table.
 */
struct wlf_texture_pass_impl {
	/**
	 * @brief Releases backend resources owned by @p pass.
	 * @param pass Texture pass to destroy.
	 */
	void (*destroy)(struct wlf_texture_pass *pass);
	/**
	 * @brief Renders one textured rectangle to @p render_target_info.
	 * @param pass Texture pass performing the draw.
	 * @param render_target_info Destination render target.
	 * @param options Texture and sampling options.
	 */
	void (*render)(struct wlf_texture_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_texture_options *options);
};

/**
 * @brief Base object shared by all texture pass implementations.
 *
 * The implementation owns backend-specific state that embeds this object.
 */
struct wlf_texture_pass {
	const struct wlf_texture_pass_impl *impl; /**< Backend virtual methods. */
	struct {
		struct wlf_signal destroy; /**< Emitted before the pass is destroyed. */
	} events;
};

/**
 * @brief Automatically creates a texture rendering pass.
 *
 * Selects the pass implementation that matches the renderer backend.
 *
 * @param renderer Renderer used to select the pass backend.
 * @return A newly created texture pass, or NULL if the backend is unsupported
 *         or pass creation fails.
 */
struct wlf_texture_pass *wlf_texture_pass_auto_create(struct wlf_renderer *renderer);

/**
 * @brief Initializes a texture pass with backend virtual methods.
 * @param pass Texture pass to initialize.
 * @param impl Backend virtual-method table.
 */
void wlf_render_texture_pass_init(struct wlf_texture_pass *pass,
	const struct wlf_texture_pass_impl *impl);

/**
 * @brief Destroys a texture pass and emits its destroy signal.
 * @param pass Texture pass to destroy.
 */
void wlf_render_texture_pass_destroy(struct wlf_texture_pass *pass);

/**
 * @brief Submits one textured rectangle to the pass.
 * @param pass Texture pass receiving the draw operation.
 * @param render_target_info Destination render target.
 * @param options Texture and sampling options.
 */
void wlf_render_pass_add_texture(struct wlf_texture_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const struct wlf_render_texture_options *options);

/**
 * @brief Returns the effective source rectangle, applying texture defaults.
 * @param options Texture rendering options.
 * @param box Output source rectangle.
 */
void wlf_render_texture_options_get_src_box(
	const struct wlf_render_texture_options *options,
	struct wlf_frect *box);
/**
 * @brief Returns the effective destination rectangle, applying texture defaults.
 * @param options Texture rendering options.
 * @param box Output destination rectangle.
 */
void wlf_render_texture_options_get_dst_box(
	const struct wlf_render_texture_options *options,
	struct wlf_frect *box);

#endif // PASS_WLF_TEXTURE_PASS_H
