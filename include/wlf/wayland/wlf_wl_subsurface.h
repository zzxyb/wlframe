/**
 * @file        wlf_wl_subsurface.h
 * @brief       Wayland wl_subsurface wrapper for wlframe.
 * @details     Wraps a wl_subsurface, exposing position, stacking, and
 *              synchronisation mode requests.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_SUBSURFACE_H
#define WAYLAND_WLF_WL_SUBSURFACE_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_subsurface;
struct wlf_wl_surface;

/**
 * @brief Wayland subsurface relationship.
 *
 * Wraps a wl_subsurface returned by wlf_wl_subcompositor_get_subsurface().
 * The wrapper exposes subsurface state requests and stores the protocol
 * version of the underlying Wayland object.
 */
struct wlf_wl_subsurface {
	struct wl_subsurface *wl_subsurface; /**< Underlying Wayland subsurface object. */
	uint32_t version;                    /**< Bound wl_subsurface protocol version. */

	struct {
		struct wlf_signal destroy; /**< Emitted before destruction. Payload: wlf_wl_subsurface. */
	} events;
};

/**
 * @brief Destroys a wlf_wl_subsurface.
 *
 * Emits the destroy signal, destroys the underlying wl_subsurface, and frees
 * the wrapper.
 *
 * @param subsurface The subsurface to destroy. Passing NULL is a no-op.
 */
void wlf_wl_subsurface_destroy(struct wlf_wl_subsurface *subsurface);

/**
 * @brief Sets the position of the subsurface relative to its parent.
 *
 * The position is relative to the parent surface's local coordinate space.
 * The request updates pending subsurface state and takes effect according to
 * the subsurface's synchronisation mode.
 *
 * @param subsurface The subsurface whose position is updated.
 * @param x Parent-local x coordinate of the subsurface.
 * @param y Parent-local y coordinate of the subsurface.
 */
void wlf_wl_subsurface_set_position(struct wlf_wl_subsurface *subsurface,
	int32_t x, int32_t y);

/**
 * @brief Places the subsurface above a sibling surface in the stacking order.
 *
 * @param subsurface The subsurface whose stacking order is updated.
 * @param sibling Sibling surface that the subsurface will be placed above.
 */
void wlf_wl_subsurface_place_above(struct wlf_wl_subsurface *subsurface,
	struct wlf_wl_surface *sibling);

/**
 * @brief Places the subsurface below a sibling surface in the stacking order.
 *
 * @param subsurface The subsurface whose stacking order is updated.
 * @param sibling Sibling surface that the subsurface will be placed below.
 */
void wlf_wl_subsurface_place_below(struct wlf_wl_subsurface *subsurface,
	struct wlf_wl_surface *sibling);

/**
 * @brief Enables synchronised commit mode (default).
 *
 * In synchronised mode, subsurface state is cached and applied when the parent
 * surface state is applied.
 *
 * @param subsurface The subsurface to put into synchronised mode.
 */
void wlf_wl_subsurface_set_sync(struct wlf_wl_subsurface *subsurface);

/**
 * @brief Enables desynchronised commit mode.
 *
 * In desynchronised mode, subsurface commits are applied independently from the
 * parent surface where the protocol permits it.
 *
 * @param subsurface The subsurface to put into desynchronised mode.
 */
void wlf_wl_subsurface_set_desync(struct wlf_wl_subsurface *subsurface);

#endif /* WAYLAND_WLF_WL_SUBSURFACE_H */
