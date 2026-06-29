/**
 * @file        wlf_ellipse_pass.h
 * @brief       Ellipse-shape rendering pass interface.
 * @details     Defines the renderer-independent options and lifecycle for
 *              submitting ellipse geometry.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef PASS_WLF_ELLIPSE_PASS_H
#define PASS_WLF_ELLIPSE_PASS_H

#include "wlf/pass/wlf_vector_pass.h"
#include "wlf/shapes/wlf_ellipse_shape.h"

struct wlf_ellipse_pass;

/**
 * @brief Rendering options for one ellipse shape.
 *
 * The shape is submitted with the specified translation, clipping, opacity,
 * and blend mode.
 */
struct wlf_render_ellipse_options {
	const struct wlf_ellipse_shape *shape; /**< Shape geometry to render. */
	double offset_x, offset_y; /**< Translation in logical coordinates. */
	float opacity; /**< Opacity in the inclusive range 0..1. */
	const pixman_region32_t *clip; /**< Optional logical clip region. */
	enum wlf_render_blend_mode blend_mode; /**< Compositing mode. */
};

/**
 * @brief Creates an ellipse pass backed by a vector pass.
 *
 * The vector pass remains owned by the caller and must outlive the ellipse
 * pass.
 *
 * @param vector_pass Vector pass used to submit generated geometry.
 * @return Newly allocated ellipse pass, or NULL on failure.
 */
struct wlf_ellipse_pass *wlf_ellipse_pass_create(
	struct wlf_vector_pass *vector_pass);

/**
 * @brief Destroys an ellipse pass and releases its renderer resources.
 * @param pass Ellipse pass to destroy.
 */
void wlf_render_ellipse_pass_destroy(struct wlf_ellipse_pass *pass);

/**
 * @brief Adds one ellipse draw operation to the render target.
 * @param pass Ellipse pass receiving the draw operation.
 * @param target Render target to draw into.
 * @param options Shape and compositing options.
 */
void wlf_render_pass_add_ellipse(struct wlf_ellipse_pass *pass,
	struct wlf_render_target_info *target,
	const struct wlf_render_ellipse_options *options);

#endif
