#include "wlf/window/wayland/xdg_toplevel_window.h"

#include "wlf/platform/wayland/backend.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/swapchain/wlf_swapchain.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/wayland/wlf_wl_interface.h"
#include "wlf/wayland/wlf_wl_surface.h"
#include "wlf/wayland/wlf_xdg_wm_base.h"
#include "wayland/protocols/xdg-shell-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static struct wlf_xdg_toplevel_window *toplevel_from_window(
		struct wlf_window *window) {
	if (!wlf_window_is_xdg_toplevel(window)) {
		return NULL;
	}

	struct wlf_xdg_toplevel_window *toplevel = NULL;
	return wlf_container_of(window, toplevel, base);
}

static bool create_base_objects(struct wlf_xdg_toplevel_window *window,
		struct wlf_backend *backend) {
	struct wlf_wl_backend *wayland = wlf_wl_backend_from_backend(backend);
	assert(wayland != NULL);

	if (wayland->registry == NULL || wayland->wl_compositor.compositor == NULL) {
		wlf_log(WLF_ERROR, "Wayland backend is missing registry/compositor");
		return false;
	}

	struct wlf_wl_interface *xdg_wm_base_reg =
		wlf_wl_backend_find_interface(wayland, xdg_wm_base_interface.name);
	if (xdg_wm_base_reg == NULL) {
		wlf_log(WLF_ERROR, "Compositor does not expose xdg_wm_base");
		return false;
	}

	struct wlf_wl_compositor compositor = {
		.base = wayland->wl_compositor.compositor,
	};
	window->surface = wlf_wl_surface_create(&compositor);
	if (window->surface == NULL) {
		return false;
	}

	window->wm_base = wlf_xdg_wm_base_create(wayland->registry,
		xdg_wm_base_reg->name, xdg_wm_base_reg->version);
	if (window->wm_base == NULL) {
		return false;
	}

	window->xdg_surface = wlf_xdg_wm_base_get_xdg_surface(window->wm_base,
		window->surface->wl_surface);
	if (window->xdg_surface == NULL) {
		return false;
	}

	return true;
}

static void handle_xdg_surface_configure(struct wlf_listener *listener,
		void *data) {
	struct wlf_xdg_toplevel_window *window =
		wlf_container_of(listener, window, xdg_surface_configure);
	uint32_t serial = (uint32_t)(uintptr_t)data;

	wlf_xdg_surface_ack_configure(window->xdg_surface, serial);
	wlf_signal_emit_mutable(&window->base.events.expose, &window->base);
}

static void handle_xdg_toplevel_configure(struct wlf_listener *listener,
		void *data) {
	WLF_UNUSED(data);
	struct wlf_xdg_toplevel_window *window =
		wlf_container_of(listener, window, xdg_toplevel_configure);
	struct wlf_xdg_toplevel *toplevel = window->xdg_toplevel;

	if (toplevel->configure_width <= 0 || toplevel->configure_height <= 0) {
		return;
	}

	window->base.state.geometry.width = toplevel->configure_width;
	window->base.state.geometry.height = toplevel->configure_height;
	wlf_signal_emit_mutable(&window->base.events.resize, &window->base);
}

static void handle_xdg_toplevel_close(struct wlf_listener *listener,
		void *data) {
	WLF_UNUSED(data);
	struct wlf_xdg_toplevel_window *window =
		wlf_container_of(listener, window, xdg_toplevel_close);
	wlf_window_close(&window->base);
}

static void xdg_toplevel_window_destroy(struct wlf_window *base) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window == NULL) {
		free(base);
		return;
	}

	if (window->has_xdg_toplevel_close_listener) {
		wlf_linked_list_remove(&window->xdg_toplevel_close.link);
	}
	if (window->has_xdg_toplevel_configure_listener) {
		wlf_linked_list_remove(&window->xdg_toplevel_configure.link);
	}
	if (window->has_xdg_surface_configure_listener) {
		wlf_linked_list_remove(&window->xdg_surface_configure.link);
	}

	if (window->xdg_toplevel != NULL) {
		wlf_xdg_toplevel_destroy(window->xdg_toplevel);
	}
	if (window->xdg_surface != NULL) {
		wlf_xdg_surface_destroy(window->xdg_surface);
	}
	if (window->wm_base != NULL) {
		wlf_xdg_wm_base_destroy(window->wm_base);
	}
	if (window->surface != NULL) {
		wlf_wl_surface_destroy(window->surface);
	}
	free(window);
}

static void xdg_toplevel_window_close(struct wlf_window *base) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window == NULL || window->surface == NULL) {
		return;
	}

	wlf_wl_surface_attach(window->surface, NULL, 0, 0);
	wlf_wl_surface_commit(window->surface);
}

static void xdg_toplevel_window_show(struct wlf_window *base) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window != NULL && window->surface != NULL) {
		wlf_wl_surface_commit(window->surface);
	}
}

static void xdg_toplevel_window_hide(struct wlf_window *base) {
	xdg_toplevel_window_close(base);
}

static void xdg_toplevel_window_set_title(struct wlf_window *base,
		const char *title) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window != NULL && title != NULL) {
		wlf_xdg_toplevel_set_title(window->xdg_toplevel, title);
	}
}

static void xdg_toplevel_window_set_geometry(struct wlf_window *base,
		const struct wlf_rect *geometry) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window != NULL && geometry != NULL) {
		wlf_xdg_surface_set_window_geometry(window->xdg_surface,
			geometry->x, geometry->y, geometry->width, geometry->height);
	}
}

static void xdg_toplevel_window_set_size(struct wlf_window *base,
		int width, int height) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window != NULL) {
		wlf_xdg_surface_set_window_geometry(window->xdg_surface,
			base->state.geometry.x, base->state.geometry.y, width, height);
	}
}

static void xdg_toplevel_window_set_min_size(struct wlf_window *base,
		int width, int height) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window != NULL) {
		wlf_xdg_toplevel_set_min_size(window->xdg_toplevel, width, height);
	}
}

static void xdg_toplevel_window_set_max_size(struct wlf_window *base,
		int width, int height) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window != NULL) {
		wlf_xdg_toplevel_set_max_size(window->xdg_toplevel, width, height);
	}
}

static void xdg_toplevel_window_set_state(struct wlf_window *base,
		enum wlf_window_state_flags state) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window == NULL) {
		return;
	}

	if (state & WLF_WINDOW_FULLSCREEN) {
		wlf_xdg_toplevel_set_fullscreen(window->xdg_toplevel, NULL);
	} else {
		wlf_xdg_toplevel_unset_fullscreen(window->xdg_toplevel);
	}

	if (state & WLF_WINDOW_MAXIMIZED) {
		wlf_xdg_toplevel_set_maximized(window->xdg_toplevel);
	} else {
		wlf_xdg_toplevel_unset_maximized(window->xdg_toplevel);
	}

	if (state & WLF_WINDOW_MINIMIZED) {
		wlf_xdg_toplevel_set_minimized(window->xdg_toplevel);
	}
}

static void xdg_toplevel_window_set_input_region(struct wlf_window *base,
		const pixman_region32_t *region) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window != NULL) {
		wlf_wl_surface_set_input_region(window->surface, region);
		wlf_wl_surface_commit(window->surface);
	}
}

static void xdg_toplevel_window_set_opaque_region(struct wlf_window *base,
		const pixman_region32_t *region) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window != NULL) {
		wlf_wl_surface_set_opaque_region(window->surface, region);
		wlf_wl_surface_commit(window->surface);
	}
}

static void *xdg_toplevel_window_native_handle(struct wlf_window *base) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	return window != NULL ? window->surface->wl_surface : NULL;
}

static const struct wlf_window_impl xdg_toplevel_window_impl = {
	.destroy = xdg_toplevel_window_destroy,
	.close = xdg_toplevel_window_close,
	.show = xdg_toplevel_window_show,
	.hide = xdg_toplevel_window_hide,
	.set_title = xdg_toplevel_window_set_title,
	.set_geometry = xdg_toplevel_window_set_geometry,
	.set_size = xdg_toplevel_window_set_size,
	.set_min_size = xdg_toplevel_window_set_min_size,
	.set_max_size = xdg_toplevel_window_set_max_size,
	.set_position = NULL,
	.set_state = xdg_toplevel_window_set_state,
	.set_flags = NULL,
	.set_input_region = xdg_toplevel_window_set_input_region,
	.set_opaque_region = xdg_toplevel_window_set_opaque_region,
	.set_opacity = NULL,
	.set_mask = NULL,
	.set_background_color = NULL,
	.native_handle = xdg_toplevel_window_native_handle,
};

struct wlf_window *wlf_xdg_toplevel_window_create_from_backend(
		struct wlf_backend *backend, uint32_t width, uint32_t height) {
	if (backend == NULL || !wlf_backend_is_wayland(backend)) {
		wlf_log(WLF_ERROR, "xdg_toplevel_window requires a Wayland backend");
		return NULL;
	}

	struct wlf_xdg_toplevel_window *window = calloc(1, sizeof(*window));
	if (window == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_xdg_toplevel_window");
		return NULL;
	}

	wlf_window_init(&window->base, WLF_WINDOW_TYPE_TOPLEVEL,
		&xdg_toplevel_window_impl, backend, width, height);
	window->backend = backend;

	if (!create_base_objects(window, backend)) {
		wlf_window_destroy(&window->base);
		return NULL;
	}

	window->xdg_toplevel = wlf_xdg_surface_get_toplevel(window->xdg_surface);
	if (window->xdg_toplevel == NULL) {
		wlf_window_destroy(&window->base);
		return NULL;
	}

	window->xdg_surface_configure.notify = handle_xdg_surface_configure;
	wlf_signal_add(&window->xdg_surface->events.configure,
		&window->xdg_surface_configure);
	window->has_xdg_surface_configure_listener = true;
	window->xdg_toplevel_configure.notify = handle_xdg_toplevel_configure;
	wlf_signal_add(&window->xdg_toplevel->events.configure,
		&window->xdg_toplevel_configure);
	window->has_xdg_toplevel_configure_listener = true;
	window->xdg_toplevel_close.notify = handle_xdg_toplevel_close;
	wlf_signal_add(&window->xdg_toplevel->events.close,
		&window->xdg_toplevel_close);
	window->has_xdg_toplevel_close_listener = true;

	return &window->base;
}

bool wlf_window_is_xdg_toplevel(const struct wlf_window *window) {
	return window != NULL && window->impl == &xdg_toplevel_window_impl;
}

struct wlf_xdg_toplevel_window *wlf_xdg_toplevel_window_from_window(
		struct wlf_window *window) {
	return toplevel_from_window(window);
}
