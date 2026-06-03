#include "wlf/wayland/wlf_wp_viewporter.h"
#include "wlf/utils/wlf_log.h"
#include "wayland/protocols/viewporter-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

struct wlf_wp_viewporter *wlf_wp_viewporter_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wp_viewporter *viewporter =
		malloc(sizeof(struct wlf_wp_viewporter));
	if (viewporter == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wp_viewporter");
		return NULL;
	}

	viewporter->base = NULL;

	uint32_t bind_version = version;
	if (version > (uint32_t)wp_viewporter_interface.version) {
		wlf_log(WLF_DEBUG,
			"Server wp_viewporter version %u is higher than client "
			"version %u, using client version",
			version, (uint32_t)wp_viewporter_interface.version);
		bind_version = (uint32_t)wp_viewporter_interface.version;
	}

	viewporter->base = wl_registry_bind(wl_registry, name,
		&wp_viewporter_interface, bind_version);
	if (viewporter->base == NULL) {
		wlf_log(WLF_ERROR,
			"Failed to bind wp_viewporter interface (name: %u)", name);
		free(viewporter);
		return NULL;
	}

	wlf_log(WLF_DEBUG,
		"Successfully bound wp_viewporter (name: %u, version: %u)",
		name, bind_version);
	viewporter->version = bind_version;
	wlf_signal_init(&viewporter->events.destroy);

	return viewporter;
}

void wlf_wp_viewporter_destroy(struct wlf_wp_viewporter *viewporter) {
	if (viewporter == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&viewporter->events.destroy, viewporter);
	assert(wlf_linked_list_empty(&viewporter->events.destroy.listener_list));

	if (viewporter->base != NULL) {
		wp_viewporter_destroy(viewporter->base);
		viewporter->base = NULL;
	}

	free(viewporter);
}

struct wlf_wp_viewport *wlf_wp_viewporter_get_viewport(
		struct wlf_wp_viewporter *viewporter, struct wl_surface *surface) {
	assert(viewporter != NULL);
	assert(viewporter->base != NULL);
	assert(surface != NULL);

	struct wlf_wp_viewport *viewport = malloc(sizeof(struct wlf_wp_viewport));
	if (viewport == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wp_viewport");
		return NULL;
	}

	viewport->base = NULL;

	viewport->base = wp_viewporter_get_viewport(viewporter->base, surface);
	if (viewport->base == NULL) {
		wlf_log(WLF_ERROR, "wp_viewporter_get_viewport() returned NULL");
		free(viewport);
		return NULL;
	}

	viewport->version =
		wl_proxy_get_version((struct wl_proxy *)viewport->base);
	wlf_signal_init(&viewport->events.destroy);

	return viewport;
}

void wlf_wp_viewport_set_source(struct wlf_wp_viewport *viewport,
		double x, double y, double width, double height) {
	assert(viewport->base != NULL);

	wp_viewport_set_source(viewport->base,
		wl_fixed_from_double(x),
		wl_fixed_from_double(y),
		wl_fixed_from_double(width),
		wl_fixed_from_double(height));
}

void wlf_wp_viewport_set_destination(struct wlf_wp_viewport *viewport,
		int32_t width, int32_t height) {
	assert(viewport->base != NULL);

	wp_viewport_set_destination(viewport->base, width, height);
}

void wlf_wp_viewport_destroy(struct wlf_wp_viewport *viewport) {
	if (viewport == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&viewport->events.destroy, viewport);
	assert(wlf_linked_list_empty(&viewport->events.destroy.listener_list));

	if (viewport->base != NULL) {
		wp_viewport_destroy(viewport->base);
		viewport->base = NULL;
	}

	free(viewport);
}
