/**
 * @file        wlf_wp_drm_lease_device_v1.c
 * @brief       Wayland wp_drm_lease_device_v1 protocol wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_wp_drm_lease_device_v1.h"
#include "wayland/protocols/drm-lease-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* -------------------------------------------------------------------------
 * Connector listeners
 * ---------------------------------------------------------------------- */

static void connector_handle_name(void *data,
	struct wp_drm_lease_connector_v1 *base, const char *name)
{
	struct wlf_wp_drm_lease_connector_v1 *connector = data;
	(void)base;
	free(connector->name);
	connector->name = name ? strdup(name) : NULL;
}

static void connector_handle_description(void *data,
	struct wp_drm_lease_connector_v1 *base, const char *description)
{
	struct wlf_wp_drm_lease_connector_v1 *connector = data;
	(void)base;
	free(connector->description);
	connector->description = description ? strdup(description) : NULL;
}

static void connector_handle_connector_id(void *data,
	struct wp_drm_lease_connector_v1 *base, uint32_t connector_id)
{
	struct wlf_wp_drm_lease_connector_v1 *connector = data;
	(void)base;
	connector->connector_id = connector_id;
}

static void connector_handle_done(void *data,
	struct wp_drm_lease_connector_v1 *base)
{
	struct wlf_wp_drm_lease_connector_v1 *connector = data;
	(void)base;
	wlf_signal_emit_mutable(&connector->events.done, connector);
}

static void connector_handle_withdrawn(void *data,
	struct wp_drm_lease_connector_v1 *base)
{
	struct wlf_wp_drm_lease_connector_v1 *connector = data;
	(void)base;
	wlf_signal_emit_mutable(&connector->events.withdrawn, connector);
}

static const struct wp_drm_lease_connector_v1_listener connector_listener = {
	.name         = connector_handle_name,
	.description  = connector_handle_description,
	.connector_id = connector_handle_connector_id,
	.done         = connector_handle_done,
	.withdrawn    = connector_handle_withdrawn,
};

/* -------------------------------------------------------------------------
 * Lease listeners
 * ---------------------------------------------------------------------- */

static void lease_handle_lease_fd(void *data,
	struct wp_drm_lease_v1 *base, int32_t leased_fd)
{
	struct wlf_wp_drm_lease_v1 *lease = data;
	(void)base;
	lease->fd = leased_fd;
	wlf_signal_emit_mutable(&lease->events.lease_fd, lease);
}

static void lease_handle_finished(void *data, struct wp_drm_lease_v1 *base)
{
	struct wlf_wp_drm_lease_v1 *lease = data;
	(void)base;
	wlf_signal_emit_mutable(&lease->events.finished, lease);
}

static const struct wp_drm_lease_v1_listener lease_listener = {
	.lease_fd = lease_handle_lease_fd,
	.finished = lease_handle_finished,
};

/* -------------------------------------------------------------------------
 * Device listeners
 * ---------------------------------------------------------------------- */

static void device_handle_drm_fd(void *data,
	struct wp_drm_lease_device_v1 *base, int32_t fd)
{
	struct wlf_wp_drm_lease_device_v1 *device = data;
	(void)base;
	if (device->drm_fd >= 0) {
		close(device->drm_fd);
	}
	device->drm_fd = fd;
}

static void device_handle_connector(void *data,
	struct wp_drm_lease_device_v1 *base,
	struct wp_drm_lease_connector_v1 *connector_base)
{
	struct wlf_wp_drm_lease_device_v1 *device = data;
	(void)base;

	struct wlf_wp_drm_lease_connector_v1 *connector =
		calloc(1, sizeof(*connector));
	if (!connector) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_wp_drm_lease_connector_v1");
		wp_drm_lease_connector_v1_destroy(connector_base);
		return;
	}

	connector->base = connector_base;
	wlf_signal_init(&connector->events.done);
	wlf_signal_init(&connector->events.withdrawn);
	wlf_signal_init(&connector->events.destroy);

	wp_drm_lease_connector_v1_add_listener(connector_base,
		&connector_listener, connector);

	wlf_signal_emit_mutable(&device->events.connector, connector);
}

static void device_handle_done(void *data,
	struct wp_drm_lease_device_v1 *base)
{
	struct wlf_wp_drm_lease_device_v1 *device = data;
	(void)base;
	wlf_signal_emit_mutable(&device->events.done, device);
}

static void device_handle_released(void *data,
	struct wp_drm_lease_device_v1 *base)
{
	/* data is NULL when destroy() was called before released fires */
	(void)base;
	if (!data) {
		return;
	}
	struct wlf_wp_drm_lease_device_v1 *device = data;
	wlf_signal_emit_mutable(&device->events.released, device);
}

static const struct wp_drm_lease_device_v1_listener device_listener = {
	.drm_fd    = device_handle_drm_fd,
	.connector = device_handle_connector,
	.done      = device_handle_done,
	.released  = device_handle_released,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_wp_drm_lease_device_v1 *wlf_wp_drm_lease_device_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver = (uint32_t)wp_drm_lease_device_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_wp_drm_lease_device_v1 *device = calloc(1, sizeof(*device));
	if (!device) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_wp_drm_lease_device_v1");
		return NULL;
	}

	device->drm_fd = -1;
	device->base = wl_registry_bind(wl_registry, name,
		&wp_drm_lease_device_v1_interface, bind_ver);
	if (!device->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for wp_drm_lease_device_v1 (name: %u)",
			name);
		free(device);
		return NULL;
	}

	wlf_signal_init(&device->events.connector);
	wlf_signal_init(&device->events.done);
	wlf_signal_init(&device->events.released);
	wlf_signal_init(&device->events.destroy);

	wp_drm_lease_device_v1_add_listener(device->base, &device_listener,
		device);

	wlf_log(WLF_DEBUG,
		"bound wp_drm_lease_device_v1 (name: %u, version: %u)",
		name, bind_ver);

	return device;
}

void wlf_wp_drm_lease_device_v1_destroy(
	struct wlf_wp_drm_lease_device_v1 *device)
{
	if (!device) {
		return;
	}

	wlf_signal_emit_mutable(&device->events.destroy, device);
	/* Null the user_data so the released event handler ignores late callbacks */
	wp_drm_lease_device_v1_set_user_data(device->base, NULL);
	wp_drm_lease_device_v1_release(device->base);
	if (device->drm_fd >= 0) {
		close(device->drm_fd);
	}
	free(device);
}

struct wp_drm_lease_request_v1 *
wlf_wp_drm_lease_device_v1_create_lease_request(
	struct wlf_wp_drm_lease_device_v1 *device)
{
	assert(device);
	assert(device->base);
	return wp_drm_lease_device_v1_create_lease_request(device->base);
}

void wlf_wp_drm_lease_request_v1_request_connector(
	struct wp_drm_lease_request_v1 *request,
	struct wlf_wp_drm_lease_connector_v1 *connector)
{
	assert(request);
	assert(connector);
	assert(connector->base);
	wp_drm_lease_request_v1_request_connector(request, connector->base);
}

struct wlf_wp_drm_lease_v1 *wlf_wp_drm_lease_request_v1_submit(
	struct wp_drm_lease_request_v1 *request)
{
	assert(request);

	struct wlf_wp_drm_lease_v1 *lease = calloc(1, sizeof(*lease));
	if (!lease) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_wp_drm_lease_v1");
		return NULL;
	}

	lease->fd = -1;
	lease->base = wp_drm_lease_request_v1_submit(request);
	if (!lease->base) {
		wlf_log(WLF_ERROR,
			"wp_drm_lease_request_v1_submit() returned NULL");
		free(lease);
		return NULL;
	}

	wlf_signal_init(&lease->events.lease_fd);
	wlf_signal_init(&lease->events.finished);
	wlf_signal_init(&lease->events.destroy);

	wp_drm_lease_v1_add_listener(lease->base, &lease_listener, lease);
	return lease;
}

void wlf_wp_drm_lease_connector_v1_destroy(
	struct wlf_wp_drm_lease_connector_v1 *connector)
{
	if (!connector) {
		return;
	}

	wlf_signal_emit_mutable(&connector->events.destroy, connector);
	wp_drm_lease_connector_v1_destroy(connector->base);
	free(connector->name);
	free(connector->description);
	free(connector);
}

void wlf_wp_drm_lease_v1_destroy(struct wlf_wp_drm_lease_v1 *lease)
{
	if (!lease) {
		return;
	}

	wlf_signal_emit_mutable(&lease->events.destroy, lease);
	wp_drm_lease_v1_destroy(lease->base);
	if (lease->fd >= 0) {
		close(lease->fd);
	}
	free(lease);
}
