#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

struct wlf_wl_compositor *wlf_wl_compositor_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wl_compositor *compositor = malloc(sizeof(struct wlf_wl_compositor));
	if (compositor == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_compositor");
		return NULL;
	}

	compositor->base = NULL;
	wlf_signal_init(&compositor->events.destroy);

	uint32_t bind_version = version;
	if (version > (uint32_t)wl_compositor_interface.version) {
		wlf_log(WLF_DEBUG, "Server compositor version %u is higher than client version %u, "
				"using client version", version, (uint32_t)wl_compositor_interface.version);
		bind_version = (uint32_t)wl_compositor_interface.version;
	}

	compositor->base = wl_registry_bind(wl_registry, name, &wl_compositor_interface, bind_version);
	if (compositor->base == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind wl_compositor interface with name %u", name);
		free(compositor);
		return NULL;
	}

	wlf_log(WLF_DEBUG, "Successfully bound wl_compositor interface (name: %u, version: %u)",
			name, bind_version);

	return compositor;
}

void wlf_wl_compositor_destroy(struct wlf_wl_compositor *compositor) {
	if (compositor == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&compositor->events.destroy, compositor);

	if (compositor->base != NULL) {
		wl_compositor_destroy(compositor->base);
		compositor->base = NULL;
	}

	free(compositor);
}

struct wl_surface *wlf_wl_compositor_create_surface(struct wlf_wl_compositor *compositor) {
	if (compositor == NULL || compositor->base == NULL) {
		wlf_log(WLF_ERROR, "Invalid compositor - cannot create surface");
		return NULL;
	}

	struct wl_surface *surface = wl_compositor_create_surface(compositor->base);
	if (surface == NULL) {
		wlf_log(WLF_ERROR, "Failed to create wl_surface from compositor");
		return NULL;
	}

	return surface;
}

struct wl_region *wlf_wl_compositor_create_region(struct wlf_wl_compositor *compositor) {
	if (compositor == NULL || compositor->base == NULL) {
		wlf_log(WLF_ERROR, "Invalid compositor - cannot create region");
		return NULL;
	}

	struct wl_region *region = wl_compositor_create_region(compositor->base);
	if (region == NULL) {
		wlf_log(WLF_ERROR, "Failed to create wl_region from compositor");
		return NULL;
	}

	return region;
}
