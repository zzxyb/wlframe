#include "wlf/window/wayland/wlr_layer_window.h"

#include "wlf/platform/wayland/backend.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/swapchain/wlf_swapchain.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/wayland/wlf_wl_interface.h"
#include "wlf/wayland/wlf_wl_surface.h"
#include "wayland/protocols/wlr-layer-shell-unstable-v1-client-protocol.h"

#include <assert.h>
#include <stdlib.h>

#include <wayland-client-protocol.h>

static struct wlf_wlr_layer_window *layer_from_window(
		struct wlf_window *window) {
	if (!wlf_window_is_wlr_layer(window)) {
		return NULL;
	}

	struct wlf_wlr_layer_window *layer = NULL;
	return wlf_container_of(window, layer, base);
}

static void handle_layer_surface_configure(struct wlf_listener *listener,
		void *data) {
	WLF_UNUSED(data);
	struct wlf_wlr_layer_window *window =
		wlf_container_of(listener, window, layer_surface_configure);
	struct wlf_zwlr_layer_surface_v1 *surface = window->layer_surface;

	wlf_zwlr_layer_surface_v1_ack_configure(surface,
		surface->configure_serial);

	bool resized = false;
	if (surface->width > 0 &&
			window->base.state.geometry.width != (int)surface->width) {
		window->base.state.geometry.width = (int)surface->width;
		resized = true;
	}
	if (surface->height > 0 &&
			window->base.state.geometry.height != (int)surface->height) {
		window->base.state.geometry.height = (int)surface->height;
		resized = true;
	}
	if (resized) {
		wlf_signal_emit_mutable(&window->base.events.resize, &window->base);
	}

	wlf_signal_emit_mutable(&window->base.events.expose, &window->base);
}

static void handle_layer_surface_closed(struct wlf_listener *listener,
		void *data) {
	WLF_UNUSED(data);
	struct wlf_wlr_layer_window *window =
		wlf_container_of(listener, window, layer_surface_closed);
	wlf_window_close(&window->base);
}

static void layer_window_destroy(struct wlf_window *base) {
	struct wlf_wlr_layer_window *window = layer_from_window(base);
	if (window == NULL) {
		free(base);
		return;
	}

	if (window->has_layer_surface_closed_listener) {
		wlf_linked_list_remove(&window->layer_surface_closed.link);
	}
	if (window->has_layer_surface_configure_listener) {
		wlf_linked_list_remove(&window->layer_surface_configure.link);
	}
	if (window->layer_surface != NULL) {
		wlf_zwlr_layer_surface_v1_destroy(window->layer_surface);
	}
	if (window->layer_shell != NULL) {
		wlf_zwlr_layer_shell_v1_destroy(window->layer_shell);
	}
	if (window->surface != NULL) {
		wlf_wl_surface_destroy(window->surface);
	}
	free(window);
}

static void layer_window_close(struct wlf_window *base) {
	struct wlf_wlr_layer_window *window = layer_from_window(base);
	if (window == NULL || window->surface == NULL) {
		return;
	}
	wlf_wl_surface_attach(window->surface, NULL, 0, 0);
	wlf_wl_surface_commit(window->surface);
}

static void layer_window_show(struct wlf_window *base) {
	struct wlf_wlr_layer_window *window = layer_from_window(base);
	if (window != NULL && window->surface != NULL) {
		wlf_wl_surface_commit(window->surface);
	}
}

static void layer_window_set_geometry(struct wlf_window *base,
		const struct wlf_rect *geometry) {
	struct wlf_wlr_layer_window *window = layer_from_window(base);
	if (window != NULL && geometry != NULL &&
			geometry->width >= 0 && geometry->height >= 0) {
		wlf_zwlr_layer_surface_v1_set_size(window->layer_surface,
			(uint32_t)geometry->width, (uint32_t)geometry->height);
	}
}

static void layer_window_set_size(struct wlf_window *base,
		int width, int height) {
	struct wlf_wlr_layer_window *window = layer_from_window(base);
	if (window != NULL && width >= 0 && height >= 0) {
		wlf_zwlr_layer_surface_v1_set_size(window->layer_surface,
			(uint32_t)width, (uint32_t)height);
	}
}

static void layer_window_set_input_region(struct wlf_window *base,
		const pixman_region32_t *region) {
	struct wlf_wlr_layer_window *window = layer_from_window(base);
	if (window != NULL) {
		wlf_wl_surface_set_input_region(window->surface, region);
		wlf_wl_surface_commit(window->surface);
	}
}

static void layer_window_set_opaque_region(struct wlf_window *base,
		const pixman_region32_t *region) {
	struct wlf_wlr_layer_window *window = layer_from_window(base);
	if (window != NULL) {
		wlf_wl_surface_set_opaque_region(window->surface, region);
		wlf_wl_surface_commit(window->surface);
	}
}

static void *layer_window_native_handle(struct wlf_window *base) {
	struct wlf_wlr_layer_window *window = layer_from_window(base);
	return window != NULL ? window->surface->wl_surface : NULL;
}

static void layer_window_schedule_frame(struct wlf_window *base) {
	struct wlf_wlr_layer_window *window = layer_from_window(base);
	if (window != NULL) {
		wlf_wl_surface_schedule_frame(window->surface, base);
	}
}

static void layer_window_arm_frame(struct wlf_window *base) {
	struct wlf_wlr_layer_window *window = layer_from_window(base);
	if (window != NULL) {
		wlf_wl_surface_arm_frame(window->surface, base);
	}
}

static const struct wlf_window_impl layer_window_impl = {
	.destroy = layer_window_destroy,
	.close = layer_window_close,
	.show = layer_window_show,
	.hide = layer_window_close,
	.set_geometry = layer_window_set_geometry,
	.set_size = layer_window_set_size,
	.set_input_region = layer_window_set_input_region,
	.set_opaque_region = layer_window_set_opaque_region,
	.native_handle = layer_window_native_handle,
	.arm_frame = layer_window_arm_frame,
	.schedule_frame = layer_window_schedule_frame,
};

struct wlf_window *wlf_wlr_layer_window_create_from_backend(
		struct wlf_backend *backend, struct wl_output *output,
		enum wlf_zwlr_layer_v1 layer, const char *namespace,
		uint32_t width, uint32_t height) {
	if (backend == NULL || !wlf_backend_is_wayland(backend)) {
		wlf_log(WLF_ERROR, "wlr_layer_window requires a Wayland backend");
		return NULL;
	}
	if (namespace == NULL) {
		wlf_log(WLF_ERROR, "wlr_layer_window requires a namespace");
		return NULL;
	}

	struct wlf_wl_backend *wayland = wlf_wl_backend_from_backend(backend);
	assert(wayland != NULL);
	if (wayland->registry == NULL || wayland->wl_compositor.compositor == NULL) {
		wlf_log(WLF_ERROR, "Wayland backend is missing registry/compositor");
		return NULL;
	}
	struct wlf_wl_interface *layer_shell_reg = wlf_wl_backend_find_interface(
		wayland, zwlr_layer_shell_v1_interface.name);
	if (layer_shell_reg == NULL) {
		wlf_log(WLF_ERROR, "Compositor does not expose zwlr_layer_shell_v1");
		return NULL;
	}

	struct wlf_wlr_layer_window *window = calloc(1, sizeof(*window));
	if (window == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wlr_layer_window");
		return NULL;
	}
	wlf_window_init(&window->base, WLF_WINDOW_TYPE_LAYER, &layer_window_impl,
		backend, width, height);
	window->backend = backend;

	struct wlf_wl_compositor compositor = {
		.base = wayland->wl_compositor.compositor,
	};
	window->surface = wlf_wl_surface_create(&compositor);
	if (window->surface == NULL) {
		wlf_window_destroy(&window->base);
		return NULL;
	}
	wlf_wl_surface_set_window(window->surface, &window->base);
	window->layer_shell = wlf_zwlr_layer_shell_v1_create(wayland->registry,
		layer_shell_reg->name, layer_shell_reg->version);
	if (window->layer_shell == NULL) {
		wlf_window_destroy(&window->base);
		return NULL;
	}
	window->layer_surface = wlf_zwlr_layer_shell_v1_get_layer_surface(
		window->layer_shell, window->surface->wl_surface, output, layer,
		namespace);
	if (window->layer_surface == NULL) {
		wlf_window_destroy(&window->base);
		return NULL;
	}
	wlf_zwlr_layer_surface_v1_set_size(window->layer_surface, width, height);

	window->layer_surface_configure.notify = handle_layer_surface_configure;
	wlf_signal_add(&window->layer_surface->events.configure,
		&window->layer_surface_configure);
	window->has_layer_surface_configure_listener = true;
	window->layer_surface_closed.notify = handle_layer_surface_closed;
	wlf_signal_add(&window->layer_surface->events.closed,
		&window->layer_surface_closed);
	window->has_layer_surface_closed_listener = true;

	return &window->base;
}

bool wlf_window_is_wlr_layer(const struct wlf_window *window) {
	return window != NULL && window->impl == &layer_window_impl;
}

struct wlf_wlr_layer_window *wlf_wlr_layer_window_from_window(
		struct wlf_window *window) {
	return layer_from_window(window);
}
