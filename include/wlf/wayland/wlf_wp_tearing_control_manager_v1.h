/**
 * @file        wlf_wp_tearing_control_manager_v1.h
 * @brief       Wayland wp_tearing_control_manager_v1 protocol wrapper for
 *              wlframe.
 * @details     Implements the staging tearing-control-v1 protocol, which
 *              allows clients to hint to the compositor whether a surface
 *              prefers tearing (async presentation) or vsync (smooth
 *              presentation).
 *
 *              Usage:
 *                1. Bind wlf_wp_tearing_control_manager_v1 from the registry.
 *                2. For each surface, call
 *                   wlf_wp_tearing_control_manager_v1_get_tearing_control()
 *                   to obtain a per-surface wlf_wp_tearing_control_v1.
 *                3. Call wlf_wp_tearing_control_v1_set_presentation_hint()
 *                   before each wl_surface.commit (double-buffered state).
 *                4. Destroy the per-surface object when done.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WP_TEARING_CONTROL_MANAGER_V1_H
#define WAYLAND_WLF_WP_TEARING_CONTROL_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_surface;
struct wp_tearing_control_manager_v1;
struct wp_tearing_control_v1;

/**
 * @brief Presentation hint values mirroring wp_tearing_control_v1.presentation_hint.
 */
enum wlf_tearing_presentation_hint {
	WLF_TEARING_VSYNC  = 0, /**< Prefer vsync (smooth) presentation     */
	WLF_TEARING_ASYNC  = 1, /**< Prefer async (tearing) presentation    */
};

/**
 * @brief Wrapper around a bound wp_tearing_control_manager_v1 global.
 */
struct wlf_wp_tearing_control_manager_v1 {
	struct wp_tearing_control_manager_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Per-surface tearing hint controller.
 *
 * Created by wlf_wp_tearing_control_manager_v1_get_tearing_control().
 * The caller owns and must destroy this object.
 */
struct wlf_wp_tearing_control_v1 {
	struct wp_tearing_control_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the wp_tearing_control_manager_v1 global from the registry.
 */
struct wlf_wp_tearing_control_manager_v1 *
wlf_wp_tearing_control_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 */
void wlf_wp_tearing_control_manager_v1_destroy(
	struct wlf_wp_tearing_control_manager_v1 *manager);

/**
 * @brief Create a per-surface tearing control object.
 *
 * @param manager  Bound manager.
 * @param surface  wl_surface to associate with.
 * @return A new wlf_wp_tearing_control_v1, or NULL on failure.
 */
struct wlf_wp_tearing_control_v1 *
wlf_wp_tearing_control_manager_v1_get_tearing_control(
	struct wlf_wp_tearing_control_manager_v1 *manager,
	struct wl_surface *surface);

/**
 * @brief Set the presentation hint for a surface.
 *
 * Must be called before wl_surface.commit to take effect.
 *
 * @param control  Per-surface tearing control object.
 * @param hint     Desired presentation hint.
 */
void wlf_wp_tearing_control_v1_set_presentation_hint(
	struct wlf_wp_tearing_control_v1 *control,
	enum wlf_tearing_presentation_hint hint);

/**
 * @brief Destroy the per-surface tearing control object.
 */
void wlf_wp_tearing_control_v1_destroy(struct wlf_wp_tearing_control_v1 *control);

#endif /* WAYLAND_WLF_WP_TEARING_CONTROL_MANAGER_V1_H */
