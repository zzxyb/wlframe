/**
 * @file        wlf_wp_viewporter.h
 * @brief       Wayland wp_viewporter protocol wrapper for wlframe.
 * @details     Implements the stable wp_viewporter protocol, which allows a
 *              client to crop and/or scale the contents of a wl_surface
 *              independently of the buffer size.
 *
 *              Usage:
 *                1. Bind wlf_wp_viewporter from the registry.
 *                2. For each surface, call
 *                   wlf_wp_viewporter_get_viewport() to obtain a per-surface
 *                   wlf_wp_viewport.
 *                3. Before each wl_surface.commit, call:
 *                     - wlf_wp_viewport_set_source() to define the source crop
 *                       rectangle in buffer coordinates (wl_fixed_t).
 *                     - wlf_wp_viewport_set_destination() to define the output
 *                       size in surface-local coordinates.
 *                   Both are double-buffered state.
 *                4. Destroy the viewport before or together with the surface.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_WP_VIEWPORTER_H
#define WAYLAND_WLF_WP_VIEWPORTER_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_surface;
struct wp_viewporter;
struct wp_viewport;

/**
 * @brief Wrapper around a bound wp_viewporter global.
 *
 * Obtain via wlf_wp_viewporter_create().
 */
struct wlf_wp_viewporter {
	struct wp_viewporter *base; /**< Underlying protocol object */
	uint32_t version;

	struct {
		struct wlf_signal destroy; /**< Emitted just before object is freed */
	} events;
};

/**
 * @brief Per-surface crop/scale object.
 *
 * Created by wlf_wp_viewporter_get_viewport().  The caller owns this object.
 * Destroying the viewport removes the crop/scale state from the surface at
 * the next commit.
 *
 * The wp_viewport protocol object has no events; this wrapper only provides
 * request helpers and a destroy signal.
 */
struct wlf_wp_viewport {
	struct wp_viewport *base; /**< Underlying protocol object */
	uint32_t version;

	struct {
		struct wlf_signal destroy; /**< Emitted just before object is freed */
	} events;
};

/**
 * @brief Bind to the wp_viewporter global from the registry.
 *
 * @param wl_registry  Wayland registry to bind from.
 * @param name         Global name (ID) from the registry event.
 * @param version      Advertised version; clamped to the client maximum.
 * @return Newly allocated viewporter, or NULL on failure.
 */
struct wlf_wp_viewporter *wlf_wp_viewporter_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the viewporter and free its resources.
 *
 * @param viewporter  Viewporter to destroy; silently ignored when NULL.
 */
void wlf_wp_viewporter_destroy(struct wlf_wp_viewporter *viewporter);

/**
 * @brief Create a per-surface viewport.
 *
 * @param viewporter  Bound viewporter.
 * @param surface     wl_surface to associate with.
 * @return Newly allocated viewport, or NULL on failure.
 */
struct wlf_wp_viewport *wlf_wp_viewporter_get_viewport(
	struct wlf_wp_viewporter *viewporter, struct wl_surface *surface);

/**
 * @brief Set the source crop rectangle (double-buffered).
 *
 * All coordinates are in post-transform, post-scale buffer space.
 * Pass -1.0 for all four arguments to unset the source rectangle.
 *
 * @param viewport  Per-surface viewport.
 * @param x         Left edge of the crop rectangle.
 * @param y         Top edge of the crop rectangle.
 * @param width     Width of the crop rectangle.
 * @param height    Height of the crop rectangle.
 */
void wlf_wp_viewport_set_source(struct wlf_wp_viewport *viewport,
	double x, double y, double width, double height);

/**
 * @brief Set the destination output size (double-buffered).
 *
 * Pass -1 for both width and height to unset the destination.
 *
 * @param viewport  Per-surface viewport.
 * @param width     Destination width in surface-local pixels.
 * @param height    Destination height in surface-local pixels.
 */
void wlf_wp_viewport_set_destination(struct wlf_wp_viewport *viewport,
	int32_t width, int32_t height);

/**
 * @brief Destroy the per-surface viewport.
 *
 * @param viewport  Viewport to destroy; silently ignored when NULL.
 */
void wlf_wp_viewport_destroy(struct wlf_wp_viewport *viewport);

#endif /* WAYLAND_WLF_WP_VIEWPORTER_H */
