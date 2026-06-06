/**
 * @file        wlf_wl_subcompositor.h
 * @brief       Wayland wl_subcompositor global wrapper for wlframe.
 * @details     Wraps wl_subcompositor bound from the Wayland registry,
 *              providing subsurface creation.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_SUBCOMPOSITOR_H
#define WAYLAND_WLF_WL_SUBCOMPOSITOR_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_subcompositor;
struct wlf_wl_subsurface;
struct wlf_wl_surface;

/**
 * @brief Wayland subcompositor global.
 *
 * Wraps a wl_subcompositor object bound from the registry. The wrapper stores
 * the bound protocol version and provides event signalling for lifecycle
 * notifications.
 */
struct wlf_wl_subcompositor {
	struct wl_subcompositor *wl_subcompositor; /**< Underlying Wayland subcompositor object. */
	uint32_t version;                          /**< Bound wl_subcompositor protocol version. */

	struct {
		struct wlf_signal destroy; /**< Emitted before destruction. Payload: wlf_wl_subcompositor. */
	} events;
};

/**
 * @brief Binds wl_subcompositor from the Wayland registry.
 *
 * The requested version is clamped to the maximum version supported by the
 * local Wayland protocol headers before binding.
 *
 * @param wl_registry Wayland registry to bind from.
 * @param name Global name of wl_subcompositor in the registry.
 * @param version Advertised wl_subcompositor protocol version.
 * @return Newly allocated wlf_wl_subcompositor, or NULL on failure.
 */
struct wlf_wl_subcompositor *wlf_wl_subcompositor_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroys a wlf_wl_subcompositor.
 *
 * Emits the destroy signal, destroys the underlying wl_subcompositor, and
 * frees the wrapper.
 *
 * @param subcompositor The subcompositor to destroy. Passing NULL is a no-op.
 */
void wlf_wl_subcompositor_destroy(struct wlf_wl_subcompositor *subcompositor);

/**
 * @brief Creates a subsurface relationship between surface and parent.
 *
 * The returned subsurface represents surface as a child of parent. Its
 * lifecycle is independent from the subcompositor wrapper after creation.
 *
 * @param subcompositor Subcompositor to use.
 * @param surface Child surface that becomes the subsurface.
 * @param parent Parent surface that owns the subsurface relationship.
 * @return Newly allocated wlf_wl_subsurface, or NULL on failure.
 */
struct wlf_wl_subsurface *wlf_wl_subcompositor_get_subsurface(
	struct wlf_wl_subcompositor *subcompositor,
	struct wlf_wl_surface *surface,
	struct wlf_wl_surface *parent);

#endif /* WAYLAND_WLF_WL_SUBCOMPOSITOR_H */
