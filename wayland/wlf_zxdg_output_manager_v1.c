/**
 * @file        wlf_zxdg_output_manager_v1.c
 * @brief       Wayland xdg-output-unstable-v1 wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_zxdg_output_manager_v1.h"
#include "wayland/protocols/xdg-output-unstable-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

/* -------------------------------------------------------------------------
 * xdg_output_v1 listeners
 * ---------------------------------------------------------------------- */

static void xdg_output_handle_logical_position(void *data,
	struct zxdg_output_v1 *base, int32_t x, int32_t y)
{
	struct wlf_zxdg_output_v1 *output = data;
	(void)base;
	output->x = x;
	output->y = y;
}

static void xdg_output_handle_logical_size(void *data,
	struct zxdg_output_v1 *base, int32_t width, int32_t height)
{
	struct wlf_zxdg_output_v1 *output = data;
	(void)base;
	output->width  = width;
	output->height = height;
}

static void xdg_output_handle_done(void *data,
	struct zxdg_output_v1 *base)
{
	struct wlf_zxdg_output_v1 *output = data;
	(void)base;
	wlf_signal_emit_mutable(&output->events.done, output);
}

static void xdg_output_handle_name(void *data,
	struct zxdg_output_v1 *base, const char *name)
{
	struct wlf_zxdg_output_v1 *output = data;
	(void)base;
	free(output->name);
	output->name = name ? strdup(name) : NULL;
}

static void xdg_output_handle_description(void *data,
	struct zxdg_output_v1 *base, const char *description)
{
	struct wlf_zxdg_output_v1 *output = data;
	(void)base;
	free(output->description);
	output->description = description ? strdup(description) : NULL;
}

static const struct zxdg_output_v1_listener xdg_output_listener = {
	.logical_position = xdg_output_handle_logical_position,
	.logical_size     = xdg_output_handle_logical_size,
	.done             = xdg_output_handle_done,
	.name             = xdg_output_handle_name,
	.description      = xdg_output_handle_description,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_zxdg_output_manager_v1 *wlf_zxdg_output_manager_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version)
{
	assert(registry);

	uint32_t bind_ver =
		(uint32_t)zxdg_output_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zxdg_output_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zxdg_output_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(registry, name,
		&zxdg_output_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"zxdg_output_manager_v1 (name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound zxdg_output_manager_v1 (name: %u, version: %u)",
		name, bind_ver);

	return manager;
}

void wlf_zxdg_output_manager_v1_destroy(
	struct wlf_zxdg_output_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	zxdg_output_manager_v1_destroy(manager->base);
	free(manager);
}

struct wlf_zxdg_output_v1 *wlf_zxdg_output_manager_v1_get_xdg_output(
	struct wlf_zxdg_output_manager_v1 *manager, struct wl_output *wl_output)
{
	assert(manager);
	assert(manager->base);
	assert(wl_output);

	struct wlf_zxdg_output_v1 *output = calloc(1, sizeof(*output));
	if (!output) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zxdg_output_v1");
		return NULL;
	}

	output->base = zxdg_output_manager_v1_get_xdg_output(manager->base,
		wl_output);
	if (!output->base) {
		wlf_log(WLF_ERROR,
			"zxdg_output_manager_v1_get_xdg_output() returned NULL");
		free(output);
		return NULL;
	}

	wlf_signal_init(&output->events.done);
	wlf_signal_init(&output->events.destroy);

	zxdg_output_v1_add_listener(output->base, &xdg_output_listener, output);

	return output;
}

void wlf_zxdg_output_v1_destroy(struct wlf_zxdg_output_v1 *output)
{
	if (!output) {
		return;
	}

	wlf_signal_emit_mutable(&output->events.destroy, output);
	free(output->name);
	free(output->description);
	zxdg_output_v1_destroy(output->base);
	free(output);
}
