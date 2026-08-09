/**
 * @file        wlf_ext_transient_seat_manager_v1.c
 * @brief       Wayland ext-transient-seat-v1 wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_ext_transient_seat_manager_v1.h"
#include "wayland/protocols/ext-transient-seat-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

/* -------------------------------------------------------------------------
 * Transient seat listeners
 * ---------------------------------------------------------------------- */

static void transient_seat_handle_ready(void *data,
	struct ext_transient_seat_v1 *base, uint32_t global_name)
{
	struct wlf_ext_transient_seat_v1 *seat = data;
	(void)base;
	seat->global_name = global_name;
	wlf_signal_emit_mutable(&seat->events.ready, seat);
}

static void transient_seat_handle_denied(void *data,
	struct ext_transient_seat_v1 *base)
{
	struct wlf_ext_transient_seat_v1 *seat = data;
	(void)base;
	wlf_signal_emit_mutable(&seat->events.denied, seat);
}

static const struct ext_transient_seat_v1_listener transient_seat_listener = {
	.ready  = transient_seat_handle_ready,
	.denied = transient_seat_handle_denied,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_ext_transient_seat_manager_v1 *
wlf_ext_transient_seat_manager_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version)
{
	assert(registry);

	uint32_t bind_ver =
		(uint32_t)ext_transient_seat_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_ext_transient_seat_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_ext_transient_seat_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(registry, name,
		&ext_transient_seat_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"ext_transient_seat_manager_v1 (name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound ext_transient_seat_manager_v1 (name: %u, version: %u)",
		name, bind_ver);

	return manager;
}

void wlf_ext_transient_seat_manager_v1_destroy(
	struct wlf_ext_transient_seat_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	ext_transient_seat_manager_v1_destroy(manager->base);
	free(manager);
}

struct wlf_ext_transient_seat_v1 *
wlf_ext_transient_seat_manager_v1_create_seat(
	struct wlf_ext_transient_seat_manager_v1 *manager)
{
	assert(manager);
	assert(manager->base);

	struct wlf_ext_transient_seat_v1 *seat = calloc(1, sizeof(*seat));
	if (!seat) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_ext_transient_seat_v1");
		return NULL;
	}

	seat->base = ext_transient_seat_manager_v1_create(manager->base);
	if (!seat->base) {
		wlf_log(WLF_ERROR,
			"ext_transient_seat_manager_v1_create() returned NULL");
		free(seat);
		return NULL;
	}

	wlf_signal_init(&seat->events.ready);
	wlf_signal_init(&seat->events.denied);
	wlf_signal_init(&seat->events.destroy);

	ext_transient_seat_v1_add_listener(seat->base,
		&transient_seat_listener, seat);

	return seat;
}

void wlf_ext_transient_seat_v1_destroy(struct wlf_ext_transient_seat_v1 *seat)
{
	if (!seat) {
		return;
	}

	wlf_signal_emit_mutable(&seat->events.destroy, seat);
	ext_transient_seat_v1_destroy(seat->base);
	free(seat);
}
