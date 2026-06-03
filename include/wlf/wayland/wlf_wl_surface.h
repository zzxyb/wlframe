/**
 * @file        wlf_wl_surface.h
 * @brief       Wayland backend implementation for wlf_wl_surface.
 * @details     This file provides the Wayland-specific wrapper for wl_surface
 *              handling in wlframe. It exposes wl_surface state requests through
 *              wlf_wl_surface helpers and forwards compositor events through
 *              wlf_signal.
 *
 *              Surface state requests update pending Wayland state and take
 *              effect when wlf_wl_surface_commit() is called. Requests that
 *              require newer wl_surface protocol versions are ignored when the
 *              bound wl_surface version does not support them.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_SURFACE_H
#define WAYLAND_WLF_WL_SURFACE_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_region.h"

#include <stdint.h>

struct wl_buffer;
struct wl_callback;
struct wl_output;
struct wl_region;
struct wl_surface;
struct wlf_wl_compositor;
struct wlf_wl_region;

/**
 * @brief Payload for wlf_wl_surface enter/leave events.
 */
struct wlf_wl_surface_output_event {
	struct wlf_wl_surface *surface; /**< Surface that entered or left the output. */
	struct wl_output *output;       /**< Wayland output involved in the event. */
};

/**
 * @brief Wayland-backed surface wrapper.
 *
 * Wraps a wl_surface created by a wlf_wl_compositor. The wrapper stores the
 * wl_surface protocol version so helpers can avoid sending unsupported
 * requests on older compositors.
 */
struct wlf_wl_surface {
	struct wl_surface *wl_surface;        /**< Underlying Wayland surface object. */
	struct wl_compositor *wl_compositor;  /**< Compositor used to create regions. */
	int32_t preferred_buffer_scale;       /**< Current preferred buffer scale factor. */
	uint32_t preferred_buffer_transform;  /**< Current preferred buffer transform. */
	uint32_t version;                     /**< Bound wl_surface protocol version. */

	struct {
		struct wlf_signal destroy;                    /**< Emitted before destruction. Payload: wlf_wl_surface. */
		struct wlf_signal enter;                      /**< Emitted when the surface enters an output. Payload: wlf_wl_surface_output_event. */
		struct wlf_signal leave;                      /**< Emitted when the surface leaves an output. Payload: wlf_wl_surface_output_event. */
		struct wlf_signal preferred_buffer_scale;     /**< Emitted when preferred_buffer_scale changes. Payload: wlf_wl_surface. */
		struct wlf_signal preferred_buffer_transform; /**< Emitted when preferred_buffer_transform changes. Payload: wlf_wl_surface. */
	} events;
};

/**
 * @brief Creates a wlf_wl_surface from a Wayland compositor.
 * @param compositor The Wayland compositor wrapper used to create the surface.
 * @return Pointer to the newly created wlf_wl_surface, or NULL on failure.
 */
struct wlf_wl_surface *wlf_wl_surface_create(struct wlf_wl_compositor *compositor);

/**
 * @brief Destroys a wlf_wl_surface.
 * @param surface The surface to destroy. Passing NULL is a no-op.
 */
void wlf_wl_surface_destroy(struct wlf_wl_surface *surface);

/**
 * @brief Attaches a buffer to the surface's pending state.
 * @param surface The target surface.
 * @param buffer The buffer to attach, or NULL to detach the current buffer.
 * @param x Surface-local x offset for the attached buffer.
 * @param y Surface-local y offset for the attached buffer.
 */
void wlf_wl_surface_attach(struct wlf_wl_surface *surface,
	struct wl_buffer *buffer, int32_t x, int32_t y);

/**
 * @brief Marks a surface-local rectangle as damaged.
 * @param surface The surface whose pending state is updated.
 * @param x Surface-local x coordinate of the damaged rectangle.
 * @param y Surface-local y coordinate of the damaged rectangle.
 * @param width Width of the damaged rectangle.
 * @param height Height of the damaged rectangle.
 */
void wlf_wl_surface_damage(struct wlf_wl_surface *surface,
	int32_t x, int32_t y, int32_t width, int32_t height);

/**
 * @brief Marks a buffer-local rectangle as damaged.
 * @param surface The surface whose pending state is updated.
 * @param x Buffer-local x coordinate of the damaged rectangle.
 * @param y Buffer-local y coordinate of the damaged rectangle.
 * @param width Width of the damaged rectangle.
 * @param height Height of the damaged rectangle.
 *
 * No request is sent when the bound wl_surface version does not support
 * wl_surface.damage_buffer.
 */
void wlf_wl_surface_damage_buffer(struct wlf_wl_surface *surface,
	int32_t x, int32_t y, int32_t width, int32_t height);

/**
 * @brief Requests a frame callback for animation pacing.
 * @param surface The surface for which to request the callback.
 * @return Raw wl_callback that will be called before the next repaint, or NULL on failure.
 */
struct wl_callback *wlf_wl_surface_frame(struct wlf_wl_surface *surface);

/**
 * @brief Sets the opaque region of the surface.
 * @param surface The surface whose pending opaque region is updated.
 * @param region The opaque region, or NULL/an empty region to clear it.
 */
void wlf_wl_surface_set_opaque_region(struct wlf_wl_surface *surface,
	struct wlf_region *region);

/**
 * @brief Sets the input region of the surface.
 * @param surface The surface whose pending input region is updated.
 * @param region The input region, or NULL/an empty region to clear it.
 */
void wlf_wl_surface_set_input_region(struct wlf_wl_surface *surface,
	struct wlf_region *region);

/**
 * @brief Sets the buffer transform.
 * @param surface The surface whose pending buffer transform is updated.
 * @param transform The wl_output_transform value to apply to attached buffers.
 *
 * No request is sent when the bound wl_surface version does not support
 * wl_surface.set_buffer_transform.
 */
void wlf_wl_surface_set_buffer_transform(struct wlf_wl_surface *surface,
	int32_t transform);

/**
 * @brief Sets the buffer scale.
 * @param surface The surface whose pending buffer scale is updated.
 * @param scale The scale factor to apply to attached buffers.
 *
 * No request is sent when the bound wl_surface version does not support
 * wl_surface.set_buffer_scale.
 */
void wlf_wl_surface_set_buffer_scale(struct wlf_wl_surface *surface,
	int32_t scale);

/**
 * @brief Sets the buffer offset without attaching a new buffer.
 * @param surface The surface whose pending buffer offset is updated.
 * @param x Surface-local x offset for the currently attached buffer.
 * @param y Surface-local y offset for the currently attached buffer.
 *
 * No request is sent when the bound wl_surface version does not support
 * wl_surface.offset.
 */
void wlf_wl_surface_offset(struct wlf_wl_surface *surface,
	int32_t x, int32_t y);

/**
 * @brief Commits the pending surface state.
 * @param surface The surface to commit.
 */
void wlf_wl_surface_commit(struct wlf_wl_surface *surface);

#endif // WAYLAND_WLF_WL_SURFACE_H
