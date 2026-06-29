/**
 * @file        wlf_zwp_pointer_constraints_v1.c
 * @brief       Wayland pointer-constraints-unstable-v1 wrapper for wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_zwp_pointer_constraints_v1.h"
#include "wayland/protocols/pointer-constraints-unstable-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

/* -------------------------------------------------------------------------
 * Locked pointer listeners
 * ---------------------------------------------------------------------- */

static void locked_handle_locked(void *data,
	struct zwp_locked_pointer_v1 *base)
{
	struct wlf_zwp_locked_pointer_v1 *locked = data;
	(void)base;
	wlf_signal_emit_mutable(&locked->events.locked, locked);
}

static void locked_handle_unlocked(void *data,
	struct zwp_locked_pointer_v1 *base)
{
	struct wlf_zwp_locked_pointer_v1 *locked = data;
	(void)base;
	wlf_signal_emit_mutable(&locked->events.unlocked, locked);
}

static const struct zwp_locked_pointer_v1_listener locked_listener = {
	.locked   = locked_handle_locked,
	.unlocked = locked_handle_unlocked,
};

/* -------------------------------------------------------------------------
 * Confined pointer listeners
 * ---------------------------------------------------------------------- */

static void confined_handle_confined(void *data,
	struct zwp_confined_pointer_v1 *base)
{
	struct wlf_zwp_confined_pointer_v1 *confined = data;
	(void)base;
	wlf_signal_emit_mutable(&confined->events.confined, confined);
}

static void confined_handle_unconfined(void *data,
	struct zwp_confined_pointer_v1 *base)
{
	struct wlf_zwp_confined_pointer_v1 *confined = data;
	(void)base;
	wlf_signal_emit_mutable(&confined->events.unconfined, confined);
}

static const struct zwp_confined_pointer_v1_listener confined_listener = {
	.confined   = confined_handle_confined,
	.unconfined = confined_handle_unconfined,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_zwp_pointer_constraints_v1 *
wlf_zwp_pointer_constraints_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver =
		(uint32_t)zwp_pointer_constraints_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zwp_pointer_constraints_v1 *constraints =
		calloc(1, sizeof(*constraints));
	if (!constraints) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_pointer_constraints_v1");
		return NULL;
	}

	constraints->base = wl_registry_bind(wl_registry, name,
		&zwp_pointer_constraints_v1_interface, bind_ver);
	if (!constraints->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"zwp_pointer_constraints_v1 (name: %u)", name);
		free(constraints);
		return NULL;
	}

	wlf_signal_init(&constraints->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound zwp_pointer_constraints_v1 (name: %u, version: %u)",
		name, bind_ver);

	return constraints;
}

void wlf_zwp_pointer_constraints_v1_destroy(
	struct wlf_zwp_pointer_constraints_v1 *constraints)
{
	if (!constraints) {
		return;
	}

	wlf_signal_emit_mutable(&constraints->events.destroy, constraints);
	zwp_pointer_constraints_v1_destroy(constraints->base);
	free(constraints);
}

struct wlf_zwp_locked_pointer_v1 *
wlf_zwp_pointer_constraints_v1_lock_pointer(
	struct wlf_zwp_pointer_constraints_v1 *constraints,
	struct wl_surface *surface, struct wl_pointer *pointer,
	struct wl_region *region, uint32_t lifetime)
{
	assert(constraints);
	assert(constraints->base);
	assert(surface);
	assert(pointer);

	struct wlf_zwp_locked_pointer_v1 *locked = calloc(1, sizeof(*locked));
	if (!locked) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_locked_pointer_v1");
		return NULL;
	}

	locked->base = zwp_pointer_constraints_v1_lock_pointer(
		constraints->base, surface, pointer, region, lifetime);
	if (!locked->base) {
		wlf_log(WLF_ERROR,
			"zwp_pointer_constraints_v1_lock_pointer() returned NULL");
		free(locked);
		return NULL;
	}

	wlf_signal_init(&locked->events.locked);
	wlf_signal_init(&locked->events.unlocked);
	wlf_signal_init(&locked->events.destroy);

	zwp_locked_pointer_v1_add_listener(locked->base, &locked_listener,
		locked);

	return locked;
}

struct wlf_zwp_confined_pointer_v1 *
wlf_zwp_pointer_constraints_v1_confine_pointer(
	struct wlf_zwp_pointer_constraints_v1 *constraints,
	struct wl_surface *surface, struct wl_pointer *pointer,
	struct wl_region *region, uint32_t lifetime)
{
	assert(constraints);
	assert(constraints->base);
	assert(surface);
	assert(pointer);

	struct wlf_zwp_confined_pointer_v1 *confined =
		calloc(1, sizeof(*confined));
	if (!confined) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwp_confined_pointer_v1");
		return NULL;
	}

	confined->base = zwp_pointer_constraints_v1_confine_pointer(
		constraints->base, surface, pointer, region, lifetime);
	if (!confined->base) {
		wlf_log(WLF_ERROR,
			"zwp_pointer_constraints_v1_confine_pointer() returned NULL");
		free(confined);
		return NULL;
	}

	wlf_signal_init(&confined->events.confined);
	wlf_signal_init(&confined->events.unconfined);
	wlf_signal_init(&confined->events.destroy);

	zwp_confined_pointer_v1_add_listener(confined->base, &confined_listener,
		confined);

	return confined;
}

void wlf_zwp_locked_pointer_v1_set_cursor_position_hint(
	struct wlf_zwp_locked_pointer_v1 *locked,
	double surface_x, double surface_y)
{
	assert(locked);
	assert(locked->base);
	zwp_locked_pointer_v1_set_cursor_position_hint(locked->base,
		wl_fixed_from_double(surface_x),
		wl_fixed_from_double(surface_y));
}

void wlf_zwp_locked_pointer_v1_set_region(
	struct wlf_zwp_locked_pointer_v1 *locked, struct wl_region *region)
{
	assert(locked);
	assert(locked->base);
	zwp_locked_pointer_v1_set_region(locked->base, region);
}

void wlf_zwp_locked_pointer_v1_destroy(
	struct wlf_zwp_locked_pointer_v1 *locked)
{
	if (!locked) {
		return;
	}

	wlf_signal_emit_mutable(&locked->events.destroy, locked);
	zwp_locked_pointer_v1_destroy(locked->base);
	free(locked);
}

void wlf_zwp_confined_pointer_v1_set_region(
	struct wlf_zwp_confined_pointer_v1 *confined, struct wl_region *region)
{
	assert(confined);
	assert(confined->base);
	zwp_confined_pointer_v1_set_region(confined->base, region);
}

void wlf_zwp_confined_pointer_v1_destroy(
	struct wlf_zwp_confined_pointer_v1 *confined)
{
	if (!confined) {
		return;
	}

	wlf_signal_emit_mutable(&confined->events.destroy, confined);
	zwp_confined_pointer_v1_destroy(confined->base);
	free(confined);
}
