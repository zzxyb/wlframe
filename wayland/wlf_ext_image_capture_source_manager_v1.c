/**
 * @file        wlf_ext_image_capture_source_manager_v1.c
 * @brief       Wayland ext-image-capture-source-v1 wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_ext_image_capture_source_manager_v1.h"
#include "wayland/protocols/ext-image-capture-source-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

/* -------------------------------------------------------------------------
 * Public API — output capture source manager
 * ---------------------------------------------------------------------- */

struct wlf_ext_output_image_capture_source_manager_v1 *
wlf_ext_output_image_capture_source_manager_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version)
{
	assert(registry);

	uint32_t bind_ver =
		(uint32_t)ext_output_image_capture_source_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_ext_output_image_capture_source_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate "
			"wlf_ext_output_image_capture_source_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(registry, name,
		&ext_output_image_capture_source_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"ext_output_image_capture_source_manager_v1 (name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound ext_output_image_capture_source_manager_v1 "
		"(name: %u, version: %u)", name, bind_ver);

	return manager;
}

void wlf_ext_output_image_capture_source_manager_v1_destroy(
	struct wlf_ext_output_image_capture_source_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	ext_output_image_capture_source_manager_v1_destroy(manager->base);
	free(manager);
}

struct wlf_ext_image_capture_source_v1 *
wlf_ext_output_image_capture_source_manager_v1_create_source(
	struct wlf_ext_output_image_capture_source_manager_v1 *manager,
	struct wl_output *output)
{
	assert(manager);
	assert(manager->base);
	assert(output);

	struct wlf_ext_image_capture_source_v1 *source =
		calloc(1, sizeof(*source));
	if (!source) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_ext_image_capture_source_v1");
		return NULL;
	}

	source->base =
		ext_output_image_capture_source_manager_v1_create_source(
			manager->base, output);
	if (!source->base) {
		wlf_log(WLF_ERROR,
			"ext_output_image_capture_source_manager_v1_create_source()"
			" returned NULL");
		free(source);
		return NULL;
	}

	wlf_signal_init(&source->events.destroy);

	return source;
}

/* -------------------------------------------------------------------------
 * Public API — foreign toplevel capture source manager
 * ---------------------------------------------------------------------- */

struct wlf_ext_foreign_toplevel_image_capture_source_manager_v1 *
wlf_ext_foreign_toplevel_image_capture_source_manager_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version)
{
	assert(registry);

	uint32_t bind_ver =
		(uint32_t)ext_foreign_toplevel_image_capture_source_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_ext_foreign_toplevel_image_capture_source_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate "
			"wlf_ext_foreign_toplevel_image_capture_source_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(registry, name,
		&ext_foreign_toplevel_image_capture_source_manager_v1_interface,
		bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"ext_foreign_toplevel_image_capture_source_manager_v1 "
			"(name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound ext_foreign_toplevel_image_capture_source_manager_v1 "
		"(name: %u, version: %u)", name, bind_ver);

	return manager;
}

void wlf_ext_foreign_toplevel_image_capture_source_manager_v1_destroy(
	struct wlf_ext_foreign_toplevel_image_capture_source_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	ext_foreign_toplevel_image_capture_source_manager_v1_destroy(manager->base);
	free(manager);
}

struct wlf_ext_image_capture_source_v1 *
wlf_ext_foreign_toplevel_image_capture_source_manager_v1_create_source(
	struct wlf_ext_foreign_toplevel_image_capture_source_manager_v1 *manager,
	struct ext_foreign_toplevel_handle_v1 *toplevel_handle)
{
	assert(manager);
	assert(manager->base);
	assert(toplevel_handle);

	struct wlf_ext_image_capture_source_v1 *source =
		calloc(1, sizeof(*source));
	if (!source) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_ext_image_capture_source_v1");
		return NULL;
	}

	source->base =
		ext_foreign_toplevel_image_capture_source_manager_v1_create_source(
			manager->base, toplevel_handle);
	if (!source->base) {
		wlf_log(WLF_ERROR,
			"ext_foreign_toplevel_image_capture_source_manager_v1_create_source()"
			" returned NULL");
		free(source);
		return NULL;
	}

	wlf_signal_init(&source->events.destroy);

	return source;
}

/* -------------------------------------------------------------------------
 * Public API — capture source
 * ---------------------------------------------------------------------- */

void wlf_ext_image_capture_source_v1_destroy(
	struct wlf_ext_image_capture_source_v1 *source)
{
	if (!source) {
		return;
	}

	wlf_signal_emit_mutable(&source->events.destroy, source);
	ext_image_capture_source_v1_destroy(source->base);
	free(source);
}
