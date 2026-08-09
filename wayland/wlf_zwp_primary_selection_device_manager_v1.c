/**
 * @file        wlf_zwp_primary_selection_device_manager_v1.c
 * @brief       Wayland primary-selection-unstable-v1 wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_zwp_primary_selection_device_manager_v1.h"
#include "wayland/protocols/primary-selection-unstable-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Offer listeners
 * ---------------------------------------------------------------------- */

static void offer_handle_offer(void *data,
	struct zwp_primary_selection_offer_v1 *base, const char *mime_type)
{
	struct wlf_zwp_primary_selection_offer_v1 *offer = data;
	(void)base;

	char **new_types = realloc(offer->mime_types,
		(offer->n_mime_types + 2) * sizeof(char *));
	if (!new_types) {
		wlf_log_errno(WLF_ERROR, "failed to grow offer->mime_types");
		return;
	}
	offer->mime_types = new_types;
	offer->mime_types[offer->n_mime_types] =
		mime_type ? strdup(mime_type) : NULL;
	offer->n_mime_types++;
	offer->mime_types[offer->n_mime_types] = NULL;
}

static const struct zwp_primary_selection_offer_v1_listener offer_listener = {
	.offer = offer_handle_offer,
};

/* -------------------------------------------------------------------------
 * Source listeners
 * ---------------------------------------------------------------------- */

static void source_handle_send(void *data,
	struct zwp_primary_selection_source_v1 *base,
	const char *mime_type, int32_t fd)
{
	struct wlf_zwp_primary_selection_source_v1 *source = data;
	(void)base;
	source->send_mime_type = mime_type;
	source->send_fd        = fd;
	wlf_signal_emit_mutable(&source->events.send, source);
}

static void source_handle_cancelled(void *data,
	struct zwp_primary_selection_source_v1 *base)
{
	struct wlf_zwp_primary_selection_source_v1 *source = data;
	(void)base;
	wlf_signal_emit_mutable(&source->events.cancelled, source);
}

static const struct zwp_primary_selection_source_v1_listener source_listener = {
	.send      = source_handle_send,
	.cancelled = source_handle_cancelled,
};

/* -------------------------------------------------------------------------
 * Device listeners
 * ---------------------------------------------------------------------- */

static void device_handle_data_offer(void *data,
	struct zwp_primary_selection_device_v1 *base,
	struct zwp_primary_selection_offer_v1 *offer_base)
{
	struct wlf_zwp_primary_selection_device_v1 *device = data;
	(void)base;

	struct wlf_zwp_primary_selection_offer_v1 *offer =
		calloc(1, sizeof(*offer));
	if (!offer) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_primary_selection_offer_v1");
		zwp_primary_selection_offer_v1_destroy(offer_base);
		return;
	}

	offer->base = offer_base;
	wlf_signal_init(&offer->events.destroy);

	zwp_primary_selection_offer_v1_add_listener(offer_base,
		&offer_listener, offer);

	wlf_signal_emit_mutable(&device->events.data_offer, offer);
}

static void device_handle_selection(void *data,
	struct zwp_primary_selection_device_v1 *base,
	struct zwp_primary_selection_offer_v1 *offer_base)
{
	struct wlf_zwp_primary_selection_device_v1 *device = data;
	(void)base;

	if (!offer_base) {
		wlf_signal_emit_mutable(&device->events.selection, NULL);
		return;
	}

	struct wlf_zwp_primary_selection_offer_v1 *offer =
		zwp_primary_selection_offer_v1_get_user_data(offer_base);
	wlf_signal_emit_mutable(&device->events.selection, offer);
}

static const struct zwp_primary_selection_device_v1_listener device_listener = {
	.data_offer = device_handle_data_offer,
	.selection  = device_handle_selection,
};

/* -------------------------------------------------------------------------
 * Public API — manager
 * ---------------------------------------------------------------------- */

struct wlf_zwp_primary_selection_device_manager_v1 *
wlf_zwp_primary_selection_device_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver =
		(uint32_t)zwp_primary_selection_device_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zwp_primary_selection_device_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate "
			"wlf_zwp_primary_selection_device_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&zwp_primary_selection_device_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"zwp_primary_selection_device_manager_v1 (name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound zwp_primary_selection_device_manager_v1 "
		"(name: %u, version: %u)", name, bind_ver);

	return manager;
}

void wlf_zwp_primary_selection_device_manager_v1_destroy(
	struct wlf_zwp_primary_selection_device_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	zwp_primary_selection_device_manager_v1_destroy(manager->base);
	free(manager);
}

struct wlf_zwp_primary_selection_source_v1 *
wlf_zwp_primary_selection_device_manager_v1_create_source(
	struct wlf_zwp_primary_selection_device_manager_v1 *manager)
{
	assert(manager);
	assert(manager->base);

	struct wlf_zwp_primary_selection_source_v1 *source =
		calloc(1, sizeof(*source));
	if (!source) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate "
			"wlf_zwp_primary_selection_source_v1");
		return NULL;
	}

	source->base =
		zwp_primary_selection_device_manager_v1_create_source(
			manager->base);
	if (!source->base) {
		wlf_log(WLF_ERROR,
			"zwp_primary_selection_device_manager_v1_create_source()"
			" returned NULL");
		free(source);
		return NULL;
	}

	wlf_signal_init(&source->events.send);
	wlf_signal_init(&source->events.cancelled);
	wlf_signal_init(&source->events.destroy);

	zwp_primary_selection_source_v1_add_listener(source->base,
		&source_listener, source);

	return source;
}

struct wlf_zwp_primary_selection_device_v1 *
wlf_zwp_primary_selection_device_manager_v1_get_device(
	struct wlf_zwp_primary_selection_device_manager_v1 *manager,
	struct wl_seat *seat)
{
	assert(manager);
	assert(manager->base);
	assert(seat);

	struct wlf_zwp_primary_selection_device_v1 *device =
		calloc(1, sizeof(*device));
	if (!device) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate "
			"wlf_zwp_primary_selection_device_v1");
		return NULL;
	}

	device->base = zwp_primary_selection_device_manager_v1_get_device(
		manager->base, seat);
	if (!device->base) {
		wlf_log(WLF_ERROR,
			"zwp_primary_selection_device_manager_v1_get_device()"
			" returned NULL");
		free(device);
		return NULL;
	}

	wlf_signal_init(&device->events.data_offer);
	wlf_signal_init(&device->events.selection);
	wlf_signal_init(&device->events.destroy);

	zwp_primary_selection_device_v1_add_listener(device->base,
		&device_listener, device);

	return device;
}

/* -------------------------------------------------------------------------
 * Public API — device
 * ---------------------------------------------------------------------- */

void wlf_zwp_primary_selection_device_v1_set_selection(
	struct wlf_zwp_primary_selection_device_v1 *device,
	struct wlf_zwp_primary_selection_source_v1 *source, uint32_t serial)
{
	assert(device);
	assert(device->base);
	zwp_primary_selection_device_v1_set_selection(device->base,
		source ? source->base : NULL, serial);
}

void wlf_zwp_primary_selection_device_v1_destroy(
	struct wlf_zwp_primary_selection_device_v1 *device)
{
	if (!device) {
		return;
	}

	wlf_signal_emit_mutable(&device->events.destroy, device);
	zwp_primary_selection_device_v1_destroy(device->base);
	free(device);
}

/* -------------------------------------------------------------------------
 * Public API — offer
 * ---------------------------------------------------------------------- */

void wlf_zwp_primary_selection_offer_v1_receive(
	struct wlf_zwp_primary_selection_offer_v1 *offer,
	const char *mime_type, int fd)
{
	assert(offer);
	assert(offer->base);
	zwp_primary_selection_offer_v1_receive(offer->base, mime_type, fd);
}

void wlf_zwp_primary_selection_offer_v1_destroy(
	struct wlf_zwp_primary_selection_offer_v1 *offer)
{
	if (!offer) {
		return;
	}

	wlf_signal_emit_mutable(&offer->events.destroy, offer);
	zwp_primary_selection_offer_v1_destroy(offer->base);
	for (size_t i = 0; i < offer->n_mime_types; i++) {
		free(offer->mime_types[i]);
	}
	free(offer->mime_types);
	free(offer);
}

/* -------------------------------------------------------------------------
 * Public API — source
 * ---------------------------------------------------------------------- */

void wlf_zwp_primary_selection_source_v1_offer(
	struct wlf_zwp_primary_selection_source_v1 *source,
	const char *mime_type)
{
	assert(source);
	assert(source->base);
	assert(mime_type);
	zwp_primary_selection_source_v1_offer(source->base, mime_type);
}

void wlf_zwp_primary_selection_source_v1_destroy(
	struct wlf_zwp_primary_selection_source_v1 *source)
{
	if (!source) {
		return;
	}

	wlf_signal_emit_mutable(&source->events.destroy, source);
	zwp_primary_selection_source_v1_destroy(source->base);
	free(source);
}
