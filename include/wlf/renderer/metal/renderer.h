/**
 * @file        renderer.h
 * @brief       Metal renderer backend implementation for wlframe.
 * @details     This file defines the Metal-based renderer implementation for wlframe.
 *              It extends the generic `wlf_renderer` interface to provide hardware-accelerated
 *              rendering through the Metal API on macOS. This backend manages Metal devices,
 *              command queues, and integrates with wlframe's renderer abstraction.
 *
 * @author      YaoBing Xiao
 * @date        2026-02-04
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2026-02-04, initial version.
 */

#ifndef METAL_MTL_RENDERER_H
#define METAL_MTL_RENDERER_H

#include "wlf/renderer/wlf_renderer.h"
#include "wlf/utils/wlf_log.h"

#include <stdbool.h>

struct wlf_mtl_device;
struct wlf_backend;

/**
 * @struct wlf_mtl_renderer
 * @brief Metal renderer implementation for wlframe.
 *
 * This structure represents a Metal-based renderer, derived from `wlf_renderer`.
 * It encapsulates Metal resources, device handles, command queues, and synchronization
 * objects required for GPU rendering on macOS.
 */
struct wlf_mtl_renderer {
	struct wlf_renderer base;        /**< Base renderer interface (must be first for inheritance). */
	struct wlf_backend *backend;     /**< Associated wlframe backend. */
	struct wlf_mtl_device *dev;      /**< Metal device wrapper. */

	void *command_queue;             /**< Metal command queue (MTLCommandQueue). */
};

/**
 * @brief Creates a Metal renderer from a wlframe backend.
 *
 * This function initializes the Metal renderer by selecting a suitable device
 * and allocating necessary GPU resources.
 *
 * @param backend Pointer to the wlframe backend.
 * @return Pointer to the created `wlf_renderer`, or NULL on failure.
 */
struct wlf_renderer *wlf_mtl_renderer_create_from_backend(
	struct wlf_backend *backend);

/**
 * @brief Destroys a Metal renderer instance.
 *
 * Releases Metal resources including command queues and device handles.
 * Should be called when the Metal renderer is no longer needed.
 *
 * @param mtl_render Pointer to the Metal renderer instance.
 */
void wlf_mtl_renderer_destroy(struct wlf_mtl_renderer *mtl_render);

/**
 * @brief Checks whether a renderer is a Metal renderer.
 *
 * @param wlf_renderer Pointer to a generic renderer instance.
 * @return true if the renderer uses Metal as backend, false otherwise.
 */
bool wlf_renderer_is_mtl(struct wlf_renderer *wlf_renderer);

/**
 * @brief Converts a generic renderer pointer to a Metal renderer pointer.
 *
 * This function safely casts a `wlf_renderer` to its derived `wlf_mtl_renderer`
 * type if the renderer is Metal-based.
 *
 * @param wlf_renderer Pointer to a generic renderer.
 * @return Pointer to the Metal renderer, or NULL if not Metal-based.
 */
struct wlf_mtl_renderer *wlf_mtl_renderer_from_render(struct wlf_renderer *wlf_renderer);

/**
 * @brief Creates a Metal renderer for a specific device.
 *
 * Internal function used to create a renderer with an existing Metal device.
 *
 * @param device Pointer to the Metal device wrapper.
 * @return Pointer to the created `wlf_renderer`, or NULL on failure.
 */
struct wlf_renderer *wlf_mtl_renderer_create_for_device(struct wlf_mtl_device *device);

#endif // METAL_MTL_RENDERER_H
