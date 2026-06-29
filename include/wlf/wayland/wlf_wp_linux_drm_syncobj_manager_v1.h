/**
 * @file        wlf_wp_linux_drm_syncobj_manager_v1.h
 * @brief       Wayland wp_linux_drm_syncobj_manager_v1 protocol wrapper for
 *              wlframe.
 * @details     Wraps the staging linux-drm-syncobj-v1 protocol, which provides
 *              explicit GPU synchronisation for Wayland surfaces using DRM
 *              syncobj timeline points.
 *
 *              Three interfaces are wrapped here:
 *
 *              - wlf_wp_linux_drm_syncobj_manager_v1  — the global manager,
 *                bound from the registry.  Creates timeline and surface objects.
 *
 *              - wlf_wp_linux_drm_syncobj_timeline_v1 — wraps a DRM syncobj
 *                imported from a file descriptor.  No events; destroy only.
 *
 *              - wlf_wp_linux_drm_syncobj_surface_v1  — per-surface sync
 *                controller.  Call set_acquire_point / set_release_point
 *                before each wl_surface.commit to specify synchronisation
 *                timeline points (64-bit point encoded as hi + lo uint32).
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_WP_LINUX_DRM_SYNCOBJ_MANAGER_V1_H
#define WAYLAND_WLF_WP_LINUX_DRM_SYNCOBJ_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_surface;
struct wp_linux_drm_syncobj_manager_v1;
struct wp_linux_drm_syncobj_timeline_v1;
struct wp_linux_drm_syncobj_surface_v1;

/**
 * @brief Wrapper around a bound wp_linux_drm_syncobj_manager_v1 global.
 */
struct wlf_wp_linux_drm_syncobj_manager_v1 {
	struct wp_linux_drm_syncobj_manager_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Wrapper around a wp_linux_drm_syncobj_timeline_v1.
 *
 * Represents a DRM syncobj timeline imported from a file descriptor.
 * The caller must close the fd independently; this object does not own it.
 * There are no events; destroy only.
 */
struct wlf_wp_linux_drm_syncobj_timeline_v1 {
	struct wp_linux_drm_syncobj_timeline_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Per-surface DRM syncobj controller.
 *
 * Created by wlf_wp_linux_drm_syncobj_manager_v1_get_surface().
 * Call set_acquire_point / set_release_point before each commit.
 * No events; destroy only.
 */
struct wlf_wp_linux_drm_syncobj_surface_v1 {
	struct wp_linux_drm_syncobj_surface_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the wp_linux_drm_syncobj_manager_v1 global from the registry.
 */
struct wlf_wp_linux_drm_syncobj_manager_v1 *
wlf_wp_linux_drm_syncobj_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 */
void wlf_wp_linux_drm_syncobj_manager_v1_destroy(
	struct wlf_wp_linux_drm_syncobj_manager_v1 *manager);

/**
 * @brief Import a DRM syncobj file descriptor as a timeline object.
 *
 * The manager does not take ownership of @p fd; the caller should close it
 * after this call returns.
 *
 * @param manager  Bound manager.
 * @param fd       A drm_syncobj file descriptor.
 * @return Newly allocated timeline object, or NULL on failure.
 */
struct wlf_wp_linux_drm_syncobj_timeline_v1 *
wlf_wp_linux_drm_syncobj_manager_v1_import_timeline(
	struct wlf_wp_linux_drm_syncobj_manager_v1 *manager, int fd);

/**
 * @brief Destroy a timeline object.
 */
void wlf_wp_linux_drm_syncobj_timeline_v1_destroy(
	struct wlf_wp_linux_drm_syncobj_timeline_v1 *timeline);

/**
 * @brief Create a per-surface syncobj controller.
 *
 * @param manager  Bound manager.
 * @param surface  wl_surface to associate with.
 * @return Newly allocated surface object, or NULL on failure.
 */
struct wlf_wp_linux_drm_syncobj_surface_v1 *
wlf_wp_linux_drm_syncobj_manager_v1_get_surface(
	struct wlf_wp_linux_drm_syncobj_manager_v1 *manager,
	struct wl_surface *surface);

/**
 * @brief Set the acquire timeline point for the next commit.
 *
 * The 64-bit point value is passed as two 32-bit halves.
 *
 * @param surface   Per-surface syncobj object.
 * @param timeline  Timeline object to wait on.
 * @param point     64-bit timeline point value.
 */
void wlf_wp_linux_drm_syncobj_surface_v1_set_acquire_point(
	struct wlf_wp_linux_drm_syncobj_surface_v1 *surface,
	struct wlf_wp_linux_drm_syncobj_timeline_v1 *timeline,
	uint64_t point);

/**
 * @brief Set the release timeline point for the next commit.
 *
 * The 64-bit point value is passed as two 32-bit halves.
 *
 * @param surface   Per-surface syncobj object.
 * @param timeline  Timeline object to signal on.
 * @param point     64-bit timeline point value.
 */
void wlf_wp_linux_drm_syncobj_surface_v1_set_release_point(
	struct wlf_wp_linux_drm_syncobj_surface_v1 *surface,
	struct wlf_wp_linux_drm_syncobj_timeline_v1 *timeline,
	uint64_t point);

/**
 * @brief Destroy the per-surface syncobj controller.
 */
void wlf_wp_linux_drm_syncobj_surface_v1_destroy(
	struct wlf_wp_linux_drm_syncobj_surface_v1 *surface);

#endif /* WAYLAND_WLF_WP_LINUX_DRM_SYNCOBJ_MANAGER_V1_H */
