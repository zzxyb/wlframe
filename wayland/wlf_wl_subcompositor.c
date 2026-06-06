#include "wlf/wayland/wlf_wl_subcompositor.h"
#include "wlf/wayland/wlf_wl_subsurface.h"
#include "wlf/wayland/wlf_wl_surface.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

struct wlf_wl_subcompositor *wlf_wl_subcompositor_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wl_subcompositor *subcompositor = calloc(1, sizeof(*subcompositor));
	if (subcompositor == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_subcompositor");
		return NULL;
	}

	uint32_t bind_version = version;
	if (version > (uint32_t)wl_subcompositor_interface.version) {
		bind_version = (uint32_t)wl_subcompositor_interface.version;
	}

	subcompositor->wl_subcompositor = wl_registry_bind(wl_registry, name,
		&wl_subcompositor_interface, bind_version);
	if (subcompositor->wl_subcompositor == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind wl_subcompositor");
		free(subcompositor);
		return NULL;
	}

	subcompositor->version = bind_version;
	wlf_signal_init(&subcompositor->events.destroy);

	return subcompositor;
}

void wlf_wl_subcompositor_destroy(struct wlf_wl_subcompositor *subcompositor) {
	if (subcompositor == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&subcompositor->events.destroy, subcompositor);
	assert(wlf_linked_list_empty(&subcompositor->events.destroy.listener_list));

	if (subcompositor->wl_subcompositor != NULL) {
		wl_subcompositor_destroy(subcompositor->wl_subcompositor);
	}
	free(subcompositor);
}

struct wlf_wl_subsurface *wlf_wl_subcompositor_get_subsurface(
		struct wlf_wl_subcompositor *subcompositor,
		struct wlf_wl_surface *surface,
		struct wlf_wl_surface *parent) {
	assert(subcompositor != NULL);
	assert(surface != NULL);
	assert(parent != NULL);

	struct wlf_wl_subsurface *subsurface = calloc(1, sizeof(*subsurface));
	if (subsurface == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_subsurface");
		return NULL;
	}

	subsurface->wl_subsurface = wl_subcompositor_get_subsurface(
		subcompositor->wl_subcompositor,
		surface->wl_surface,
		parent->wl_surface);
	if (subsurface->wl_subsurface == NULL) {
		wlf_log(WLF_ERROR, "wl_subcompositor_get_subsurface failed");
		free(subsurface);
		return NULL;
	}

	wlf_signal_init(&subsurface->events.destroy);
	subsurface->version =
		wl_proxy_get_version((struct wl_proxy *)subsurface->wl_subsurface);

	return subsurface;
}
