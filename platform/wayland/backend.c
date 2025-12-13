#include "wlf/platform/wayland/backend.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/types/wlf_output.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/wayland/wlf_wl_output.h"
#include "wlf/wayland/wlf_wl_interface.h"
#include "protocols/xdg-output-unstable-v1-client-protocol.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

static void display_global_added(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version) {
	wlf_log(WLF_DEBUG, "Wayland registry global: %s v%" PRIu32, interface, version);
	struct wlf_backend_wayland *wayland = data;

	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		uint32_t bind_version = version;
		if (version > (uint32_t)wl_compositor_interface.version) {
			wlf_log(WLF_DEBUG, "Server compositor version %u is higher than client version %u, "
					"using client version", version, (uint32_t)wl_compositor_interface.version);
			bind_version = (uint32_t)wl_compositor_interface.version;
		}

		wayland->compositor = wl_registry_bind(wl_registry,
			name,
			&wl_compositor_interface,
			bind_version);
		if (wayland->compositor == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind wl_compositor interface with name %u", name);
			return;
		} else {
			wlf_log(WLF_DEBUG, "Successfully bound wl_compositor interface (name: %u, version: %u)",
					name, bind_version);
		}
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		uint32_t bind_version = version;
		if (version > (uint32_t)wl_shm_interface.version) {
			wlf_log(WLF_DEBUG, "Server shm version %u is higher than client version %u, "
					"using client version", version, (uint32_t)wl_shm_interface.version);
			bind_version = (uint32_t)wl_shm_interface.version;
		}

		wayland->shm = wl_registry_bind(wl_registry,
			name,
			&wl_shm_interface,
			bind_version);
		if (wayland->shm == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind wl_shm interface with name %u", name);
			return;
		} else {
			wlf_log(WLF_DEBUG, "Successfully bound wl_shm interface (name: %u, version: %u)",
					name, bind_version);
		}
	}

	struct wlf_wl_interface *new_reg = wlf_wl_interface_create(interface, version, name);
	if (new_reg == NULL) {
		return;
	}

	wlf_linked_list_insert(&wayland->interfaces, &new_reg->link);
	wlf_signal_emit_mutable(&wayland->events.global_add, new_reg);
}

static void display_global_remove(void *data,
		struct wl_registry *wl_registry, uint32_t name) {
	struct wlf_backend_wayland *wayland = data;
	struct wlf_wl_interface *reg, *tmp;

	wlf_linked_list_for_each_safe(reg, tmp, &wayland->interfaces, link) {
		if (reg->name == name) {
			wlf_log(WLF_DEBUG, "Interface %s removed", reg->interface);
			wlf_signal_emit_mutable(&wayland->events.global_remove, reg);
			wlf_wl_interface_destroy(reg);
			break;
		}
	}
}

static const struct wl_registry_listener wl_base_registry_listener = {
	.global = display_global_added,
	.global_remove = display_global_remove,
};

static void wayland_backend_destroy(struct wlf_backend *backend) {
	wlf_log(WLF_DEBUG, "Destroying Wayland backend");

	struct wlf_backend_wayland *wayland = (struct wlf_backend_wayland *)backend;
	wlf_linked_list_remove(&wayland->interfaces);
	wlf_linked_list_remove(&wayland->listeners.compositor_destroy.link);
	wlf_linked_list_remove(&wayland->listeners.output_manager_destroy.link);

	if (wayland->display) {
		wl_display_disconnect(wayland->display);
	}

	free(wayland);
}

static const struct wlf_backend_impl wayland_backend_impl = {
	.destroy = wayland_backend_destroy,
};

struct wlf_backend *wayland_backend_create(void) {
	struct wlf_backend_wayland *backend = calloc(1, sizeof(struct wlf_backend_wayland));
	if (backend == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate Wayland backend");
		return NULL;
	}

	wlf_backend_init(&backend->base, &wayland_backend_impl);
	wlf_linked_list_init(&backend->interfaces);
	backend->display = wl_display_connect(NULL);
	if (backend->display == NULL) {
		wlf_log(WLF_ERROR, "No Wayland display connection");
		goto failed;
	}

	backend->registry = wl_display_get_registry(backend->display);
	if (backend->registry == NULL) {
		wlf_log(WLF_ERROR, "Failed to get Wayland registry");
		goto failed;
	}

	wl_registry_add_listener(backend->registry, &wl_base_registry_listener, backend);
	wl_display_roundtrip(backend->display);

	wlf_log(WLF_DEBUG, "Created Wayland backend");

	return &backend->base;

failed:
	wlf_backend_destroy(&backend->base);

	return NULL;
}

bool wlf_backend_is_wayland(const struct wlf_backend *backend) {
	return (backend && backend->impl == &wayland_backend_impl);
}

struct wlf_backend_wayland *wlf_backend_wayland_from_backend(struct wlf_backend *wlf_backend) {
	assert(wlf_backend && wlf_backend->impl == &wayland_backend_impl);

	struct wlf_backend_wayland *backend = wlf_container_of(wlf_backend, backend, base);

	return backend;
}
