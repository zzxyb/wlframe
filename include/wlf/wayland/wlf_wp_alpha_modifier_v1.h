/**
 * @file        wlf_wp_alpha_modifier_v1.h
 * @brief       Wayland wp_alpha_modifier_v1 protocol wrapper for wlframe.
 * @details     Wraps the staging alpha-modifier-v1 protocol, which allows
 *              a client to apply an additional per-surface alpha multiplier on
 *              top of the per-pixel alpha stored in the buffer.
 *              The protocol represents opacity as an unsigned integer where
 *              0 is fully transparent and UINT32_MAX is fully opaque; this
 *              wrapper exposes it as a floating-point value in the [0, 1]
 *              range.
 *
 *              Usage:
 *                1. Bind wlf_wp_alpha_modifier_v1 from the registry.
 *                2. Call wlf_wp_alpha_modifier_v1_get_surface() for each
 *                   surface that needs an alpha override.
 *                3. Call wlf_wp_alpha_modifier_surface_v1_set_multiplier()
 *                   before wl_surface.commit to update the double-buffered
 *                   surface state.
 *                4. Destroy the per-surface object before destroying its
 *                   wl_surface. Destroying resets the multiplier to opaque
 *                   with the same double-buffered semantics.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_WP_ALPHA_MODIFIER_V1_H
#define WAYLAND_WLF_WP_ALPHA_MODIFIER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_surface;
struct wp_alpha_modifier_v1;
struct wp_alpha_modifier_surface_v1;

/**
 * @brief Wrapper around a bound wp_alpha_modifier_v1 global.
 *
 * Obtain via wlf_wp_alpha_modifier_v1_create(). Emits a destroy signal before
 * teardown.
 */
struct wlf_wp_alpha_modifier_v1 {
	struct wp_alpha_modifier_v1 *base; /**< Underlying protocol object. */
	uint32_t version;

	struct {
		struct wlf_signal destroy; /**< Emitted just before object is freed. */
	} events;
};

/**
 * @brief Per-surface alpha multiplier controller.
 *
 * Created by wlf_wp_alpha_modifier_v1_get_surface(). The caller owns this
 * object and must call wlf_wp_alpha_modifier_surface_v1_destroy() before the
 * associated wl_surface is destroyed. The multiplier is double-buffered
 * surface state and takes effect on wl_surface.commit.
 */
struct wlf_wp_alpha_modifier_surface_v1 {
	struct wp_alpha_modifier_surface_v1 *base; /**< Underlying protocol object. */
	uint32_t version;
	uint32_t multiplier; /**< Cached protocol multiplier value. */

	struct {
		struct wlf_signal destroy; /**< Emitted just before object is freed. */
	} events;
};

/**
 * @brief Bind to the wp_alpha_modifier_v1 global from the registry.
 *
 * @param wl_registry  Wayland registry to bind from.
 * @param name         Global name from the registry event.
 * @param version      Advertised version; clamped to the client maximum.
 * @return Newly allocated manager, or NULL on failure.
 */
struct wlf_wp_alpha_modifier_v1 *wlf_wp_alpha_modifier_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 *
 * Emits the destroy signal, sends the protocol destroy request and frees the
 * wrapper. Per-surface alpha modifier objects are unaffected and must be
 * destroyed separately.
 *
 * @param manager  Manager to destroy; silently ignored when NULL.
 */
void wlf_wp_alpha_modifier_v1_destroy(struct wlf_wp_alpha_modifier_v1 *manager);

/**
 * @brief Create a per-surface alpha modifier object.
 *
 * Associates @p surface with a new wp_alpha_modifier_surface_v1 object via the
 * manager. The compositor raises an already_constructed protocol error if an
 * alpha modifier object already exists for this surface.
 *
 * @param manager  Bound manager.
 * @param surface  wl_surface to associate with.
 * @return Newly allocated per-surface object, or NULL on failure.
 */
struct wlf_wp_alpha_modifier_surface_v1 *wlf_wp_alpha_modifier_v1_get_surface(
	struct wlf_wp_alpha_modifier_v1 *manager, struct wl_surface *surface);

/**
 * @brief Set the alpha multiplier for the surface (double-buffered).
 *
 * Takes effect on the next wl_surface.commit. Values outside [0, 1] are
 * clamped by the implementation before conversion to the protocol's uint32
 * multiplier.
 *
 * @param surface  Per-surface alpha modifier object.
 * @param alpha    Alpha multiplier: 0.0 is transparent, 1.0 is opaque.
 */
void wlf_wp_alpha_modifier_surface_v1_set_multiplier(
	struct wlf_wp_alpha_modifier_surface_v1 *surface, float alpha);

/**
 * @brief Destroy the per-surface alpha modifier object.
 *
 * Emits the destroy signal, sends the protocol destroy request (resetting the
 * surface multiplier to opaque as double-buffered state) and frees the wrapper.
 *
 * @param surface  Object to destroy; silently ignored when NULL.
 */
void wlf_wp_alpha_modifier_surface_v1_destroy(
	struct wlf_wp_alpha_modifier_surface_v1 *surface);

#endif /* WAYLAND_WLF_WP_ALPHA_MODIFIER_V1_H */
