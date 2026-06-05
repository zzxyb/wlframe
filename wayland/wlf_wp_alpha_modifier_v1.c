#include "wlf/wayland/wlf_wp_alpha_modifier_v1.h"
#include "wayland/protocols/alpha-modifier-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct wlf_wp_alpha_modifier_v1 *wlf_wp_alpha_modifier_v1_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry);

	uint32_t bind_version = wp_alpha_modifier_v1_interface.version;
	if (version < bind_version) {
		bind_version = version;
		wlf_log(WLF_DEBUG,
			"wp_alpha_modifier_v1: binding version %u "
			"(server advertised %u, interface supports %u)",
			bind_version, version,
			wp_alpha_modifier_v1_interface.version);
	}

	struct wlf_wp_alpha_modifier_v1 *manager = calloc(1, sizeof(*manager));
	if (manager == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_wp_alpha_modifier_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&wp_alpha_modifier_v1_interface, bind_version);
	if (manager->base == NULL) {
		wlf_log(WLF_ERROR, "wl_registry_bind failed for wp_alpha_modifier_v1");
		free(manager);
		return NULL;
	}

	manager->version = bind_version;
	wlf_signal_init(&manager->events.destroy);
	return manager;
}

void wlf_wp_alpha_modifier_v1_destroy(struct wlf_wp_alpha_modifier_v1 *manager) {
	if (manager == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	assert(wlf_linked_list_empty(&manager->events.destroy.listener_list));

	if (manager->base != NULL) {
		wp_alpha_modifier_v1_destroy(manager->base);
	}

	free(manager);
}

struct wlf_wp_alpha_modifier_surface_v1 *wlf_wp_alpha_modifier_v1_get_surface(
		struct wlf_wp_alpha_modifier_v1 *manager, struct wl_surface *surface) {
	assert(manager);
	assert(surface);

	struct wlf_wp_alpha_modifier_surface_v1 *surf = calloc(1, sizeof(*surf));
	if (surf == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_wp_alpha_modifier_surface_v1");
		return NULL;
	}

	surf->base = wp_alpha_modifier_v1_get_surface(manager->base, surface);
	if (surf->base == NULL) {
		wlf_log(WLF_ERROR, "wp_alpha_modifier_v1_get_surface failed");
		free(surf);
		return NULL;
	}

	wlf_signal_init(&surf->events.destroy);
	surf->multiplier = UINT32_MAX;
	surf->version = wl_proxy_get_version((struct wl_proxy *)surf->base);

	return surf;
}

static uint32_t alpha_to_multiplier(float alpha) {
	if (!(alpha >= 0.0f)) {
		return 0;
	}
	if (alpha >= 1.0f) {
		return UINT32_MAX;
	}

	return (uint32_t)((double)UINT32_MAX * alpha + 0.5);
}

void wlf_wp_alpha_modifier_surface_v1_set_multiplier(
		struct wlf_wp_alpha_modifier_surface_v1 *surface, float alpha) {
	assert(surface);
	assert(surface->base);

	uint32_t multiplier = alpha_to_multiplier(alpha);
	if (surface->multiplier == multiplier) {
		return;
	}

	surface->multiplier = multiplier;
	wp_alpha_modifier_surface_v1_set_multiplier(surface->base, multiplier);
}

void wlf_wp_alpha_modifier_surface_v1_destroy(
		struct wlf_wp_alpha_modifier_surface_v1 *surface) {
	if (surface == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&surface->events.destroy, surface);
	assert(wlf_linked_list_empty(&surface->events.destroy.listener_list));

	if (surface->base != NULL) {
		wp_alpha_modifier_surface_v1_destroy(surface->base);
	}

	free(surface);
}
