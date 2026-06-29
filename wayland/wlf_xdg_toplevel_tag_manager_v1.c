/**
 * @file        wlf_xdg_toplevel_tag_manager_v1.c
 * @brief       Wayland xdg_toplevel_tag_manager_v1 protocol wrapper for
 *              wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_xdg_toplevel_tag_manager_v1.h"
#include "wayland/protocols/xdg-toplevel-tag-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct wlf_xdg_toplevel_tag_manager_v1 *
wlf_xdg_toplevel_tag_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver =
		(uint32_t)xdg_toplevel_tag_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_xdg_toplevel_tag_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_xdg_toplevel_tag_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&xdg_toplevel_tag_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for xdg_toplevel_tag_manager_v1 "
			"(name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound xdg_toplevel_tag_manager_v1 (name: %u, version: %u)",
		name, bind_ver);

	return manager;
}

void wlf_xdg_toplevel_tag_manager_v1_destroy(
	struct wlf_xdg_toplevel_tag_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	xdg_toplevel_tag_manager_v1_destroy(manager->base);
	free(manager);
}

void wlf_xdg_toplevel_tag_manager_v1_set_toplevel_tag(
	struct wlf_xdg_toplevel_tag_manager_v1 *manager,
	struct xdg_toplevel *toplevel, const char *tag)
{
	assert(manager);
	assert(manager->base);
	assert(toplevel);
	xdg_toplevel_tag_manager_v1_set_toplevel_tag(manager->base,
		toplevel, tag);
}

void wlf_xdg_toplevel_tag_manager_v1_set_toplevel_description(
	struct wlf_xdg_toplevel_tag_manager_v1 *manager,
	struct xdg_toplevel *toplevel, const char *description)
{
	assert(manager);
	assert(manager->base);
	assert(toplevel);
	xdg_toplevel_tag_manager_v1_set_toplevel_description(manager->base,
		toplevel, description);
}
