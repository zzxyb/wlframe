#include "wlf/wayland/wlf_zwlr_layer_shell_v1.h"
#include "wayland/protocols/wlr-layer-shell-unstable-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client.h>

static void layer_surface_handle_configure(void *data,
		struct zwlr_layer_surface_v1 *base,
		uint32_t serial,
		uint32_t width,
		uint32_t height) {
	WLF_UNUSED(base);

	struct wlf_zwlr_layer_surface_v1 *surface = data;
	surface->configure_serial = serial;
	surface->width = width;
	surface->height = height;
	wlf_signal_emit_mutable(&surface->events.configure, surface);
}

static void layer_surface_handle_closed(void *data,
		struct zwlr_layer_surface_v1 *base) {
	WLF_UNUSED(base);

	struct wlf_zwlr_layer_surface_v1 *surface = data;
	wlf_signal_emit_mutable(&surface->events.closed, surface);
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_handle_configure,
	.closed = layer_surface_handle_closed,
};

struct wlf_zwlr_layer_shell_v1 *wlf_zwlr_layer_shell_v1_create(
		struct wl_registry *registry,
		uint32_t name,
		uint32_t version) {
	assert(registry);

	uint32_t bind_ver = (uint32_t)zwlr_layer_shell_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_zwlr_layer_shell_v1 *shell = calloc(1, sizeof(*shell));
	if (shell == NULL) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwlr_layer_shell_v1");
		return NULL;
	}

	shell->base = wl_registry_bind(
		registry, name, &zwlr_layer_shell_v1_interface, bind_ver);
	if (shell->base == NULL) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for "
			"zwlr_layer_shell_v1 (name: %u)",
			name);
		free(shell);
		return NULL;
	}
	shell->version = bind_ver;

	wlf_signal_init(&shell->events.destroy);

	wlf_log(WLF_DEBUG, "bound zwlr_layer_shell_v1 (name: %u, version: %u)",
		name, bind_ver);

	return shell;
}

void wlf_zwlr_layer_shell_v1_destroy(struct wlf_zwlr_layer_shell_v1 *shell) {
	if (shell == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&shell->events.destroy, shell);
	assert(wlf_linked_list_empty(&shell->events.destroy.listener_list));
	if (shell->base != NULL) {
		if (shell->version >=
			ZWLR_LAYER_SHELL_V1_DESTROY_SINCE_VERSION) {
			zwlr_layer_shell_v1_destroy(shell->base);
		} else {
			wl_proxy_destroy((struct wl_proxy *)shell->base);
		}
		shell->base = NULL;
	}
	free(shell);
}

struct wlf_zwlr_layer_surface_v1 *wlf_zwlr_layer_shell_v1_get_layer_surface(
		struct wlf_zwlr_layer_shell_v1 *shell,
		struct wl_surface *wl_surface,
		struct wl_output *wl_output,
		enum wlf_zwlr_layer_v1 layer,
		const char *namespace) {
	assert(shell);
	assert(shell->base);
	assert(wl_surface);
	assert(namespace);

	struct wlf_zwlr_layer_surface_v1 *surface = calloc(1, sizeof(*surface));
	if (surface == NULL) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_zwlr_layer_surface_v1");
		return NULL;
	}

	surface->base = zwlr_layer_shell_v1_get_layer_surface(
		shell->base, wl_surface, wl_output, (uint32_t)layer, namespace);
	if (surface->base == NULL) {
		wlf_log(WLF_ERROR,
			"zwlr_layer_shell_v1_get_layer_surface() returned "
			"NULL");
		free(surface);
		return NULL;
	}
	surface->version =
		wl_proxy_get_version((struct wl_proxy *)surface->base);

	wlf_signal_init(&surface->events.configure);
	wlf_signal_init(&surface->events.closed);
	wlf_signal_init(&surface->events.destroy);

	zwlr_layer_surface_v1_add_listener(
		surface->base, &layer_surface_listener, surface);

	return surface;
}

void wlf_zwlr_layer_surface_v1_set_size(
		struct wlf_zwlr_layer_surface_v1 *surface,
		uint32_t width,
		uint32_t height) {
	assert(surface);
	assert(surface->base);

	zwlr_layer_surface_v1_set_size(surface->base, width, height);
}

void wlf_zwlr_layer_surface_v1_set_anchor(
		struct wlf_zwlr_layer_surface_v1 *surface,
		enum wlf_zwlr_layer_surface_v1_anchor anchor) {
	assert(surface);
	assert(surface->base);

	zwlr_layer_surface_v1_set_anchor(surface->base, (uint32_t)anchor);
}

void wlf_zwlr_layer_surface_v1_set_exclusive_zone(
		struct wlf_zwlr_layer_surface_v1 *surface,
		int32_t zone) {
	assert(surface);
	assert(surface->base);

	zwlr_layer_surface_v1_set_exclusive_zone(surface->base, zone);
}

void wlf_zwlr_layer_surface_v1_set_margin(
		struct wlf_zwlr_layer_surface_v1 *surface,
		int32_t top,
		int32_t right,
		int32_t bottom,
		int32_t left) {
	assert(surface);
	assert(surface->base);

	zwlr_layer_surface_v1_set_margin(
		surface->base, top, right, bottom, left);
}

void wlf_zwlr_layer_surface_v1_set_keyboard_interactivity(
		struct wlf_zwlr_layer_surface_v1 *surface,
		enum wlf_zwlr_layer_surface_v1_keyboard_interactivity interactivity) {
	assert(surface);
	assert(surface->base);

	if (interactivity ==
			WLF_ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND &&
		surface->version <
			ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND_SINCE_VERSION) {
		wlf_log(WLF_ERROR,
			"zwlr_layer_surface_v1 keyboard interactivity "
			"on_demand "
			"requires version 4");
		return;
	}
	zwlr_layer_surface_v1_set_keyboard_interactivity(
		surface->base, (uint32_t)interactivity);
}

void wlf_zwlr_layer_surface_v1_get_popup(
		struct wlf_zwlr_layer_surface_v1 *surface,
		struct xdg_popup *popup) {
	assert(surface);
	assert(surface->base);
	assert(popup);

	zwlr_layer_surface_v1_get_popup(surface->base, popup);
}

void wlf_zwlr_layer_surface_v1_ack_configure(
		struct wlf_zwlr_layer_surface_v1 *surface,
		uint32_t serial) {
	assert(surface);
	assert(surface->base);

	zwlr_layer_surface_v1_ack_configure(surface->base, serial);
}

void wlf_zwlr_layer_surface_v1_set_layer(struct wlf_zwlr_layer_surface_v1 *surface,
		enum wlf_zwlr_layer_v1 layer) {
	assert(surface);
	assert(surface->base);

	if (surface->version < ZWLR_LAYER_SURFACE_V1_SET_LAYER_SINCE_VERSION) {
		wlf_log(WLF_ERROR,
			"zwlr_layer_surface_v1.set_layer requires version 2");
		return;
	}
	zwlr_layer_surface_v1_set_layer(surface->base, (uint32_t)layer);
}

void wlf_zwlr_layer_surface_v1_set_exclusive_edge(
		struct wlf_zwlr_layer_surface_v1 *surface,
		enum wlf_zwlr_layer_surface_v1_anchor edge) {
	assert(surface);
	assert(surface->base);

	if (surface->version <
		ZWLR_LAYER_SURFACE_V1_SET_EXCLUSIVE_EDGE_SINCE_VERSION) {
		wlf_log(WLF_ERROR,
			"zwlr_layer_surface_v1.set_exclusive_edge requires "
			"version 5");
		return;
	}
	zwlr_layer_surface_v1_set_exclusive_edge(surface->base, (uint32_t)edge);
}

void wlf_zwlr_layer_surface_v1_destroy(struct wlf_zwlr_layer_surface_v1 *surface) {
	if (surface == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&surface->events.destroy, surface);
	assert(wlf_linked_list_empty(&surface->events.configure.listener_list));
	assert(wlf_linked_list_empty(&surface->events.closed.listener_list));
	assert(wlf_linked_list_empty(&surface->events.destroy.listener_list));
	if (surface->base) {
		zwlr_layer_surface_v1_destroy(surface->base);
		surface->base = NULL;
	}
	free(surface);
}
