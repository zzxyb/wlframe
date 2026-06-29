/**
 * @file        wlf_xdg_system_bell_v1.h
 * @brief       Wayland xdg_system_bell_v1 protocol wrapper for wlframe.
 * @details     Implements the staging xdg-system-bell-v1 protocol, which
 *              allows clients to trigger a system bell notification, optionally
 *              associated with a specific surface.
 *
 *              Usage:
 *                1. Bind wlf_xdg_system_bell_v1 from the registry.
 *                2. Call wlf_xdg_system_bell_v1_ring() with an optional
 *                   surface to trigger the bell.
 *                3. Destroy when done.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_XDG_SYSTEM_BELL_V1_H
#define WAYLAND_WLF_XDG_SYSTEM_BELL_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_surface;
struct xdg_system_bell_v1;

/**
 * @brief Wrapper around a bound xdg_system_bell_v1 global.
 */
struct wlf_xdg_system_bell_v1 {
	struct xdg_system_bell_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the xdg_system_bell_v1 global from the registry.
 */
struct wlf_xdg_system_bell_v1 *wlf_xdg_system_bell_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the bell object and free its resources.
 */
void wlf_xdg_system_bell_v1_destroy(struct wlf_xdg_system_bell_v1 *bell);

/**
 * @brief Ring the system bell.
 *
 * @param bell     Bound bell object.
 * @param surface  Optional surface to associate with the bell, or NULL.
 */
void wlf_xdg_system_bell_v1_ring(struct wlf_xdg_system_bell_v1 *bell,
	struct wl_surface *surface);

#endif /* WAYLAND_WLF_XDG_SYSTEM_BELL_V1_H */
