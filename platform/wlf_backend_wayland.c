#include "wlf/platform/wlf_backend_wayland.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <wayland-client-protocol.h>

// Forward declarations
static bool wayland_backend_start(struct wlf_backend *backend);
static void wayland_backend_stop(struct wlf_backend *backend);
static void wayland_backend_destroy(struct wlf_backend *backend);

static const struct wlf_backend_impl wayland_backend_impl = {
	.start = wayland_backend_start,
	.stop = wayland_backend_stop,
	.destroy = wayland_backend_destroy,
};

static void handle_display_destroy(struct wlf_listener *listener, void *data) {
	struct wlf_backend_wayland *backend =
		wlf_container_of(listener, backend, listeners.display_destroy);

	wlf_log(WLF_INFO, "Wayland display destroyed");

	// Stop the backend
	wayland_backend_stop(&backend->base);
}

static bool wayland_backend_start(struct wlf_backend *backend) {
	struct wlf_backend_wayland *wayland = (struct wlf_backend_wayland *)backend;

	if (wayland->started) {
		wlf_log(WLF_INFO, "Wayland backend already started");
		return true;
	}

	if (!wayland->display) {
		wlf_log(WLF_ERROR, "No Wayland display connection");
		return false;
	}

	// Initialize the registry to discover global interfaces
	if (!wlf_wl_display_init_registry(wayland->display)) {
		wlf_log(WLF_ERROR, "Failed to initialize Wayland registry");
		return false;
	}

	// Find the compositor interface in the registry
	struct wlf_wl_interface *compositor_interface =
		wlf_wl_display_get_registry_from_interface(wayland->display, wl_compositor_interface.name);
	if (!compositor_interface) {
		wlf_log(WLF_ERROR, "Failed to find compositor interface in registry");
		return false;
	}

	// Create compositor interface using the new signature
	wayland->compositor = wlf_wl_compositor_create(
		wayland->display->registry,
		compositor_interface->name,
		compositor_interface->version
	);
	if (!wayland->compositor) {
		wlf_log(WLF_ERROR, "Failed to create Wayland compositor interface");
		return false;
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

	// Cleanup compositor
	if (wayland->compositor) {
		wlf_wl_compositor_destroy(wayland->compositor);
		wayland->compositor = NULL;
	}

	wayland->started = false;
}

static void wayland_backend_destroy(struct wlf_backend *backend) {
	struct wlf_backend_wayland *wayland = (struct wlf_backend_wayland *)backend;

	wlf_log(WLF_DEBUG, "Destroying Wayland backend");

	// Stop first if still running
	wayland_backend_stop(backend);

	// Remove listeners
	wlf_linked_list_remove(&wayland->listeners.display_destroy.link);

	// Destroy display connection
	if (wayland->display) {
		wlf_wl_display_destroy(wayland->display);
		wayland->display = NULL;
	}

	free(wayland);
}

struct wlf_backend *wlf_backend_wayland_create(void *args) {
	struct wlf_backend_wayland *backend = calloc(1, sizeof(struct wlf_backend_wayland));
	if (!backend) {
		wlf_log(WLF_ERROR, "Failed to allocate Wayland backend");
		return NULL;
	}

	// Initialize base backend
	backend->base.impl = &wayland_backend_impl;
	backend->base.type = WLF_BACKEND_WAYLAND;
	backend->base.data = backend;

	// Initialize signals
	wlf_signal_init(&backend->base.events.destroy);

	// Parse arguments
	const struct wlf_backend_create_args *create_args =
		(const struct wlf_backend_create_args *)args;

	if (create_args && create_args->wayland.display) {
		// Use provided display
		backend->display = create_args->wayland.display;
		wlf_log(WLF_DEBUG, "Using provided Wayland display");
	} else {
		// Create new display connection
		backend->display = wlf_wl_display_create();
		if (!backend->display) {
			wlf_log(WLF_ERROR, "Failed to connect to Wayland display");
			free(backend);
			return NULL;
		}
		wlf_log(WLF_DEBUG, "Created new Wayland display connection");
	}

	// Setup listeners
	backend->listeners.display_destroy.notify = handle_display_destroy;
	wlf_signal_add(&backend->display->events.destroy, &backend->listeners.display_destroy);

	backend->started = false;

	wlf_log(WLF_INFO, "Created Wayland backend");
	return &backend->base;
}

bool wlf_backend_wayland_is_available(void) {
	// Check if we can connect to a Wayland display
	const char *wayland_display = getenv("WAYLAND_DISPLAY");
	if (!wayland_display) {
		// Try default socket
		wayland_display = "wayland-0";
	}

	// Simple availability check - see if socket exists
	char socket_path[256];
	const char *runtime_dir = getenv("XDG_RUNTIME_DIR");
	if (!runtime_dir) {
		return false;
	}

	snprintf(socket_path, sizeof(socket_path), "%s/%s", runtime_dir, wayland_display);
	return access(socket_path, F_OK) == 0;
}

bool wlf_backend_wayland_register(void) {
	static struct wlf_backend_registry_entry entry = {
		.type = WLF_BACKEND_WAYLAND,
		.name = "wayland",
		.priority = 100,  // High priority if available
		.create = wlf_backend_wayland_create,
		.is_available = wlf_backend_wayland_is_available,
		.handle = NULL,  // Statically linked
	};

	return wlf_backend_register(&entry);
}
