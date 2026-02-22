#include "wlf/platform/wayland/backend.h"
#include "wlf/types/wlf_output.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/utils/wlf_log.h"
#include "protocols/xdg-output-unstable-v1-client-protocol.h"
#include "wlf/wayland/wlf_wl_display.h"
#include "wlf/wayland/wlf_wl_output.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void wayland_compositor_destroy(struct wlf_listener *listener, void *data) {
	struct wlf_backend_wayland *backend =
		wlf_container_of(listener, backend, listeners.display_destroy);
	wlf_wl_compositor_destroy(backend->compositor);
	backend->compositor = NULL;
}

static void zxdg_output_manager_destory(struct wlf_listener *listener, void *data) {
	struct wlf_backend_wayland *backend =
		wlf_container_of(listener, backend, listeners.output_manager_destroy);
	wlf_output_manager_destroy(backend->base.output_manager);
	backend->base.output_manager = NULL;
}

static bool wayland_backend_start(struct wlf_backend *backend) {
	struct wlf_backend_wayland *wayland = (struct wlf_backend_wayland *)backend;

	if (wayland->started) {
		wlf_log(WLF_INFO, "Wayland backend already started");
		return true;
	}

	if (wayland->display == NULL) {
		wlf_log(WLF_ERROR, "No Wayland display connection");
		return false;
	}

	if (!wlf_wl_display_init_registry(wayland->display)) {
		wlf_log(WLF_ERROR, "Failed to initialize Wayland registry");
		return false;
	}

	struct wlf_wl_interface *compositor_interface =
		wlf_wl_display_find_interface(wayland->display, wl_compositor_interface.name);
	if (compositor_interface == NULL) {
		wlf_log(WLF_ERROR, "Failed to find compositor interface in registry");
		return false;
	}

	wayland->compositor = wlf_wl_compositor_create(
		wayland->display->registry,
		compositor_interface->name,
		compositor_interface->version
	);
	if (wayland->compositor == NULL) {
		wlf_log(WLF_ERROR, "Failed to create Wayland compositor interface");
		return false;
	}

	wayland->listeners.compositor_destroy.notify = wayland_compositor_destroy;
	wlf_signal_add(&compositor_interface->events.destroy, &wayland->listeners.compositor_destroy);

	struct wlf_wl_interface *output_manager_interface =
		wlf_wl_display_find_interface(wayland->display, zxdg_output_manager_v1_interface.name);
	if (output_manager_interface == NULL) {
		wlf_log(WLF_ERROR, "Failed to find zxdg_output_manager_v1 interface in registry");
		return false;
	}

	backend->output_manager = wlf_output_manager_create_from_wl_registry(
		wayland->display->registry,
		output_manager_interface->name,
		output_manager_interface->version
	);
	if (backend->output_manager == NULL) {
		wlf_log(WLF_ERROR, "Failed to create Wayland zxdg_output_manager_v1 interface");
		return false;
	}

	wayland->listeners.output_manager_destroy.notify = zxdg_output_manager_destory;
	wlf_signal_add(&output_manager_interface->events.destroy, &wayland->listeners.output_manager_destroy);

	struct wlf_wl_interface *reg, *tmp;
	wlf_linked_list_for_each_safe(reg, tmp, &wayland->display->interfaces, link) {
		if (reg->interface == wl_output_interface.name) {
			struct wlf_output *output = wlf_output_create_from_wl_registry(
				wayland->display->registry,
				reg->name,
				reg->version
			);
			struct wlf_wl_output *wl_output = wlf_wl_output_from_backend(output);
			struct wlf_wl_output_manager *wl_manager = wlf_wl_output_manager_from_backend(backend->output_manager);
			wl_output->xdg_output = zxdg_output_manager_v1_get_xdg_output(wl_manager->manager, wl_output->output);
			wlf_linked_list_insert(&backend->output_manager->outputs, &output->link);
			wlf_signal_emit_mutable(&backend->output_manager->events.output_added, output);
		}
	}
	wayland->started = true;
	wlf_log(WLF_INFO, "Wayland backend started successfully");

	return true;
}

static void wayland_backend_stop(struct wlf_backend *backend) {
	struct wlf_backend_wayland *wayland = (struct wlf_backend_wayland *)backend;

	if (!wayland->started) {
		return;
	}

	wlf_log(WLF_DEBUG, "Stopping Wayland backend");

	if (wayland->compositor) {
		wlf_wl_compositor_destroy(wayland->compositor);
		wayland->compositor = NULL;
	}

	wayland->started = false;
}

static void wayland_backend_destroy(struct wlf_backend *backend) {
	struct wlf_backend_wayland *wayland = (struct wlf_backend_wayland *)backend;
	wlf_log(WLF_DEBUG, "Destroying Wayland backend");

	wayland_backend_stop(backend);
	wlf_linked_list_remove(&wayland->listeners.display_destroy.link);
	wlf_linked_list_remove(&wayland->listeners.compositor_destroy.link);
	wlf_linked_list_remove(&wayland->listeners.output_manager_destroy.link);

	if (wayland->display) {
		wlf_wl_display_destroy(wayland->display);
		wayland->display = NULL;
	}

	free(wayland);
}

static const struct wlf_backend_impl wayland_backend_impl = {
	.start = wayland_backend_start,
	.stop = wayland_backend_stop,
	.destroy = wayland_backend_destroy,
};

static void wayland_display_destroy(struct wlf_listener *listener, void *data) {
	struct wlf_backend_wayland *backend =
		wlf_container_of(listener, backend, listeners.display_destroy);
	wlf_log(WLF_INFO, "Wayland display destroyed");
	wayland_backend_stop(&backend->base);
}

static struct wlf_backend *wayland_backend_create(void *args) {
	struct wlf_backend_wayland *backend = calloc(1, sizeof(struct wlf_backend_wayland));
	if (backend == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate Wayland backend");
		return NULL;
	}

	backend->base.impl = &wayland_backend_impl;
	backend->base.type = WLF_BACKEND_WAYLAND;
	backend->base.data = backend;
	wlf_signal_init(&backend->base.events.destroy);

	const struct wlf_backend_create_args *create_args =
		(const struct wlf_backend_create_args *)args;

	if (create_args && create_args->wayland.display) {
		backend->display = create_args->wayland.display;
		wlf_log(WLF_DEBUG, "Using provided Wayland display");
	} else {
		backend->display = wlf_wl_display_create();
		if (backend->display == NULL) {
			wlf_log(WLF_ERROR, "Failed to connect to Wayland display");
			free(backend);
			return NULL;
		}
		wlf_log(WLF_DEBUG, "Created new Wayland display connection");
	}

	backend->listeners.display_destroy.notify = wayland_display_destroy;
	wlf_signal_add(&backend->display->events.destroy, &backend->listeners.display_destroy);

	backend->started = false;

	wlf_log(WLF_INFO, "Created Wayland backend");
	return &backend->base;
}

static bool wayland_backend_is_available(void) {
	const char *wayland_display = getenv("WAYLAND_DISPLAY");
	if (wayland_display == NULL) {
		wlf_log(WLF_ERROR, "WAYLAND_DISPLAY environment variable is not set");
		wayland_display = "wayland-0";
	}

	char socket_path[256];
	const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
	if (!runtime_dir) {
		wlf_log(WLF_ERROR, "XDG_RUNTIME_DIR environment variable is not set");
		return false;
	}

	snprintf(socket_path, sizeof(socket_path), "%s/%s", runtime_dir, wayland_display);
	return access(socket_path, F_OK) == 0;
}

static struct wlf_backend_registry_entry entry = {
	.type = WLF_BACKEND_WAYLAND,
	.name = "wayland",
	.priority = 100,
	.create = wayland_backend_create,
	.is_available = wayland_backend_is_available,
	.handle = NULL,
};

bool wlf_backend_wayland_register(void) {
	return wlf_backend_register(&entry);
}

bool wlf_backend_is_wayland(const struct wlf_backend *backend) {
	return (backend && backend->impl == &wayland_backend_impl &&
			backend->type == WLF_BACKEND_WAYLAND);
}

struct wlf_backend_wayland *wlf_backend_wayland_from_backend(struct wlf_backend *wlf_backend) {
	assert(wlf_backend && wlf_backend->impl == &wayland_backend_impl);
	struct wlf_backend_wayland *backend = wlf_container_of(wlf_backend, backend, base);

	return backend;
}
