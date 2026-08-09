/**
 * @file        wlf_xdg_toplevel_icon_manager_v1.c
 * @brief       Wayland xdg_toplevel_icon_manager_v1 protocol wrapper for
 *              wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_xdg_toplevel_icon_manager_v1.h"
#include "wayland/protocols/xdg-toplevel-icon-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Manager listeners
 * ---------------------------------------------------------------------- */

static void manager_handle_icon_size(void *data,
	struct xdg_toplevel_icon_manager_v1 *base, int32_t size)
{
	struct wlf_xdg_toplevel_icon_manager_v1 *manager = data;
	(void)base;

	int32_t *new_sizes = realloc(manager->preferred_sizes,
		(manager->n_preferred_sizes + 1) * sizeof(int32_t));
	if (!new_sizes) {
		wlf_log_errno(WLF_ERROR,
			"failed to grow preferred_sizes for "
			"xdg_toplevel_icon_manager_v1");
		return;
	}

	manager->preferred_sizes = new_sizes;
	manager->preferred_sizes[manager->n_preferred_sizes] = size;
	manager->n_preferred_sizes++;
}

static void manager_handle_done(void *data,
	struct xdg_toplevel_icon_manager_v1 *base)
{
	struct wlf_xdg_toplevel_icon_manager_v1 *manager = data;
	(void)base;
	wlf_signal_emit_mutable(&manager->events.done, manager);
}

static const struct xdg_toplevel_icon_manager_v1_listener manager_listener = {
	.icon_size = manager_handle_icon_size,
	.done      = manager_handle_done,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_xdg_toplevel_icon_manager_v1 *
wlf_xdg_toplevel_icon_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver =
		(uint32_t)xdg_toplevel_icon_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_xdg_toplevel_icon_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_xdg_toplevel_icon_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&xdg_toplevel_icon_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for xdg_toplevel_icon_manager_v1 "
			"(name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.done);
	wlf_signal_init(&manager->events.destroy);

	xdg_toplevel_icon_manager_v1_add_listener(manager->base,
		&manager_listener, manager);

	wlf_log(WLF_DEBUG,
		"bound xdg_toplevel_icon_manager_v1 (name: %u, version: %u)",
		name, bind_ver);

	return manager;
}

void wlf_xdg_toplevel_icon_manager_v1_destroy(
	struct wlf_xdg_toplevel_icon_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	xdg_toplevel_icon_manager_v1_destroy(manager->base);
	free(manager->preferred_sizes);
	free(manager);
}

struct wlf_xdg_toplevel_icon_v1 *
wlf_xdg_toplevel_icon_manager_v1_create_icon(
	struct wlf_xdg_toplevel_icon_manager_v1 *manager)
{
	assert(manager);
	assert(manager->base);

	struct wlf_xdg_toplevel_icon_v1 *icon = calloc(1, sizeof(*icon));
	if (!icon) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_xdg_toplevel_icon_v1");
		return NULL;
	}

	icon->base = xdg_toplevel_icon_manager_v1_create_icon(manager->base);
	if (!icon->base) {
		wlf_log(WLF_ERROR,
			"xdg_toplevel_icon_manager_v1_create_icon() returned NULL");
		free(icon);
		return NULL;
	}

	wlf_signal_init(&icon->events.destroy);
	return icon;
}

void wlf_xdg_toplevel_icon_manager_v1_set_icon(
	struct wlf_xdg_toplevel_icon_manager_v1 *manager,
	struct xdg_toplevel *toplevel,
	struct wlf_xdg_toplevel_icon_v1 *icon)
{
	assert(manager);
	assert(manager->base);
	assert(toplevel);

	struct xdg_toplevel_icon_v1 *raw_icon = icon ? icon->base : NULL;
	xdg_toplevel_icon_manager_v1_set_icon(manager->base, toplevel, raw_icon);
}

void wlf_xdg_toplevel_icon_v1_set_name(
	struct wlf_xdg_toplevel_icon_v1 *icon, const char *icon_name)
{
	assert(icon);
	assert(icon->base);
	xdg_toplevel_icon_v1_set_name(icon->base, icon_name);
}

void wlf_xdg_toplevel_icon_v1_add_buffer(
	struct wlf_xdg_toplevel_icon_v1 *icon,
	struct wl_buffer *buffer, int32_t scale)
{
	assert(icon);
	assert(icon->base);
	assert(buffer);
	xdg_toplevel_icon_v1_add_buffer(icon->base, buffer, scale);
}

void wlf_xdg_toplevel_icon_v1_destroy(struct wlf_xdg_toplevel_icon_v1 *icon)
{
	if (!icon) {
		return;
	}

	wlf_signal_emit_mutable(&icon->events.destroy, icon);
	xdg_toplevel_icon_v1_destroy(icon->base);
	free(icon);
}
