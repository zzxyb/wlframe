/**
 * @file        wlf_zwp_relative_pointer_manager_v1.c
 * @brief       Wayland relative-pointer-unstable-v1 wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_zwp_relative_pointer_manager_v1.h"
#include "wayland/protocols/relative-pointer-unstable-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

/* -------------------------------------------------------------------------
 * Relative-pointer listeners
 * ---------------------------------------------------------------------- */

static void relative_pointer_handle_relative_motion(void *data,
	struct zwp_relative_pointer_v1 *base,
	uint32_t utime_hi, uint32_t utime_lo,
	wl_fixed_t dx, wl_fixed_t dy,
	wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel)
{
	struct wlf_zwp_relative_pointer_v1 *rp = data;
	(void)base;
	rp->current.utime_hi   = utime_hi;
	rp->current.utime_lo   = utime_lo;
	rp->current.dx         = wl_fixed_to_double(dx);
	rp->current.dy         = wl_fixed_to_double(dy);
	rp->current.dx_unaccel = wl_fixed_to_double(dx_unaccel);
	rp->current.dy_unaccel = wl_fixed_to_double(dy_unaccel);
	wlf_signal_emit_mutable(&rp->events.relative_motion, rp);
}

static const struct zwp_relative_pointer_v1_listener relative_pointer_listener = {
	.relative_motion = relative_pointer_handle_relative_motion,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_zwp_relative_pointer_manager_v1 *
wlf_zwp_relative_pointer_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver =
		(uint32_t)zwp_relative_pointer_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zwp_relative_pointer_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_relative_pointer_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&zwp_relative_pointer_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"zwp_relative_pointer_manager_v1 (name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound zwp_relative_pointer_manager_v1 (name: %u, version: %u)",
		name, bind_ver);

	return manager;
}

void wlf_zwp_relative_pointer_manager_v1_destroy(
	struct wlf_zwp_relative_pointer_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	zwp_relative_pointer_manager_v1_destroy(manager->base);
	free(manager);
}

struct wlf_zwp_relative_pointer_v1 *
wlf_zwp_relative_pointer_manager_v1_get_relative_pointer(
	struct wlf_zwp_relative_pointer_manager_v1 *manager,
	struct wl_pointer *pointer)
{
	assert(manager);
	assert(manager->base);
	assert(pointer);

	struct wlf_zwp_relative_pointer_v1 *rp = calloc(1, sizeof(*rp));
	if (!rp) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_relative_pointer_v1");
		return NULL;
	}

	rp->base = zwp_relative_pointer_manager_v1_get_relative_pointer(
		manager->base, pointer);
	if (!rp->base) {
		wlf_log(WLF_ERROR,
			"zwp_relative_pointer_manager_v1_get_relative_pointer()"
			" returned NULL");
		free(rp);
		return NULL;
	}

	wlf_signal_init(&rp->events.relative_motion);
	wlf_signal_init(&rp->events.destroy);

	zwp_relative_pointer_v1_add_listener(rp->base,
		&relative_pointer_listener, rp);

	return rp;
}

void wlf_zwp_relative_pointer_v1_destroy(
	struct wlf_zwp_relative_pointer_v1 *rp)
{
	if (!rp) {
		return;
	}

	wlf_signal_emit_mutable(&rp->events.destroy, rp);
	zwp_relative_pointer_v1_destroy(rp->base);
	free(rp);
}
