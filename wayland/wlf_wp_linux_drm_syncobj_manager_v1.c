/**
 * @file        wlf_wp_linux_drm_syncobj_manager_v1.c
 * @brief       Wayland wp_linux_drm_syncobj_manager_v1 protocol wrapper for
 *              wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#include "wlf/wayland/wlf_wp_linux_drm_syncobj_manager_v1.h"
#include "wayland/protocols/linux-drm-syncobj-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Manager
 * ---------------------------------------------------------------------- */

struct wlf_wp_linux_drm_syncobj_manager_v1 *
wlf_wp_linux_drm_syncobj_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver = wp_linux_drm_syncobj_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
		wlf_log(WLF_DEBUG,
			"wp_linux_drm_syncobj_manager_v1: binding version %u "
			"(server advertised %u, interface supports %u)",
			bind_ver, version,
			wp_linux_drm_syncobj_manager_v1_interface.version);
	}

	struct wlf_wp_linux_drm_syncobj_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_wp_linux_drm_syncobj_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&wp_linux_drm_syncobj_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for wp_linux_drm_syncobj_manager_v1");
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);
	return manager;
}

void wlf_wp_linux_drm_syncobj_manager_v1_destroy(
	struct wlf_wp_linux_drm_syncobj_manager_v1 *manager)
{
	assert(manager);
	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	wp_linux_drm_syncobj_manager_v1_destroy(manager->base);
	free(manager);
}

/* -------------------------------------------------------------------------
 * Timeline
 * ---------------------------------------------------------------------- */

struct wlf_wp_linux_drm_syncobj_timeline_v1 *
wlf_wp_linux_drm_syncobj_manager_v1_import_timeline(
	struct wlf_wp_linux_drm_syncobj_manager_v1 *manager, int fd)
{
	assert(manager);

	struct wlf_wp_linux_drm_syncobj_timeline_v1 *timeline =
		calloc(1, sizeof(*timeline));
	if (!timeline) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_wp_linux_drm_syncobj_timeline_v1");
		return NULL;
	}

	timeline->base = wp_linux_drm_syncobj_manager_v1_import_timeline(
		manager->base, fd);
	if (!timeline->base) {
		wlf_log(WLF_ERROR,
			"wp_linux_drm_syncobj_manager_v1_import_timeline failed");
		free(timeline);
		return NULL;
	}

	wlf_signal_init(&timeline->events.destroy);
	return timeline;
}

void wlf_wp_linux_drm_syncobj_timeline_v1_destroy(
	struct wlf_wp_linux_drm_syncobj_timeline_v1 *timeline)
{
	assert(timeline);
	wlf_signal_emit_mutable(&timeline->events.destroy, timeline);
	wp_linux_drm_syncobj_timeline_v1_destroy(timeline->base);
	free(timeline);
}

/* -------------------------------------------------------------------------
 * Per-surface object
 * ---------------------------------------------------------------------- */

struct wlf_wp_linux_drm_syncobj_surface_v1 *
wlf_wp_linux_drm_syncobj_manager_v1_get_surface(
	struct wlf_wp_linux_drm_syncobj_manager_v1 *manager,
	struct wl_surface *surface)
{
	assert(manager);
	assert(surface);

	struct wlf_wp_linux_drm_syncobj_surface_v1 *surf =
		calloc(1, sizeof(*surf));
	if (!surf) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_wp_linux_drm_syncobj_surface_v1");
		return NULL;
	}

	surf->base = wp_linux_drm_syncobj_manager_v1_get_surface(
		manager->base, surface);
	if (!surf->base) {
		wlf_log(WLF_ERROR,
			"wp_linux_drm_syncobj_manager_v1_get_surface failed");
		free(surf);
		return NULL;
	}

	wlf_signal_init(&surf->events.destroy);
	return surf;
}

void wlf_wp_linux_drm_syncobj_surface_v1_set_acquire_point(
	struct wlf_wp_linux_drm_syncobj_surface_v1 *surface,
	struct wlf_wp_linux_drm_syncobj_timeline_v1 *timeline,
	uint64_t point)
{
	assert(surface);
	assert(timeline);
	uint32_t hi = (uint32_t)(point >> 32);
	uint32_t lo = (uint32_t)(point & 0xFFFFFFFFu);
	wp_linux_drm_syncobj_surface_v1_set_acquire_point(
		surface->base, timeline->base, hi, lo);
}

void wlf_wp_linux_drm_syncobj_surface_v1_set_release_point(
	struct wlf_wp_linux_drm_syncobj_surface_v1 *surface,
	struct wlf_wp_linux_drm_syncobj_timeline_v1 *timeline,
	uint64_t point)
{
	assert(surface);
	assert(timeline);
	uint32_t hi = (uint32_t)(point >> 32);
	uint32_t lo = (uint32_t)(point & 0xFFFFFFFFu);
	wp_linux_drm_syncobj_surface_v1_set_release_point(
		surface->base, timeline->base, hi, lo);
}

void wlf_wp_linux_drm_syncobj_surface_v1_destroy(
	struct wlf_wp_linux_drm_syncobj_surface_v1 *surface)
{
	assert(surface);
	wlf_signal_emit_mutable(&surface->events.destroy, surface);
	wp_linux_drm_syncobj_surface_v1_destroy(surface->base);
	free(surface);
}
