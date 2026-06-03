#include "wlf/wayland/wlf_wp_content_type_manager_v1.h"
#include "wlf/utils/wlf_log.h"
#include "wayland/protocols/content-type-v1-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

struct wlf_wp_content_type_manager_v1 *wlf_wp_content_type_manager_v1_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wp_content_type_manager_v1 *manager =
		malloc(sizeof(struct wlf_wp_content_type_manager_v1));
	if (manager == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate wlf_wp_content_type_manager_v1");
		return NULL;
	}

	manager->base = NULL;
	wlf_signal_init(&manager->events.destroy);

	uint32_t bind_version = version;
	if (version > (uint32_t)wp_content_type_manager_v1_interface.version) {
		wlf_log(WLF_DEBUG,
			"Server wp_content_type_manager_v1 version %u is higher than "
			"client version %u, using client version",
			version,
			(uint32_t)wp_content_type_manager_v1_interface.version);
		bind_version = (uint32_t)wp_content_type_manager_v1_interface.version;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&wp_content_type_manager_v1_interface, bind_version);
	if (manager->base == NULL) {
		wlf_log(WLF_ERROR,
			"Failed to bind wp_content_type_manager_v1 interface (name: %u)",
			name);
		free(manager);
		return NULL;
	}

	manager->version = bind_version;
	wlf_log(WLF_DEBUG,
		"Successfully bound wp_content_type_manager_v1 "
		"(name: %u, version: %u)",
		name, bind_version);

	return manager;
}

void wlf_wp_content_type_manager_v1_destroy(struct wlf_wp_content_type_manager_v1 *manager) {
	if (manager == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	assert(wlf_linked_list_empty(&manager->events.destroy.listener_list));

	if (manager->base != NULL) {
		wp_content_type_manager_v1_destroy(manager->base);
		manager->base = NULL;
	}

	free(manager);
}

struct wlf_wp_content_type_v1 *wlf_wp_content_type_manager_v1_get_surface_content_type(
		struct wlf_wp_content_type_manager_v1 *manager, struct wl_surface *surface) {
	assert(manager != NULL);
	assert(manager->base != NULL);
	assert(surface != NULL);

	struct wlf_wp_content_type_v1 *content_type =
		malloc(sizeof(struct wlf_wp_content_type_v1));
	if (content_type == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate wlf_wp_content_type_v1");
		return NULL;
	}

	content_type->base = NULL;
	wlf_signal_init(&content_type->events.destroy);

	content_type->base = wp_content_type_manager_v1_get_surface_content_type(
		manager->base, surface);
	if (content_type->base == NULL) {
		wlf_log(WLF_ERROR,
			"wp_content_type_manager_v1_get_surface_content_type() "
			"returned NULL");
		free(content_type);
		return NULL;
	}

	content_type->version =
		wl_proxy_get_version((struct wl_proxy *)content_type->base);
	return content_type;
}

void wlf_wp_content_type_v1_set_content_type(
		struct wlf_wp_content_type_v1 *content_type,
		enum wlf_content_type type) {
	if (content_type->type == type) {
		return;
	}

	assert(content_type != NULL);
	assert(content_type->base != NULL);

	content_type->type = type;
	wp_content_type_v1_set_content_type(content_type->base,
		(uint32_t)type);
}

void wlf_wp_content_type_v1_destroy(struct wlf_wp_content_type_v1 *content_type) {
	if (content_type == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&content_type->events.destroy, content_type);
	assert(wlf_linked_list_empty(&content_type->events.destroy.listener_list));

	if (content_type->base != NULL) {
		wp_content_type_v1_destroy(content_type->base);
		content_type->base = NULL;
	}

	free(content_type);
}
