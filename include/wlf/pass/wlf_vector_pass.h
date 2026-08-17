/**
 * @file        wlf_vector_pass.h
 * @brief       Renderer-independent vector pass interface.
 * @details     A vector pass submits covered triangles for solid-color and
 *              shape rendering backends.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef PASS_WLF_VECTOR_PASS_H
#define PASS_WLF_VECTOR_PASS_H

#include "wlf/pass/wlf_pass.h"
#include "wlf/pass/wlf_render_target_info.h"
#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_signal.h"

#include <pixman.h>
#include <stddef.h>

/**
 * @brief One covered vertex in a vector-pass triangle list.
 *
 * Coverage is multiplied into the source color by the pass implementation.
 */
struct wlf_vector_vertex {
	float x, y; /**< Vertex position in logical coordinates. */
	float coverage; /**< Coverage multiplier in the range 0..1. */
};

/**
 * @brief Options for drawing a list of independent triangles.
 *
 * Every group of three vertices forms one independent triangle.
 */
struct wlf_vector_options {
	const struct wlf_vector_vertex *vertices; /**< Interleaved triangle vertices. */
	size_t vertex_count; /**< Must be a multiple of three. */
	struct wlf_color color; /**< Source color before coverage and opacity. */
	const pixman_region32_t *clip; /**< Optional logical clip region. */
	enum wlf_render_blend_mode blend_mode; /**< Compositing mode. */
};

/**
 * @brief Renderer-independent vector pass object.
 *
 * Backend-specific passes embed this base structure.
 */
struct wlf_vector_pass;

/**
 * @brief Virtual methods implemented by a vector pass backend.
 *
 * Backend implementations provide resource destruction and triangle drawing
 * through this table.
 */
struct wlf_vector_pass_impl {
	/**
	 * @brief Releases backend resources owned by @p pass.
	 * @param pass Vector pass to destroy.
	 */
	void (*destroy)(struct wlf_vector_pass *pass);
	/**
	 * @brief Renders one triangle list to @p render_target_info.
	 * @param pass Vector pass performing the draw.
	 * @param render_target_info Destination render target.
	 * @param options Triangle and compositing options.
	 */
	void (*render)(struct wlf_vector_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_vector_options *options);
};

/**
 * @brief Base object shared by all vector pass implementations.
 *
 * The implementation owns backend-specific state that embeds this object.
 */
struct wlf_vector_pass {
	const struct wlf_vector_pass_impl *impl; /**< Backend virtual methods. */
	struct {
		struct wlf_signal destroy; /**< Emitted before the pass is destroyed. */
	} events;
};

/**
 * @brief Automatically creates a vector rendering pass.
 *
 * Selects the pass implementation that matches the renderer backend.
 *
 * @param renderer Renderer used to select the pass backend.
 * @return A newly created vector pass, or NULL if the backend is unsupported
 *         or pass creation fails.
 */
struct wlf_vector_pass *wlf_vector_pass_auto_create(struct wlf_renderer *renderer);

/**
 * @brief Initializes a vector pass with backend virtual methods.
 * @param pass Vector pass to initialize.
 * @param impl Backend virtual-method table.
 */
void wlf_vector_pass_init(struct wlf_vector_pass *pass,
	const struct wlf_vector_pass_impl *impl);

/**
 * @brief Destroys a vector pass and emits its destroy signal.
 * @param pass Vector pass to destroy.
 */
void wlf_vector_pass_destroy(struct wlf_vector_pass *pass);

/**
 * @brief Submits a list of independent triangles to the pass.
 * @param pass Vector pass receiving the draw operation.
 * @param render_target_info Destination render target.
 * @param options Triangle and compositing options.
 */
void wlf_render_pass_add_triangles(struct wlf_vector_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const struct wlf_vector_options *options);

#endif // PASS_WLF_VECTOR_PASS_H
