/**
 * @file        wlf_wp_single_pixel_buffer_manager_v1.c
 * @brief       Wayland wp_single_pixel_buffer_manager_v1 protocol wrapper for
 *              wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#include "wlf/wayland/wlf_wp_single_pixel_buffer_manager_v1.h"
#include "wayland/protocols/single-pixel-buffer-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct wlf_wp_single_pixel_buffer_manager_v1 *
wlf_wp_single_pixel_buffer_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver = wp_single_pixel_buffer_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
		wlf_log(WLF_DEBUG,
			"wp_single_pixel_buffer_manager_v1: binding version %u "
			"(server advertised %u, interface supports %u)",
			bind_ver, version,
			wp_single_pixel_buffer_manager_v1_interface.version);
	}

	struct wlf_wp_single_pixel_buffer_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_wp_single_pixel_buffer_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&wp_single_pixel_buffer_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for wp_single_pixel_buffer_manager_v1");
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);
	return manager;
}

void wlf_wp_single_pixel_buffer_manager_v1_destroy(
	struct wlf_wp_single_pixel_buffer_manager_v1 *manager)
{
	assert(manager);
	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	wp_single_pixel_buffer_manager_v1_destroy(manager->base);
	free(manager);
}

struct wl_buffer *wlf_wp_single_pixel_buffer_manager_v1_create_u32_rgba_buffer(
	struct wlf_wp_single_pixel_buffer_manager_v1 *manager,
	uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
	assert(manager);
	return wp_single_pixel_buffer_manager_v1_create_u32_rgba_buffer(
		manager->base, r, g, b, a);
}
