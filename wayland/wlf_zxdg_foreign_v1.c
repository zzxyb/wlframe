/**
 * @file        wlf_zxdg_foreign_v1.c
 * @brief       Wayland xdg-foreign-unstable-v1 wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_zxdg_foreign_v1.h"
#include "wayland/protocols/xdg-foreign-unstable-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

/* -------------------------------------------------------------------------
 * Exported listeners
 * ---------------------------------------------------------------------- */

static void exported_handle_handle(void *data,
	struct zxdg_exported_v1 *base, const char *handle)
{
	struct wlf_zxdg_exported_v1 *exported = data;
	(void)base;
	free(exported->handle);
	exported->handle = handle ? strdup(handle) : NULL;
	wlf_signal_emit_mutable(&exported->events.handle, exported);
}

static const struct zxdg_exported_v1_listener exported_listener = {
	.handle = exported_handle_handle,
};

/* -------------------------------------------------------------------------
 * Imported listeners
 * ---------------------------------------------------------------------- */

static void imported_handle_destroyed(void *data,
	struct zxdg_imported_v1 *base)
{
	struct wlf_zxdg_imported_v1 *imported = data;
	(void)base;
	wlf_signal_emit_mutable(&imported->events.destroyed, imported);
}

static const struct zxdg_imported_v1_listener imported_listener = {
	.destroyed = imported_handle_destroyed,
};

/* -------------------------------------------------------------------------
 * Public API — exporter
 * ---------------------------------------------------------------------- */

struct wlf_zxdg_exporter_v1 *wlf_zxdg_exporter_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version)
{
	assert(registry);

	uint32_t bind_ver = (uint32_t)zxdg_exporter_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zxdg_exporter_v1 *exporter = calloc(1, sizeof(*exporter));
	if (!exporter) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zxdg_exporter_v1");
		return NULL;
	}

	exporter->base = wl_registry_bind(registry, name,
		&zxdg_exporter_v1_interface, bind_ver);
	if (!exporter->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"zxdg_exporter_v1 (name: %u)", name);
		free(exporter);
		return NULL;
	}

	wlf_signal_init(&exporter->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound zxdg_exporter_v1 (name: %u, version: %u)",
		name, bind_ver);

	return exporter;
}

void wlf_zxdg_exporter_v1_destroy(struct wlf_zxdg_exporter_v1 *exporter)
{
	if (!exporter) {
		return;
	}

	wlf_signal_emit_mutable(&exporter->events.destroy, exporter);
	zxdg_exporter_v1_destroy(exporter->base);
	free(exporter);
}

struct wlf_zxdg_exported_v1 *wlf_zxdg_exporter_v1_export(
	struct wlf_zxdg_exporter_v1 *exporter, struct wl_surface *surface)
{
	assert(exporter);
	assert(exporter->base);
	assert(surface);

	struct wlf_zxdg_exported_v1 *exported = calloc(1, sizeof(*exported));
	if (!exported) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zxdg_exported_v1");
		return NULL;
	}

	exported->base = zxdg_exporter_v1_export(exporter->base, surface);
	if (!exported->base) {
		wlf_log(WLF_ERROR,
			"zxdg_exporter_v1_export() returned NULL");
		free(exported);
		return NULL;
	}

	wlf_signal_init(&exported->events.handle);
	wlf_signal_init(&exported->events.destroy);

	zxdg_exported_v1_add_listener(exported->base, &exported_listener,
		exported);

	return exported;
}

void wlf_zxdg_exported_v1_destroy(struct wlf_zxdg_exported_v1 *exported)
{
	if (!exported) {
		return;
	}

	wlf_signal_emit_mutable(&exported->events.destroy, exported);
	free(exported->handle);
	zxdg_exported_v1_destroy(exported->base);
	free(exported);
}

/* -------------------------------------------------------------------------
 * Public API — importer
 * ---------------------------------------------------------------------- */

struct wlf_zxdg_importer_v1 *wlf_zxdg_importer_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version)
{
	assert(registry);

	uint32_t bind_ver = (uint32_t)zxdg_importer_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zxdg_importer_v1 *importer = calloc(1, sizeof(*importer));
	if (!importer) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zxdg_importer_v1");
		return NULL;
	}

	importer->base = wl_registry_bind(registry, name,
		&zxdg_importer_v1_interface, bind_ver);
	if (!importer->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"zxdg_importer_v1 (name: %u)", name);
		free(importer);
		return NULL;
	}

	wlf_signal_init(&importer->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound zxdg_importer_v1 (name: %u, version: %u)",
		name, bind_ver);

	return importer;
}

void wlf_zxdg_importer_v1_destroy(struct wlf_zxdg_importer_v1 *importer)
{
	if (!importer) {
		return;
	}

	wlf_signal_emit_mutable(&importer->events.destroy, importer);
	zxdg_importer_v1_destroy(importer->base);
	free(importer);
}

struct wlf_zxdg_imported_v1 *wlf_zxdg_importer_v1_import(
	struct wlf_zxdg_importer_v1 *importer, const char *handle)
{
	assert(importer);
	assert(importer->base);
	assert(handle);

	struct wlf_zxdg_imported_v1 *imported = calloc(1, sizeof(*imported));
	if (!imported) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zxdg_imported_v1");
		return NULL;
	}

	imported->base = zxdg_importer_v1_import(importer->base, handle);
	if (!imported->base) {
		wlf_log(WLF_ERROR,
			"zxdg_importer_v1_import() returned NULL");
		free(imported);
		return NULL;
	}

	wlf_signal_init(&imported->events.destroyed);
	wlf_signal_init(&imported->events.destroy);

	zxdg_imported_v1_add_listener(imported->base, &imported_listener,
		imported);

	return imported;
}

void wlf_zxdg_imported_v1_set_parent_of(
	struct wlf_zxdg_imported_v1 *imported, struct wl_surface *surface)
{
	assert(imported);
	assert(imported->base);
	assert(surface);
	zxdg_imported_v1_set_parent_of(imported->base, surface);
}

void wlf_zxdg_imported_v1_destroy(struct wlf_zxdg_imported_v1 *imported)
{
	if (!imported) {
		return;
	}

	wlf_signal_emit_mutable(&imported->events.destroy, imported);
	zxdg_imported_v1_destroy(imported->base);
	free(imported);
}
