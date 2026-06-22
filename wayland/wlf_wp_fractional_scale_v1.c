#include "wlf/wayland/wlf_wp_fractional_scale_v1.h"

#include "wlf/utils/wlf_log.h"
#include "wayland/protocols/fractional-scale-v1-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

static void fractional_scale_handle_preferred_scale(void *data,
		struct wp_fractional_scale_v1 *wp_fractional_scale_v1,
		uint32_t scale) {
	(void)wp_fractional_scale_v1;

	struct wlf_wp_fractional_scale_v1 *fractional_scale = data;
	fractional_scale->preferred_scale_double =
		wlf_wp_fractional_scale_v1_to_double(scale);

	wlf_signal_emit_mutable(&fractional_scale->events.preferred_scale,
		fractional_scale);
}

static const struct wp_fractional_scale_v1_listener fractional_scale_listener = {
	.preferred_scale = fractional_scale_handle_preferred_scale,
};

struct wlf_wp_fractional_scale_manager_v1 *
wlf_wp_fractional_scale_manager_v1_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wp_fractional_scale_manager_v1 *manager =
		malloc(sizeof(struct wlf_wp_fractional_scale_manager_v1));
	if (manager == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wp_fractional_scale_manager_v1");
		return NULL;
	}

	manager->base = NULL;
	wlf_signal_init(&manager->events.destroy);

	uint32_t bind_version = version;
	if (version > (uint32_t)wp_fractional_scale_manager_v1_interface.version) {
		wlf_log(WLF_DEBUG,
			"Server wp_fractional_scale_manager_v1 version %u is higher "
			"than client version %u, using client version",
			version,
			(uint32_t)wp_fractional_scale_manager_v1_interface.version);
		bind_version =
			(uint32_t)wp_fractional_scale_manager_v1_interface.version;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&wp_fractional_scale_manager_v1_interface, bind_version);
	if (manager->base == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind wp_fractional_scale_manager_v1 interface (name: %u)", name);
		free(manager);
		return NULL;
	}

	manager->version = bind_version;
	wlf_log(WLF_DEBUG, "Successfully bound wp_fractional_scale_manager_v1 (name: %u, version: %u)", name, bind_version);

	return manager;
}

void wlf_wp_fractional_scale_manager_v1_destroy(
		struct wlf_wp_fractional_scale_manager_v1 *manager) {
	if (manager == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	assert(wlf_linked_list_empty(&manager->events.destroy.listener_list));

	if (manager->base != NULL) {
		wp_fractional_scale_manager_v1_destroy(manager->base);
	}

	free(manager);
}

struct wlf_wp_fractional_scale_v1 *
wlf_wp_fractional_scale_manager_v1_get_fractional_scale(
		struct wlf_wp_fractional_scale_manager_v1 *manager,
		struct wl_surface *surface) {
	assert(manager != NULL);
	assert(manager->base != NULL);
	assert(surface != NULL);

	struct wlf_wp_fractional_scale_v1 *fractional_scale =
		malloc(sizeof(struct wlf_wp_fractional_scale_v1));
	if (fractional_scale == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wp_fractional_scale_v1");
		return NULL;
	}

	fractional_scale->base = NULL;
	fractional_scale->version = 0;
	fractional_scale->preferred_scale_double = 0.0;
	wlf_signal_init(&fractional_scale->events.preferred_scale);
	wlf_signal_init(&fractional_scale->events.destroy);

	fractional_scale->base =
		wp_fractional_scale_manager_v1_get_fractional_scale(
			manager->base, surface);
	if (fractional_scale->base == NULL) {
		wlf_log(WLF_ERROR, "wp_fractional_scale_manager_v1_get_fractional_scale() returned NULL");
		free(fractional_scale);
		return NULL;
	}

	fractional_scale->version =
		wl_proxy_get_version((struct wl_proxy *)fractional_scale->base);
	wp_fractional_scale_v1_add_listener(fractional_scale->base,
		&fractional_scale_listener, fractional_scale);

	return fractional_scale;
}

double wlf_wp_fractional_scale_v1_to_double(uint32_t preferred_scale) {
	return (double)preferred_scale /
		(double)WLF_WP_FRACTIONAL_SCALE_V1_DENOMINATOR;
}

void wlf_wp_fractional_scale_v1_destroy(
		struct wlf_wp_fractional_scale_v1 *fractional_scale) {
	if (fractional_scale == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&fractional_scale->events.destroy,
		fractional_scale);
	assert(wlf_linked_list_empty(
		&fractional_scale->events.destroy.listener_list));
	assert(wlf_linked_list_empty(
		&fractional_scale->events.preferred_scale.listener_list));

	if (fractional_scale->base != NULL) {
		wp_fractional_scale_v1_destroy(fractional_scale->base);
	}

	free(fractional_scale);
}
