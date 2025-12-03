#include "wlf/platform/wayland/backend.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void handle_wl_compositor_destroy(struct wlf_listener *listener, void *data) {
	struct wlf_backend_wayland *backend =
		wlf_container_of(listener, backend, listeners.display_destroy);
	wlf_wl_compositor_destroy(backend->compositor);
	backend->compositor = NULL;
}

static bool handle_wayland_backend_start(struct wlf_backend *backend) {
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

	wayland->listeners.compositor_destroy.notify = handle_wl_compositor_destroy;
	wlf_signal_add(&compositor_interface->events.destroy, &wayland->listeners.compositor_destroy);

	wayland->compositor = wlf_wl_compositor_create(
		wayland->display->registry,
		compositor_interface->name,
		compositor_interface->version
	);
	if (wayland->compositor == NULL) {
		wlf_log(WLF_ERROR, "Failed to create Wayland compositor interface");
		return false;
	}

	wayland->started = true;
	wlf_log(WLF_INFO, "Wayland backend started successfully");

	return true;
}

static void handle_wayland_backend_stop(struct wlf_backend *backend) {
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

static void handle_wayland_backend_destroy(struct wlf_backend *backend) {
	struct wlf_backend_wayland *wayland = (struct wlf_backend_wayland *)backend;
	wlf_log(WLF_DEBUG, "Destroying Wayland backend");

	handle_wayland_backend_stop(backend);
	wlf_linked_list_remove(&wayland->listeners.display_destroy.link);
	wlf_linked_list_remove(&wayland->listeners.compositor_destroy.link);

	if (wayland->display) {
		wlf_wl_display_destroy(wayland->display);
		wayland->display = NULL;
	}

	free(wayland);
}

static const struct wlf_backend_impl wayland_backend_impl = {
	.start = handle_wayland_backend_start,
	.stop = handle_wayland_backend_stop,
	.destroy = handle_wayland_backend_destroy,
};

static void handle_display_destroy(struct wlf_listener *listener, void *data) {
	struct wlf_backend_wayland *backend =
		wlf_container_of(listener, backend, listeners.display_destroy);
	wlf_log(WLF_INFO, "Wayland display destroyed");
	handle_wayland_backend_stop(&backend->base);
}

static struct wlf_backend *handle_backend_create(void *args) {
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

	backend->listeners.display_destroy.notify = handle_display_destroy;
	wlf_signal_add(&backend->display->events.destroy, &backend->listeners.display_destroy);

	backend->started = false;

	wlf_log(WLF_INFO, "Created Wayland backend");
	return &backend->base;
}

static bool handle_backend_is_available(void) {
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
	.create = handle_backend_create,
	.is_available = handle_backend_is_available,
	.handle = NULL,
};

bool wlf_backend_wayland_register(void) {
	return wlf_backend_register(&entry);
}

bool wlf_backend_is_wayland(struct wlf_backend *backend) {
	return (backend && backend->impl == &wayland_backend_impl &&
			backend->type == WLF_BACKEND_WAYLAND);
}

struct wlf_backend_wayland *wlf_backend_wayland_from_backend(struct wlf_backend *wlf_backend) {
	assert(wlf_backend && wlf_backend->impl == &wayland_backend_impl);
	struct wlf_backend_wayland *backend = wlf_container_of(wlf_backend, backend, base);

	return backend;
}
