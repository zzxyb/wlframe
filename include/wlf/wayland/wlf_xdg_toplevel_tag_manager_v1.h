/**
 * @file        wlf_xdg_toplevel_tag_manager_v1.h
 * @brief       Wayland xdg_toplevel_tag_manager_v1 protocol wrapper for
 *              wlframe.
 * @details     Implements the staging xdg-toplevel-tag-v1 protocol, which
 *              allows clients to attach a tag and human-readable description
 *              to an xdg_toplevel.  The compositor may use this information
 *              for window management, scripting, or accessibility purposes.
 *
 *              Usage:
 *                1. Bind wlf_xdg_toplevel_tag_manager_v1 from the registry.
 *                2. Call wlf_xdg_toplevel_tag_manager_v1_set_toplevel_tag()
 *                   with a stable identifier string.
 *                3. Optionally call
 *                   wlf_xdg_toplevel_tag_manager_v1_set_toplevel_description()
 *                   with a localised, human-readable description.
 *                4. Destroy when done.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_XDG_TOPLEVEL_TAG_MANAGER_V1_H
#define WAYLAND_WLF_XDG_TOPLEVEL_TAG_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct xdg_toplevel;
struct xdg_toplevel_tag_manager_v1;

/**
 * @brief Wrapper around a bound xdg_toplevel_tag_manager_v1 global.
 */
struct wlf_xdg_toplevel_tag_manager_v1 {
	struct xdg_toplevel_tag_manager_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the xdg_toplevel_tag_manager_v1 global from the registry.
 */
struct wlf_xdg_toplevel_tag_manager_v1 *
wlf_xdg_toplevel_tag_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 */
void wlf_xdg_toplevel_tag_manager_v1_destroy(
	struct wlf_xdg_toplevel_tag_manager_v1 *manager);

/**
 * @brief Set a stable tag identifier for a toplevel.
 *
 * @param manager   Bound manager.
 * @param toplevel  The xdg_toplevel to tag.
 * @param tag       A stable, application-defined identifier string.
 */
void wlf_xdg_toplevel_tag_manager_v1_set_toplevel_tag(
	struct wlf_xdg_toplevel_tag_manager_v1 *manager,
	struct xdg_toplevel *toplevel, const char *tag);

/**
 * @brief Set a human-readable description for a toplevel.
 *
 * @param manager      Bound manager.
 * @param toplevel     The xdg_toplevel to describe.
 * @param description  A localised description string.
 */
void wlf_xdg_toplevel_tag_manager_v1_set_toplevel_description(
	struct wlf_xdg_toplevel_tag_manager_v1 *manager,
	struct xdg_toplevel *toplevel, const char *description);

#endif /* WAYLAND_WLF_XDG_TOPLEVEL_TAG_MANAGER_V1_H */
