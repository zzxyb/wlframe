/**
 * @file        wlf_rect_shape_pass.h
 * @brief       Rounded-rectangle rendering pass interface.
 * @details     Defines the renderer-independent options and lifecycle for
 *              submitting rectangle-shape geometry.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef PASS_WLF_RECT_SHAPE_PASS_H
#define PASS_WLF_RECT_SHAPE_PASS_H

#include "wlf/pass/wlf_vector_pass.h"
#include "wlf/shapes/wlf_rect_shape.h"

struct wlf_rect_shape_pass;

/**
 * @brief Rendering options for one rectangle shape.
 *
 * The shape is submitted with the specified translation, clipping, opacity,
 * and blend mode.
 */
struct wlf_render_rect_shape_options {
	const struct wlf_rect_shape *shape; /**< Shape geometry to render. */
	double offset_x, offset_y; /**< Translation in logical coordinates. */
	float opacity; /**< Opacity in the inclusive range 0..1. */
	const pixman_region32_t *clip; /**< Optional logical clip region. */
	enum wlf_render_blend_mode blend_mode; /**< Compositing mode. */
};

/**
 * @brief Creates a rectangle-shape pass backed by a vector pass.
 *
 * The vector pass remains owned by the caller and must outlive the
 * rectangle-shape pass.
 *
 * @param vector_pass Vector pass used to submit generated geometry.
 * @return Newly allocated rectangle-shape pass, or NULL on failure.
 */
struct wlf_rect_shape_pass *wlf_rect_shape_pass_create(
	struct wlf_vector_pass *vector_pass);

/**
 * @brief Destroys a rectangle-shape pass and releases its renderer resources.
 * @param pass Rectangle-shape pass to destroy.
 */
void wlf_render_rect_shape_pass_destroy(struct wlf_rect_shape_pass *pass);

/**
 * @brief Adds one rectangle-shape draw operation to the render target.
 * @param pass Rectangle-shape pass receiving the draw operation.
 * @param target Render target to draw into.
 * @param options Shape and compositing options.
 */
void wlf_render_pass_add_rect_shape(struct wlf_rect_shape_pass *pass,
	struct wlf_render_target_info *target,
	const struct wlf_render_rect_shape_options *options);

#endif
