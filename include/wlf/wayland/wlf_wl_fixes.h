/**
 * @file        wlf_wl_fixes.h
 * @brief       Wayland wl_fixes global wrapper for wlframe.
 * @details     Wraps wl_fixes bound from the Wayland registry, providing
 *              the destroy_registry request for proper wl_registry cleanup.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_FIXES_H
#define WAYLAND_WLF_WL_FIXES_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_fixes;
struct wl_registry;

/**
 * @brief Wayland fixes global.
 */
struct wlf_wl_fixes {
	struct wl_fixes *wl_fixes; /**< Underlying Wayland object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before destruction. */
	} events;
};

/**
 * @brief Binds wl_fixes from the Wayland registry.
 *
 * @param wl_registry Wayland registry.
 * @param name        Global name of wl_fixes.
 * @param version     Advertised version.
 * @return Newly allocated wlf_wl_fixes, or NULL on failure.
 */
struct wlf_wl_fixes *wlf_wl_fixes_create(struct wl_registry *wl_registry,
	uint32_t name, uint32_t version);

/**
 * @brief Destroys a wlf_wl_fixes. Passing NULL is a no-op.
 */
void wlf_wl_fixes_destroy(struct wlf_wl_fixes *fixes);

/**
 * @brief Destroys a wl_registry, allowing the compositor to reclaim its ID.
 *
 * @param fixes    Fixes global.
 * @param registry Registry to destroy.
 */
void wlf_wl_fixes_destroy_registry(struct wlf_wl_fixes *fixes,
	struct wl_registry *registry);

#endif /* WAYLAND_WLF_WL_FIXES_H */
