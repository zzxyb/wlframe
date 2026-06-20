#include "wlf/wayland/wlf_zxdg_decoration_v1.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wayland/protocols/xdg-decoration-unstable-v1-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void toplevel_decoration_configure(void *data,
		struct zxdg_toplevel_decoration_v1 *zxdg_decoration,
		uint32_t mode) {
	WLF_UNUSED(zxdg_decoration);

	struct wlf_zxdg_toplevel_decoration_v1 *decoration = data;
	decoration->mode = (enum wlf_decoration_mode)mode;
	wlf_signal_emit_mutable(&decoration->events.configure, decoration);
}

static const struct zxdg_toplevel_decoration_v1_listener
		toplevel_decoration_listener = {
	.configure = toplevel_decoration_configure,
};

struct wlf_zxdg_decoration_manager_v1 *wlf_zxdg_decoration_manager_v1_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_zxdg_decoration_manager_v1 *manager =
		malloc(sizeof(struct wlf_zxdg_decoration_manager_v1));
	if (manager == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate wlf_zxdg_decoration_manager_v1");
		return NULL;
	}

	manager->base = NULL;

	uint32_t bind_version = version;
	if (version > (uint32_t)zxdg_decoration_manager_v1_interface.version) {
		wlf_log(WLF_DEBUG,
			"Server zxdg_decoration_manager_v1 version %u is higher than "
			"client version %u, using client version",
			version,
			(uint32_t)zxdg_decoration_manager_v1_interface.version);
		bind_version = (uint32_t)zxdg_decoration_manager_v1_interface.version;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&zxdg_decoration_manager_v1_interface, bind_version);
	if (manager->base == NULL) {
		wlf_log(WLF_ERROR,
			"Failed to bind zxdg_decoration_manager_v1 interface (name: %u)",
			name);
		free(manager);
		return NULL;
	}

	wlf_log(WLF_DEBUG,
		"Successfully bound zxdg_decoration_manager_v1 "
		"(name: %u, version: %u)",
		name, bind_version);
	manager->version = bind_version;
	wlf_signal_init(&manager->events.destroy);

	return manager;
}

void wlf_zxdg_decoration_manager_v1_destroy(
		struct wlf_zxdg_decoration_manager_v1 *manager) {
	if (manager == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	assert(wlf_linked_list_empty(&manager->events.destroy.listener_list));

	if (manager->base != NULL) {
		zxdg_decoration_manager_v1_destroy(manager->base);
		manager->base = NULL;
	}

	free(manager);
}

struct wlf_zxdg_toplevel_decoration_v1 *
wlf_zxdg_decoration_manager_v1_get_toplevel_decoration(
		struct wlf_zxdg_decoration_manager_v1 *manager,
		struct xdg_toplevel *toplevel) {
	assert(manager->base != NULL);
	assert(toplevel != NULL);

	struct wlf_zxdg_toplevel_decoration_v1 *decoration =
		malloc(sizeof(struct wlf_zxdg_toplevel_decoration_v1));
	if (decoration == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate wlf_zxdg_toplevel_decoration_v1");
		return NULL;
	}

	decoration->base = NULL;
	decoration->mode = 0;

	decoration->base =
		zxdg_decoration_manager_v1_get_toplevel_decoration(
			manager->base, toplevel);
	if (decoration->base == NULL) {
		wlf_log(WLF_ERROR,
			"zxdg_decoration_manager_v1_get_toplevel_decoration() "
			"returned NULL");
		free(decoration);
		return NULL;
	}

	decoration->version =
		wl_proxy_get_version((struct wl_proxy *)decoration->base);
	zxdg_toplevel_decoration_v1_add_listener(decoration->base,
		&toplevel_decoration_listener, decoration);
	wlf_signal_init(&decoration->events.configure);
	wlf_signal_init(&decoration->events.destroy);

	return decoration;
}

void wlf_zxdg_toplevel_decoration_v1_set_mode(
		struct wlf_zxdg_toplevel_decoration_v1 *decoration,
		enum wlf_decoration_mode mode) {
	assert(decoration->base != NULL);

	zxdg_toplevel_decoration_v1_set_mode(decoration->base, (uint32_t)mode);
}

void wlf_zxdg_toplevel_decoration_v1_unset_mode(
		struct wlf_zxdg_toplevel_decoration_v1 *decoration) {
	assert(decoration->base != NULL);

	zxdg_toplevel_decoration_v1_unset_mode(decoration->base);
}

void wlf_zxdg_toplevel_decoration_v1_destroy(
		struct wlf_zxdg_toplevel_decoration_v1 *decoration) {
	if (decoration == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&decoration->events.destroy, decoration);
	assert(wlf_linked_list_empty(&decoration->events.destroy.listener_list));

	if (decoration->base != NULL) {
		zxdg_toplevel_decoration_v1_destroy(decoration->base);
		decoration->base = NULL;
	}

	free(decoration);
}
