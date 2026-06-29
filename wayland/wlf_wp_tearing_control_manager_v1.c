/**
 * @file        wlf_wp_tearing_control_manager_v1.c
 * @brief       Wayland wp_tearing_control_manager_v1 protocol wrapper for
 *              wlframe.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#include "wlf/wayland/wlf_wp_tearing_control_manager_v1.h"
#include "wayland/protocols/tearing-control-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct wlf_wp_tearing_control_manager_v1 *
wlf_wp_tearing_control_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version)
{
	assert(wl_registry);

	uint32_t bind_ver =
		(uint32_t)wp_tearing_control_manager_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_wp_tearing_control_manager_v1 *manager =
		calloc(1, sizeof(*manager));
	if (!manager) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_wp_tearing_control_manager_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&wp_tearing_control_manager_v1_interface, bind_ver);
	if (!manager->base) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for wp_tearing_control_manager_v1 "
			"(name: %u)", name);
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG,
		"bound wp_tearing_control_manager_v1 (name: %u, version: %u)",
		name, bind_ver);

	return manager;
}

void wlf_wp_tearing_control_manager_v1_destroy(
	struct wlf_wp_tearing_control_manager_v1 *manager)
{
	if (!manager) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	wp_tearing_control_manager_v1_destroy(manager->base);
	free(manager);
}

struct wlf_wp_tearing_control_v1 *
wlf_wp_tearing_control_manager_v1_get_tearing_control(
	struct wlf_wp_tearing_control_manager_v1 *manager,
	struct wl_surface *surface)
{
	assert(manager);
	assert(manager->base);
	assert(surface);

	struct wlf_wp_tearing_control_v1 *control =
		calloc(1, sizeof(*control));
	if (!control) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_wp_tearing_control_v1");
		return NULL;
	}

	control->base = wp_tearing_control_manager_v1_get_tearing_control(
		manager->base, surface);
	if (!control->base) {
		wlf_log(WLF_ERROR,
			"wp_tearing_control_manager_v1_get_tearing_control() returned NULL");
		free(control);
		return NULL;
	}

	wlf_signal_init(&control->events.destroy);
	return control;
}

void wlf_wp_tearing_control_v1_set_presentation_hint(
	struct wlf_wp_tearing_control_v1 *control,
	enum wlf_tearing_presentation_hint hint)
{
	assert(control);
	assert(control->base);
	wp_tearing_control_v1_set_presentation_hint(control->base,
		(uint32_t)hint);
}

void wlf_wp_tearing_control_v1_destroy(
	struct wlf_wp_tearing_control_v1 *control)
{
	if (!control) {
		return;
	}

	wlf_signal_emit_mutable(&control->events.destroy, control);
	wp_tearing_control_v1_destroy(control->base);
	free(control);
}
