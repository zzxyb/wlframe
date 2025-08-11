/**
 * @file        wlf_renderer.h
 * @brief       Renderer interface and backend abstraction for wlframe.
 * @details     This file defines the renderer abstraction layer, allowing wlframe
 *              to support multiple rendering backends (such as GPU and CPU).
 *              It includes the renderer type enumeration, implementation interface,
 *              and functions for renderer creation and destruction.
 *
 * @author      YaoBing Xiao
 * @date        2025-11-03
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2025-11-03, initial version.
 */

#ifndef RENDERER_WLF_RENDERER_H
#define RENDERER_WLF_RENDERER_H

#include "wlf/utils/wlf_signal.h"

struct wlf_renderer;
struct wlf_backend;

/**
 * @enum wlf_renderer_type
 * @brief Renderer backend types.
 *
 * This enumeration specifies the type of renderer backend in use.
 * It helps wlframe choose appropriate rendering methods.
 */
enum wlf_renderer_type {
	GPU, /**< GPU-based renderer, typically using Vulkan or OpenGL. */
	CPU, /**< CPU-based renderer, typically using software rasterization (e.g., Pixman). */
};

/**
 * @struct wlf_renderer_impl
 * @brief Renderer implementation interface.
 *
 * This structure defines function pointers used by specific renderer implementations.
 * Custom renderers (e.g., VulkanRenderer, PixmanRenderer) must implement these callbacks.
 */
struct wlf_renderer_impl {
	void (*destroy)(struct wlf_renderer *render);
};

/**
 * @struct wlf_renderer
 * @brief Core renderer object.
 *
 * This structure represents an initialized renderer with backend information,
 * event signals, and optional implementation data.
 */
struct wlf_renderer {
	const struct wlf_renderer_impl *impl;

	struct {
		struct wlf_signal destroy;  /**< Signal emitted when the renderer is destroyed. */
	} events;

	void *data;                    /**< Backend-specific user data (opaque pointer). */

	enum wlf_renderer_type type;   /**< Type of renderer backend. */
};

/**
 * @brief Destroys a renderer instance.
 *
 * This function cleans up all resources associated with a renderer and emits
 * the `destroy` signal before final deallocation.
 *
 * @param render Pointer to the renderer to destroy.
 */
struct wlf_renderer *wlf_renderer_autocreate(struct wlf_backend *backend);

/**
 * @brief Destroy a render
 * @param render Pointer to the render to destroy
 */
void wlf_renderer_destroy(struct wlf_renderer *render);

#endif // RENDERER_WLF_RENDERER_H
