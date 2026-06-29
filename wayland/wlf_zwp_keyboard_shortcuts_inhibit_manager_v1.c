/**
 * @file        wlf_zwp_keyboard_shortcuts_inhibit_manager_v1.c
 * @brief       Wayland keyboard-shortcuts-inhibit-unstable-v1 wrapper.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_zwp_keyboard_shortcuts_inhibit_manager_v1.h"
#include "wayland/protocols/keyboard-shortcuts-inhibit-unstable-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Inhibitor listeners
 * ---------------------------------------------------------------------- */

static void inhibitor_handle_active(void *data,
	struct zwp_keyboard_shortcuts_inhibitor_v1 *base)
{
	struct wlf_zwp_keyboard_shortcuts_inhibitor_v1 *inhibitor = data;
	(void)base;
	wlf_signal_emit_mutable(&inhibitor->events.active, inhibitor);
}

static void inhibitor_handle_inactive(void *data,
	struct zwp_keyboard_shortcuts_inhibitor_v1 *base)
{
	struct wlf_zwp_keyboard_shortcuts_inhibitor_v1 *inhibitor = data;
	(void)base;
	wlf_signal_emit_mutable(&inhibitor->events.inactive, inhibitor);
}

static const struct zwp_keyboard_shortcuts_inhibitor_v1_listener
inhibitor_listener = {
	.active   = inhibitor_handle_active,
	.inactive = inhibitor_handle_inactive,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_zwp_keyboard_shortcuts_inhibit_manager_v1 *
wlf_zwp_keyboard_shortcuts_inhibit_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver =
		(uint32_t)zwp_keyboard_shortcuts_inhibit_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zwp_keyboard_shortcuts_inhibit_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate "
			"wlf_zwp_keyboard_shortcuts_inhibit_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&zwp_keyboard_shortcuts_inhibit_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"zwp_keyboard_shortcuts_inhibit_manager_v1 (name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound zwp_keyboard_shortcuts_inhibit_manager_v1 "
		"(name: %u, version: %u)", name, bind_ver);

	return manager;
}

void wlf_zwp_keyboard_shortcuts_inhibit_manager_v1_destroy(
	struct wlf_zwp_keyboard_shortcuts_inhibit_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	zwp_keyboard_shortcuts_inhibit_manager_v1_destroy(manager->base);
	free(manager);
}

struct wlf_zwp_keyboard_shortcuts_inhibitor_v1 *
wlf_zwp_keyboard_shortcuts_inhibit_manager_v1_inhibit_shortcuts(
	struct wlf_zwp_keyboard_shortcuts_inhibit_manager_v1 *manager,
	struct wl_surface *surface, struct wl_seat *seat)
{
	assert(manager);
	assert(manager->base);
	assert(surface);
	assert(seat);

	struct wlf_zwp_keyboard_shortcuts_inhibitor_v1 *inhibitor =
		calloc(1, sizeof(*inhibitor));
	if (!inhibitor) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate "
			"wlf_zwp_keyboard_shortcuts_inhibitor_v1");
		return NULL;
	}

	inhibitor->base =
		zwp_keyboard_shortcuts_inhibit_manager_v1_inhibit_shortcuts(
			manager->base, surface, seat);
	if (!inhibitor->base) {
		wlf_log(WLF_ERROR,
			"zwp_keyboard_shortcuts_inhibit_manager_v1_inhibit_shortcuts"
			"() returned NULL");
		free(inhibitor);
		return NULL;
	}

	wlf_signal_init(&inhibitor->events.active);
	wlf_signal_init(&inhibitor->events.inactive);
	wlf_signal_init(&inhibitor->events.destroy);

	zwp_keyboard_shortcuts_inhibitor_v1_add_listener(inhibitor->base,
		&inhibitor_listener, inhibitor);

	return inhibitor;
}

void wlf_zwp_keyboard_shortcuts_inhibitor_v1_destroy(
	struct wlf_zwp_keyboard_shortcuts_inhibitor_v1 *inhibitor)
{
	if (!inhibitor) {
		return;
	}

	wlf_signal_emit_mutable(&inhibitor->events.destroy, inhibitor);
	zwp_keyboard_shortcuts_inhibitor_v1_destroy(inhibitor->base);
	free(inhibitor);
}
