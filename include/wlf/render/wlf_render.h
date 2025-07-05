#ifndef RENDER_WLF_RENDER_H
#define RENDER_WLF_RENDER_H

#include "wlf/utils/wlf_signal.h"

struct wlf_render;
struct wlf_backend;

/**
 * @brief Renderer backend types
 */
enum wlf_render_type {
	WLF_RENDER_PIXMAN = 0,    /**< CPU software rendering with pixman */
	WLF_RENDER_GLES,          /**< OpenGL ES hardware rendering */
	WLF_RENDER_VULKAN,        /**< Vulkan hardware rendering */
};

/**
 * @brief Renderer implementation interface
 */
struct wlf_render_impl {
	enum wlf_render_type type;        /**< Renderer type */

	void (*destroy)(struct wlf_render *render);
};

struct wlf_render {
	const struct wlf_render_impl *impl;

	struct {
		struct wlf_signal destroy;  /**< Signal emitted when the renderer is destroyed */
	} events;

	void *data;
};

/**
 * @brief Create a renderer with automatic backend detection
 * @param backend Pointer to the backend
 * @return Pointer to the created renderer, or NULL on failure
 */
struct wlf_render *wlf_render_autocreate(struct wlf_backend *backend);

/**
 * @brief Destroy a renderer
 * @param render Pointer to the renderer to destroy
 */
void wlf_render_destroy(struct wlf_render *render);

void wlf_render_init(struct wlf_render *render, const struct wlf_render_impl *impl);

#endif // RENDER_WLF_RENDER_H
