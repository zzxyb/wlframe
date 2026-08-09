/**
 * @file        wlf_xdg_system_bell_v1.c
 * @brief       Wayland xdg_system_bell_v1 protocol wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_xdg_system_bell_v1.h"
#include "wayland/protocols/xdg-system-bell-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct wlf_xdg_system_bell_v1 *wlf_xdg_system_bell_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver = (uint32_t)xdg_system_bell_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_xdg_system_bell_v1 *bell = calloc(1, sizeof(*bell));
	if (!bell) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_xdg_system_bell_v1");
		return NULL;
	}

	bell->base = wl_registry_bind(wl_registry, name,
		&xdg_system_bell_v1_interface, bind_ver);
	if (!bell->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for xdg_system_bell_v1 (name: %u)",
			name);
		free(bell);
		return NULL;
	}

	wlf_signal_init(&bell->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound xdg_system_bell_v1 (name: %u, version: %u)",
		name, bind_ver);

	return bell;
}

void wlf_xdg_system_bell_v1_destroy(struct wlf_xdg_system_bell_v1 *bell)
{
	if (!bell) {
		return;
	}

	wlf_signal_emit_mutable(&bell->events.destroy, bell);
	xdg_system_bell_v1_destroy(bell->base);
	free(bell);
}

void wlf_xdg_system_bell_v1_ring(struct wlf_xdg_system_bell_v1 *bell,
	struct wl_surface *surface)
{
	assert(bell);
	assert(bell->base);
	xdg_system_bell_v1_ring(bell->base, surface);
}
