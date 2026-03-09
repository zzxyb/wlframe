/**
 * @file        renderer.h
 * @brief       Pixman software renderer backend implementation for wlframe.
 * @details     This file defines the Pixman-based CPU software renderer for wlframe.
 *              It extends the generic `wlf_renderer` interface to provide software
 *              rasterization through the Pixman library. This backend is suitable for
 *              platforms without GPU acceleration or when a lightweight fallback renderer
 *              is required.
 *
 * @author      YaoBing Xiao
 * @date        2026-03-08
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2026-03-08, initial version.
 */

#ifndef PIXMAN_RENDERER_H
#define PIXMAN_RENDERER_H

#include "wlf/renderer/wlf_renderer.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_backend;

/**
 * @struct wlf_pixman_renderer
 * @brief Pixman software renderer implementation for wlframe.
 *
 * This structure represents a Pixman-based CPU software renderer, derived from
 * `wlf_renderer`.
 */
struct wlf_pixman_renderer {
	struct wlf_renderer base;     /**< Base renderer interface. */
	struct wlf_backend *backend;  /**< Associated wlframe backend. */

	struct wlf_linked_list buffers; // wlf_pixman_render_buffer.link
	struct wlf_linked_list textures; // wlf_pixman_texture.link
};

/**
 * @brief Creates a Pixman renderer from a wlframe backend.
 *
 * Initializes the Pixman renderer and allocates a default rendering surface.
 *
 * @param backend Pointer to the wlframe backend.
 * @return Pointer to the created `wlf_renderer`, or NULL on failure.
 */
struct wlf_renderer *wlf_pixman_renderer_create_from_backend(
	struct wlf_backend *backend);

/**
 * @brief Checks whether a renderer is a Pixman renderer.
 *
 * @param renderer Pointer to a generic renderer instance.
 * @return true if the renderer uses Pixman as backend, false otherwise.
 */
bool wlf_renderer_is_pixman(const struct wlf_renderer *renderer);

/**
 * @brief Converts a generic renderer pointer to a Pixman renderer pointer.
 *
 * This function safely casts a `wlf_renderer` to its derived `wlf_pixman_renderer`
 * type if the renderer is Pixman-based.
 *
 * @param renderer Pointer to a generic renderer.
 * @return Pointer to the Pixman renderer, or NULL if not Pixman-based.
 */
struct wlf_pixman_renderer *wlf_pixman_renderer_from_renderer(
	struct wlf_renderer *renderer);

#endif /* PIXMAN_RENDERER_H */
