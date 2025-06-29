#include "wlf/utils/wlf_log.h"
#include "wlf/wayland/wlf_wl_display.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

static struct wlf_wl_display *g_display = NULL;
static struct wlf_wl_compositor *g_compositor = NULL;
static bool compositor_found = false;

static void on_global_add(struct wlf_listener *listener, void *data) {
	struct wlf_wl_interface *interface = (struct wlf_wl_interface *)data;

	wlf_log(WLF_DEBUG, "Global interface added: %s v%u", interface->interface, interface->version);

	if (strcmp(interface->interface, wl_compositor_interface.name) == 0) {
		wlf_log(WLF_INFO, "Compositor interface found! Creating compositor...");

		g_compositor = wlf_wl_compositor_create(
			g_display->registry,
			interface->name,
			interface->version
		);

		if (g_compositor != NULL) {
			wlf_log(WLF_INFO, "Compositor created successfully");
			compositor_found = true;
		} else {
			wlf_log(WLF_ERROR, "Failed to create compositor");
		}
	}
}

static void on_global_remove(struct wlf_listener *listener, void *data) {
	struct wlf_wl_interface *interface = (struct wlf_wl_interface *)data;

	wlf_log(WLF_DEBUG, "Global interface removed: %s", interface->interface);

	if (strcmp(interface->interface, wl_compositor_interface.name) == 0) {
		wlf_log(WLF_INFO, "Compositor interface removed");
		if (g_compositor) {
			wlf_wl_compositor_destroy(g_compositor);
			g_compositor = NULL;
		}
		compositor_found = false;
	}
}

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);
	g_display = wlf_wl_display_create();
	if (g_display == NULL) {
		wlf_log(WLF_ERROR, "Failed to create display");
		return -1;
	}

	wlf_wl_display_init_registry(g_display);
	struct wlf_wl_interface *compositor_interface =
		wlf_wl_display_get_registry_from_interface(g_display, wl_compositor_interface.name);

	if (compositor_interface == NULL) {
		wlf_log(WLF_INFO, "Compositor interface not found initially, setting up listeners...");

		struct wlf_listener global_add_listener = { .notify = on_global_add };
		struct wlf_listener global_remove_listener = { .notify = on_global_remove };

		wlf_signal_add(&g_display->events.global_add, &global_add_listener);
		wlf_signal_add(&g_display->events.global_remove, &global_remove_listener);
		wlf_log(WLF_INFO, "Waiting for compositor interface...");
		while (!compositor_found) {
			if (wl_display_dispatch(g_display->base) == -1) {
				wlf_log(WLF_ERROR, "Failed to dispatch Wayland events");
				wlf_wl_display_destroy(g_display);
				return -1;
			}
		}
	} else {
		wlf_log(WLF_INFO, "Compositor interface found immediately");
		g_compositor = wlf_wl_compositor_create(
			g_display->registry,
			compositor_interface->name,
			compositor_interface->version
		);
		if (g_compositor == NULL) {
			wlf_log(WLF_ERROR, "Failed to create compositor");
			wlf_wl_display_destroy(g_display);
			return -1;
		}
	}

	assert(g_compositor->base != NULL);
	wlf_log(WLF_INFO, "wl_compositor interface: %p", g_compositor->base);

	struct wl_surface *surface = wlf_wl_compositor_create_surface(g_compositor);
	if (surface == NULL) {
		wlf_log(WLF_ERROR, "Failed created wl_surface");
		wlf_wl_compositor_destroy(g_compositor);
		wlf_wl_display_destroy(g_display);
		return -1;
	}

	struct wl_region *region = wlf_wl_compositor_create_region(g_compositor);
	if (region == NULL) {
		wlf_log(WLF_INFO, "Failed created wl_region");
		wl_surface_destroy(surface);
		wlf_wl_compositor_destroy(g_compositor);
		wlf_wl_display_destroy(g_display);
		return -1;
	}

	wl_surface_destroy(surface);
	wl_region_destroy(region);
	wlf_wl_compositor_destroy(g_compositor);
	wlf_wl_display_destroy(g_display);

	wlf_log(WLF_INFO, "Wayland compositor test completed successfully");
	return 0;
}
