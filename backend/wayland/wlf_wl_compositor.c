#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/wayland/wlf_wl_display.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <wayland-client-protocol.h>

static bool wlf_bind_wl_compositor(struct wlf_wl_compositor *compositor, struct wlf_wl_interface *registry) {
	client_interface_version_is_higher(wl_compositor_interface.name, registry->version,
			wl_compositor_interface.version);
	compositor->compositor = wl_registry_bind(compositor->display->registry, registry->name,
		&wl_compositor_interface, registry->version);
	if (compositor->compositor == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind wl_compositor!");
		return false;
	}
	wlf_log(WLF_INFO, "Bound wl_compositor with name %u", registry->name);

	return true;
}

static void handle_global_add(struct wlf_listener *listener, void *data) {
	struct wlf_wl_interface *registry = (struct wlf_wl_interface *)data;
	struct wlf_wl_compositor *compositor = wlf_container_of(listener, compositor, global_add);
	assert(compositor != NULL);
	assert(registry != NULL);
	if (strcmp(registry->interface, wl_compositor_interface.name) == 0) {
		if (!wlf_bind_wl_compositor(compositor, registry)) {
			assert(false);
			return;
		}
		wlf_log(WLF_INFO, "Bound wl_compositor with name %u", registry->name);
	}
}

static void handle_global_remove(struct wlf_listener *listener, void *data) {
	WLF_UNUSED(data);
	struct wlf_wl_compositor *compositor = wlf_container_of(listener, compositor, global_remove);
	assert(compositor != NULL);
	wlf_wl_compositor_destroy(compositor);
}

struct wlf_wl_compositor *wlf_wl_compositor_create(struct wlf_wl_display *display) {
	struct wlf_wl_compositor *compositor = malloc(sizeof(struct wlf_wl_compositor));
	if (compositor == NULL) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_wl_compositor failed!");
		return NULL;
	}

	compositor->display = display;
	wlf_signal_init(&compositor->events.destroy);

	struct wlf_wl_interface *registry = wlf_wl_display_get_registry_from_interface(display, wl_compositor_interface.name);
	if (registry == NULL) {
		compositor->compositor = NULL;
		compositor->global_add.notify = handle_global_add;
		wlf_signal_add(&display->events.global_add, &compositor->global_add);
		compositor->global_remove.notify = handle_global_remove;
		wlf_signal_add(&display->events.global_remove, &compositor->global_remove);
	} else {
		if (!wlf_bind_wl_compositor(compositor, registry)) {
			free(compositor);
			assert(false);
			return NULL;
		}
		wlf_log(WLF_INFO, "Bound registryed wl_compositor with name %u", registry->name);
	}

	return compositor;
}

bool wlf_wl_compositor_is_nil(struct wlf_wl_compositor *compositor) {
	return compositor == NULL || compositor->compositor == NULL;
}

void wlf_wl_compositor_destroy(struct wlf_wl_compositor *compositor) {
	if (compositor == NULL) {
		return;
	}

	wlf_signal_emit(&compositor->events.destroy, compositor);
	if (compositor->compositor != NULL) {
		wl_compositor_destroy(compositor->compositor);
		compositor->compositor = NULL;
	}

	wlf_double_list_remove(&compositor->global_add.link);
	wlf_double_list_remove(&compositor->global_remove.link);
	free(compositor);
}

struct wl_surface *wlf_wl_compositor_create_surface(struct wlf_wl_compositor *compositor) {
	if (compositor == NULL || compositor->compositor == NULL) {
		return NULL;
	}

	struct wl_surface *surface = wl_compositor_create_surface(compositor->compositor);
	if (surface == NULL) {
		wlf_log(WLF_ERROR, "Failed to create wl_surface!");
		assert(false);
		return NULL;
	}
	return surface;
}

struct wl_region *wlf_wl_compositor_create_region(struct wlf_wl_compositor *compositor) {
	if (compositor == NULL || compositor->compositor == NULL) {
		return NULL;
	}

	struct wl_region *region = wl_compositor_create_region(compositor->compositor);
	if (region == NULL) {
		wlf_log(WLF_ERROR, "Failed to create wl_region!");
		assert(false);
		return NULL;
	}

	return region;
}
