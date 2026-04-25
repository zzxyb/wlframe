#include "wlf/window/wayland/xdg_window.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/platform/wayland/backend.h"
#include "wlf/wayland/wlf_wl_interface.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

#include <wayland-client-protocol.h>

static struct wlf_wl_backend *xdg_window_wayland_backend(
		struct wlf_xdg_window *xdg_window) {
	if (xdg_window == NULL || xdg_window->backend == NULL) {
		return NULL;
	}

	return wlf_wl_backend_from_backend(xdg_window->backend);
}

static struct wl_region *xdg_window_create_wl_region(
		struct wlf_xdg_window *xdg_window, const struct wlf_region *region) {
	if (region == NULL || region->data == NULL || region->data->numRects == 0) {
		return NULL;
	}

	struct wlf_wl_backend *wayland = xdg_window_wayland_backend(xdg_window);
	if (wayland == NULL || wayland->wl_compositor.compositor == NULL) {
		return NULL;
	}

	struct wl_region *wl_region = wl_compositor_create_region(wayland->wl_compositor.compositor);
	if (wl_region == NULL) {
		wlf_log(WLF_ERROR, "Failed to create wl_region");
		return NULL;
	}

	for (long i = 0; i < region->data->numRects; ++i) {
		const struct wlf_frect *rect = &region->data->rects[i];
		wl_region_add(wl_region, (int32_t)rect->x, (int32_t)rect->y,
			(int32_t)rect->width, (int32_t)rect->height);
	}

	return wl_region;
}

static void xdg_window_destroy(struct wlf_window *window) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL) {
		free(window);
		return;
	}

	if (xdg_window->xdg_toplevel != NULL) {
		xdg_toplevel_destroy(xdg_window->xdg_toplevel);
	}

	if (xdg_window->xdg_surface != NULL) {
		xdg_surface_destroy(xdg_window->xdg_surface);
	}

	if (xdg_window->surface != NULL) {
		wl_surface_destroy(xdg_window->surface);
	}

	free(xdg_window);
}

static void xdg_window_close(struct wlf_window *window) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->surface == NULL) {
		return;
	}

	/* Unmap the surface by committing a NULL buffer. */
	wl_surface_attach(xdg_window->surface, NULL, 0, 0);
	wl_surface_commit(xdg_window->surface);
}

static void xdg_window_show(struct wlf_window *window) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->surface == NULL) {
		return;
	}

	wl_surface_commit(xdg_window->surface);
}

static void xdg_window_hide(struct wlf_window *window) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->surface == NULL) {
		return;
	}

	wl_surface_attach(xdg_window->surface, NULL, 0, 0);
	wl_surface_commit(xdg_window->surface);
}

static void xdg_window_set_title(struct wlf_window *window, const char *title) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->xdg_toplevel == NULL || title == NULL) {
		return;
	}

	xdg_toplevel_set_title(xdg_window->xdg_toplevel, title);
	wl_surface_commit(xdg_window->surface);
}

static void xdg_window_set_geometry(struct wlf_window *window,
		const struct wlf_rect *geometry) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->xdg_surface == NULL || geometry == NULL) {
		return;
	}

	xdg_surface_set_window_geometry(xdg_window->xdg_surface,
		geometry->x, geometry->y, geometry->width, geometry->height);
	wl_surface_commit(xdg_window->surface);
}

static void xdg_window_set_size(struct wlf_window *window, int width, int height) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->xdg_surface == NULL) {
		return;
	}

	xdg_surface_set_window_geometry(xdg_window->xdg_surface,
		window->state.geometry.x, window->state.geometry.y, width, height);
	wl_surface_commit(xdg_window->surface);
}

static void xdg_window_set_min_size(struct wlf_window *window, int width, int height) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->xdg_toplevel == NULL) {
		return;
	}

	xdg_toplevel_set_min_size(xdg_window->xdg_toplevel, width, height);
	wl_surface_commit(xdg_window->surface);
}

static void xdg_window_set_max_size(struct wlf_window *window, int width, int height) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->xdg_toplevel == NULL) {
		return;
	}

	xdg_toplevel_set_max_size(xdg_window->xdg_toplevel, width, height);
	wl_surface_commit(xdg_window->surface);
}

static void xdg_window_set_state(struct wlf_window *window,
		enum wlf_window_state_flags state) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->xdg_toplevel == NULL) {
		return;
	}

	if (state & WLF_WINDOW_FULLSCREEN) {
		xdg_toplevel_set_fullscreen(xdg_window->xdg_toplevel, NULL);
	} else {
		xdg_toplevel_unset_fullscreen(xdg_window->xdg_toplevel);
	}

	if (state & WLF_WINDOW_MAXIMIZED) {
		xdg_toplevel_set_maximized(xdg_window->xdg_toplevel);
	} else {
		xdg_toplevel_unset_maximized(xdg_window->xdg_toplevel);
	}

	if (state & WLF_WINDOW_MINIMIZED) {
		xdg_toplevel_set_minimized(xdg_window->xdg_toplevel);
	}

	wl_surface_commit(xdg_window->surface);
}

static void xdg_window_set_input_region(struct wlf_window *window,
		const struct wlf_region *region) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->surface == NULL) {
		return;
	}

	struct wl_region *wl_region = xdg_window_create_wl_region(xdg_window, region);
	wl_surface_set_input_region(xdg_window->surface, wl_region);
	if (wl_region != NULL) {
		wl_region_destroy(wl_region);
	}
	wl_surface_commit(xdg_window->surface);
}

static void xdg_window_set_opaque_region(struct wlf_window *window,
		const struct wlf_region *region) {
	struct wlf_xdg_window *xdg_window = wlf_xdg_window_from_window(window);
	if (xdg_window == NULL || xdg_window->surface == NULL) {
		return;
	}

	struct wl_region *wl_region = xdg_window_create_wl_region(xdg_window, region);
	wl_surface_set_opaque_region(xdg_window->surface, wl_region);
	if (wl_region != NULL) {
		wl_region_destroy(wl_region);
	}
	wl_surface_commit(xdg_window->surface);
}

static const struct wlf_window_impl xdg_window_impl = {
	.destroy = xdg_window_destroy,
	.close = xdg_window_close,
	.show = xdg_window_show,
	.hide = xdg_window_hide,
	.set_title = xdg_window_set_title,
	.set_geometry = xdg_window_set_geometry,
	.set_size = xdg_window_set_size,
	.set_min_size = xdg_window_set_min_size,
	.set_max_size = xdg_window_set_max_size,
	.set_position = NULL,
	.set_state = xdg_window_set_state,
	.set_flags = NULL,
	.set_input_region = xdg_window_set_input_region,
	.set_opaque_region = xdg_window_set_opaque_region,
	.set_opacity = NULL,
	.set_mask = NULL,
	.set_background_color = NULL,
};

static void xdg_surface_configure(void *data,
		struct xdg_surface *xdg_surface, uint32_t serial) {
	struct wlf_xdg_window *xdg_window = data;
	xdg_surface_ack_configure(xdg_surface, serial);
	wlf_signal_emit_mutable(&xdg_window->base.events.expose, &xdg_window->base);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure,
};

static void xdg_toplevel_configure(void *data,
		struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height,
		struct wl_array *states) {
	(void)xdg_toplevel;
	(void)states;

	struct wlf_xdg_window *xdg_window = data;
	if (width <= 0 || height <= 0) {
		return;
	}

	xdg_window->base.state.geometry.width = width;
	xdg_window->base.state.geometry.height = height;
	wlf_signal_emit_mutable(&xdg_window->base.events.resize, &xdg_window->base);
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel) {
	(void)xdg_toplevel;

	struct wlf_xdg_window *xdg_window = data;
	wlf_window_close(&xdg_window->base);
}

static void xdg_toplevel_wm_capabilities(void *data,
		struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities) {
	(void)data;
	(void)xdg_toplevel;
	(void)capabilities;
}

static void xdg_toplevel_configure_bounds(void *data,
		struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height) {
	(void)data;
	(void)xdg_toplevel;
	(void)width;
	(void)height;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_configure,
	.close = xdg_toplevel_close,
	.configure_bounds = xdg_toplevel_configure_bounds,
	.wm_capabilities = xdg_toplevel_wm_capabilities,
};

bool wlf_window_is_xdg(const struct wlf_window *window) {
	return window != NULL && window->impl == &xdg_window_impl;
}

struct wlf_xdg_window *wlf_xdg_window_from_window(struct wlf_window *window) {
	if (!wlf_window_is_xdg(window)) {
		return NULL;
	}

	return (struct wlf_xdg_window *)window;
}

struct wlf_window *wlf_xdg_window_create_from_backend(
		struct wlf_backend *backend, enum wlf_window_type type,
		uint32_t width, uint32_t height) {
	if (backend == NULL) {
		return NULL;
	}

	if (!wlf_backend_is_wayland(backend)) {
		wlf_log(WLF_ERROR, "wlf_xdg_window requires a Wayland backend");
		return NULL;
	}

	if (type != WLF_WINDOW_TYPE_TOPLEVEL && type != WLF_WINDOW_TYPE_DIALOG &&
			type != WLF_WINDOW_TYPE_TOOLTIP) {
		wlf_log(WLF_ERROR, "xdg_window only supports toplevel-like window types");
		return NULL;
	}

	struct wlf_wl_backend *wayland = wlf_wl_backend_from_backend(backend);
	assert(wayland != NULL);

	if (wayland->registry == NULL || wayland->wl_compositor.compositor == NULL) {
		wlf_log(WLF_ERROR, "Wayland backend is missing registry/compositor");
		return NULL;
	}

	struct wlf_wl_interface *xdg_wm_base_reg =
		wlf_wl_backend_find_interface(wayland, xdg_wm_base_interface.name);
	if (xdg_wm_base_reg == NULL) {
		wlf_log(WLF_ERROR, "Compositor does not expose xdg_wm_base");
		return NULL;
	}

	uint32_t bind_version = xdg_wm_base_reg->version;
	if (bind_version > (uint32_t)xdg_wm_base_interface.version) {
		bind_version = (uint32_t)xdg_wm_base_interface.version;
	}

	struct wlf_xdg_window *xdg_window = calloc(1, sizeof(*xdg_window));
	if (xdg_window == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_xdg_window");
		return NULL;
	}

	wlf_window_init(&xdg_window->base, type, &xdg_window_impl, width, height);
	xdg_window->backend = backend;

	xdg_window->surface = wl_compositor_create_surface(wayland->wl_compositor.compositor);
	if (xdg_window->surface == NULL) {
		wlf_log(WLF_ERROR, "Failed to create wl_surface for xdg_window");
		goto failed;
	}

	xdg_window->xdg_surface = xdg_wm_base_get_xdg_surface(
		wayland->xdg_wm_base.wm_base, xdg_window->surface);
	if (xdg_window->xdg_surface == NULL) {
		wlf_log(WLF_ERROR, "Failed to create xdg_surface");
		goto failed;
	}
	xdg_surface_add_listener(
		xdg_window->xdg_surface, &xdg_surface_listener, xdg_window);

	xdg_window->xdg_toplevel = xdg_surface_get_toplevel(xdg_window->xdg_surface);
	if (xdg_window->xdg_toplevel == NULL) {
		wlf_log(WLF_ERROR, "Failed to create xdg_toplevel");
		goto failed;
	}
	xdg_toplevel_add_listener(
		xdg_window->xdg_toplevel, &xdg_toplevel_listener, xdg_window);

	wl_surface_commit(xdg_window->surface);
	if (wayland->display != NULL) {
		wl_display_roundtrip(wayland->display);
	}

	return &xdg_window->base;

failed:
	xdg_window_destroy(&xdg_window->base);
	return NULL;
}
