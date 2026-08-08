/**
 * @file        wlf_poly_pass.h
 * @brief       Polygon-shape rendering pass interface.
 * @details     Defines the renderer-independent options and lifecycle for
 *              submitting polygon geometry.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef PASS_WLF_POLY_PASS_H
#define PASS_WLF_POLY_PASS_H

#include "wlf/pass/wlf_vector_pass.h"
#include "wlf/shapes/wlf_poly_shape.h"

struct wlf_poly_pass;

/**
 * @brief Rendering options for one polygon shape.
 *
 * The shape is submitted with the specified translation, clipping, opacity,
 * and blend mode.
 */
struct wlf_render_poly_options {
	const struct wlf_poly_shape *shape; /**< Shape geometry to render. */
	double offset_x, offset_y; /**< Translation in logical coordinates. */
	float opacity; /**< Opacity in the inclusive range 0..1. */
	const pixman_region32_t *clip; /**< Optional logical clip region. */
	enum wlf_render_blend_mode blend_mode; /**< Compositing mode. */
};

/**
 * @brief Creates a polygon pass backed by a vector pass.
 *
 * The vector pass remains owned by the caller and must outlive the polygon
 * pass.
 *
 * @param vector_pass Vector pass used to submit generated geometry.
 * @return Newly allocated polygon pass, or NULL on failure.
 */
struct wlf_poly_pass *wlf_poly_pass_create(struct wlf_vector_pass *vector_pass);

/**
 * @brief Destroys a polygon pass and releases its renderer resources.
 * @param pass Polygon pass to destroy.
 */
void wlf_render_poly_pass_destroy(struct wlf_poly_pass *pass);

/**
 * @brief Adds one polygon draw operation to the render target.
 * @param pass Polygon pass receiving the draw operation.
 * @param target Render target to draw into.
 * @param options Shape and compositing options.
 */
void wlf_render_pass_add_poly(struct wlf_poly_pass *pass,
	struct wlf_render_target_info *target,
	const struct wlf_render_poly_options *options);

#endif
