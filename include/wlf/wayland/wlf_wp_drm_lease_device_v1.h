/**
 * @file        wlf_wp_drm_lease_device_v1.h
 * @brief       Wayland wp_drm_lease_device_v1 protocol wrapper for wlframe.
 * @details     Implements the staging drm-lease-v1 protocol, which lets a
 *              client request exclusive DRM KMS access ("lease") to specific
 *              connectors.  A direct-rendering compositor (e.g. a VR runtime)
 *              uses this to take over certain outputs from the compositor.
 *
 *              Interfaces wrapped:
 *              - wlf_wp_drm_lease_device_v1   — global device; carries the
 *                DRM fd and enumerates available connectors.
 *              - wlf_wp_drm_lease_connector_v1 — one leasable connector; has
 *                name, description, connector_id, done, withdrawn signals.
 *              - wlf_wp_drm_lease_request_v1  — a lease request (transient;
 *                submit converts it to a lease).
 *              - wlf_wp_drm_lease_v1          — an active lease; carries
 *                the leased DRM fd.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WP_DRM_LEASE_DEVICE_V1_H
#define WAYLAND_WLF_WP_DRM_LEASE_DEVICE_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wp_drm_lease_device_v1;
struct wp_drm_lease_connector_v1;
struct wp_drm_lease_request_v1;
struct wp_drm_lease_v1;

/* Forward declarations */
struct wlf_wp_drm_lease_connector_v1;

/**
 * @brief Wrapper around a bound wp_drm_lease_device_v1 global.
 *
 * The compositor emits connector events (one per available connector),
 * followed by a done event.  The DRM fd is delivered via the drm_fd event.
 */
struct wlf_wp_drm_lease_device_v1 {
	struct wp_drm_lease_device_v1 *base;

	/** DRM file descriptor delivered by the drm_fd event (-1 until set). */
	int drm_fd;

	struct {
		/** Data: wlf_wp_drm_lease_connector_v1 * — new connector   */
		struct wlf_signal connector;
		/** Emitted after initial connector enumeration. Data: self  */
		struct wlf_signal done;
		/** Emitted when the device is released. Data: self          */
		struct wlf_signal released;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Represents a single leasable DRM connector.
 *
 * String fields (name, description) are heap-allocated; freed on destroy.
 */
struct wlf_wp_drm_lease_connector_v1 {
	struct wp_drm_lease_connector_v1 *base;

	char *name;           /**< Connector name string                 */
	char *description;    /**< Human-readable description            */
	uint32_t connector_id; /**< DRM connector ID                     */

	struct {
		/** Emitted after all descriptor events. Data: self          */
		struct wlf_signal done;
		/** Emitted when connector is no longer leasable. Data: self */
		struct wlf_signal withdrawn;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Active DRM lease object.
 *
 * The leased DRM fd is delivered via events.lease_fd (stored in @c fd).
 * The compositor signals revocation via events.finished.
 */
struct wlf_wp_drm_lease_v1 {
	struct wp_drm_lease_v1 *base;

	/** Leased DRM fd (-1 until lease_fd event fires). */
	int fd;

	struct {
		/** Emitted with the leased fd. Data: self                   */
		struct wlf_signal lease_fd;
		/** Emitted when the lease is terminated. Data: self         */
		struct wlf_signal finished;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the wp_drm_lease_device_v1 global from the registry.
 */
struct wlf_wp_drm_lease_device_v1 *wlf_wp_drm_lease_device_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Release the device binding and free its resources.
 */
void wlf_wp_drm_lease_device_v1_destroy(
	struct wlf_wp_drm_lease_device_v1 *device);

/**
 * @brief Create a lease request.
 *
 * @param device  Bound device.
 * @return A raw wp_drm_lease_request_v1.  Add connectors, then call
 *         wlf_wp_drm_lease_device_v1_submit_request() to convert it
 *         into a wlf_wp_drm_lease_v1.
 */
struct wp_drm_lease_request_v1 *
wlf_wp_drm_lease_device_v1_create_lease_request(
	struct wlf_wp_drm_lease_device_v1 *device);

/**
 * @brief Add a connector to a lease request.
 *
 * @param request    Raw lease request object.
 * @param connector  Connector to add to the request.
 */
void wlf_wp_drm_lease_request_v1_request_connector(
	struct wp_drm_lease_request_v1 *request,
	struct wlf_wp_drm_lease_connector_v1 *connector);

/**
 * @brief Submit the lease request (destructor; request is consumed).
 *
 * @param request  Raw lease request to submit.
 * @return A new wlf_wp_drm_lease_v1, or NULL on failure.
 */
struct wlf_wp_drm_lease_v1 *wlf_wp_drm_lease_request_v1_submit(
	struct wp_drm_lease_request_v1 *request);

/**
 * @brief Destroy a connector wrapper object.
 */
void wlf_wp_drm_lease_connector_v1_destroy(
	struct wlf_wp_drm_lease_connector_v1 *connector);

/**
 * @brief Destroy an active lease object and close the leased fd.
 */
void wlf_wp_drm_lease_v1_destroy(struct wlf_wp_drm_lease_v1 *lease);

#endif /* WAYLAND_WLF_WP_DRM_LEASE_DEVICE_V1_H */
