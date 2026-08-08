#include "wlf/window/wayland/xdg_popup_window.h"

#include "wlf/platform/wayland/backend.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/swapchain/wlf_swapchain.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/wayland/wlf_wl_interface.h"
#include "wlf/wayland/wlf_wl_surface.h"
#include "wlf/wayland/wlf_xdg_wm_base.h"
#include "wlf/window/wayland/xdg_dialog_window.h"
#include "wlf/window/wayland/xdg_toplevel_window.h"
#include "wlf/window/wayland/wlr_layer_window.h"
#include "wayland/protocols/xdg-shell-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static struct wlf_xdg_popup_window *popup_from_window(
		struct wlf_window *window) {
	if (!wlf_window_is_xdg_popup(window)) {
		return NULL;
	}

	struct wlf_xdg_popup_window *popup = NULL;
	return wlf_container_of(window, popup, base);
}

static struct wlf_xdg_surface *parent_xdg_surface_from_window(
		struct wlf_window *parent) {
	if (parent == NULL) {
		return NULL;
	}

	struct wlf_xdg_toplevel_window *toplevel =
		wlf_xdg_toplevel_window_from_window(parent);
	if (toplevel != NULL) {
		return toplevel->xdg_surface;
	}

	struct wlf_xdg_popup_window *popup =
		wlf_xdg_popup_window_from_window(parent);
	if (popup != NULL) {
		return popup->xdg_surface;
	}

	struct wlf_xdg_dialog_window *dialog =
		wlf_xdg_dialog_window_from_window(parent);
	if (dialog != NULL) {
		return dialog->xdg_surface;
	}

	return NULL;
}

static bool create_base_objects(struct wlf_xdg_popup_window *window,
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
	struct wlf_xdg_popup_window *window =
		wlf_container_of(listener, window, xdg_surface_configure);
	uint32_t serial = (uint32_t)(uintptr_t)data;

	wlf_xdg_surface_ack_configure(window->xdg_surface, serial);
	if (window->base.state.swapchain != NULL) {
		pixman_region32_t damage;
		pixman_region32_init_rect(&damage, 0, 0,
			window->base.state.geometry.width,
			window->base.state.geometry.height);
		wlf_swapchain_present(window->base.state.swapchain, &damage);
		pixman_region32_fini(&damage);
	}
	wlf_signal_emit_mutable(&window->base.events.expose, &window->base);
}

static void handle_xdg_popup_configure(struct wlf_listener *listener,
		void *data) {
	WLF_UNUSED(data);
	struct wlf_xdg_popup_window *window =
		wlf_container_of(listener, window, xdg_popup_configure);
	struct wlf_xdg_popup *popup = window->xdg_popup;

	window->base.state.geometry.x = popup->configure_x;
	window->base.state.geometry.y = popup->configure_y;
	if (popup->configure_width > 0 && popup->configure_height > 0) {
		window->base.state.geometry.width = popup->configure_width;
		window->base.state.geometry.height = popup->configure_height;
		wlf_signal_emit_mutable(&window->base.events.resize, &window->base);
	}
	wlf_signal_emit_mutable(&window->base.events.move, &window->base);
}

static void handle_xdg_popup_done(struct wlf_listener *listener,
		void *data) {
	WLF_UNUSED(data);
	struct wlf_xdg_popup_window *window =
		wlf_container_of(listener, window, xdg_popup_done);
	wlf_window_close(&window->base);
}

static void xdg_popup_window_destroy(struct wlf_window *base) {
	struct wlf_xdg_popup_window *window = popup_from_window(base);
	if (window == NULL) {
		free(base);
		return;
	}

	if (window->has_xdg_popup_done_listener) {
		wlf_linked_list_remove(&window->xdg_popup_done.link);
	}
	if (window->has_xdg_popup_configure_listener) {
		wlf_linked_list_remove(&window->xdg_popup_configure.link);
	}
	if (window->has_xdg_surface_configure_listener) {
		wlf_linked_list_remove(&window->xdg_surface_configure.link);
	}

	if (window->xdg_popup != NULL) {
		wlf_xdg_popup_destroy(window->xdg_popup);
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

static void xdg_popup_window_close(struct wlf_window *base) {
	struct wlf_xdg_popup_window *window = popup_from_window(base);
	if (window == NULL || window->surface == NULL) {
		return;
	}

	wlf_wl_surface_attach(window->surface, NULL, 0, 0);
	wlf_wl_surface_commit(window->surface);
}

static void xdg_popup_window_show(struct wlf_window *base) {
	struct wlf_xdg_popup_window *window = popup_from_window(base);
	if (window != NULL && window->surface != NULL) {
		wlf_wl_surface_commit(window->surface);
	}
}

static void xdg_popup_window_hide(struct wlf_window *base) {
	xdg_popup_window_close(base);
}

static void xdg_popup_window_set_geometry(struct wlf_window *base,
		const struct wlf_rect *geometry) {
	struct wlf_xdg_popup_window *window = popup_from_window(base);
	if (window != NULL && geometry != NULL) {
		wlf_xdg_surface_set_window_geometry(window->xdg_surface,
			geometry->x, geometry->y, geometry->width, geometry->height);
	}
}

static void xdg_popup_window_set_size(struct wlf_window *base,
		int width, int height) {
	struct wlf_xdg_popup_window *window = popup_from_window(base);
	if (window != NULL) {
		wlf_xdg_surface_set_window_geometry(window->xdg_surface,
			base->state.geometry.x, base->state.geometry.y, width, height);
	}
}

static void xdg_popup_window_set_input_region(struct wlf_window *base,
		const pixman_region32_t *region) {
	struct wlf_xdg_popup_window *window = popup_from_window(base);
	if (window != NULL) {
		wlf_wl_surface_set_input_region(window->surface, region);
		wlf_wl_surface_commit(window->surface);
	}
}

static void xdg_popup_window_set_opaque_region(struct wlf_window *base,
		const pixman_region32_t *region) {
	struct wlf_xdg_popup_window *window = popup_from_window(base);
	if (window != NULL) {
		wlf_wl_surface_set_opaque_region(window->surface, region);
		wlf_wl_surface_commit(window->surface);
	}
}

static void *xdg_popup_window_native_handle(struct wlf_window *base) {
	struct wlf_xdg_popup_window *window = popup_from_window(base);
	return window != NULL ? window->surface->wl_surface : NULL;
}

static const struct wlf_window_impl xdg_popup_window_impl = {
	.destroy = xdg_popup_window_destroy,
	.close = xdg_popup_window_close,
	.show = xdg_popup_window_show,
	.hide = xdg_popup_window_hide,
	.set_title = NULL,
	.set_geometry = xdg_popup_window_set_geometry,
	.set_size = xdg_popup_window_set_size,
	.set_min_size = NULL,
	.set_max_size = NULL,
	.set_position = NULL,
	.set_state = NULL,
	.set_flags = NULL,
	.set_input_region = xdg_popup_window_set_input_region,
	.set_opaque_region = xdg_popup_window_set_opaque_region,
	.set_opacity = NULL,
	.set_mask = NULL,
	.set_background_color = NULL,
	.native_handle = xdg_popup_window_native_handle,
};

struct wlf_window *wlf_xdg_popup_window_create_from_backend(
		struct wlf_backend *backend, struct wlf_window *parent,
		int32_t x, int32_t y, uint32_t width, uint32_t height) {
	if (backend == NULL || !wlf_backend_is_wayland(backend)) {
		wlf_log(WLF_ERROR, "xdg_popup_window requires a Wayland backend");
		return NULL;
	}
	if (parent != NULL && parent->state.backend != backend) {
		wlf_log(WLF_ERROR,
			"xdg_popup_window parent belongs to a different backend");
		return NULL;
	}

	struct wlf_xdg_surface *parent_surface =
		parent_xdg_surface_from_window(parent);
	struct wlf_wlr_layer_window *layer_parent =
		wlf_wlr_layer_window_from_window(parent);
	if (parent != NULL && parent_surface == NULL && layer_parent == NULL) {
		wlf_log(WLF_ERROR,
			"xdg_popup_window parent is not an xdg or layer window");
		return NULL;
	}

	struct wlf_xdg_popup_window *window = calloc(1, sizeof(*window));
	if (window == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_xdg_popup_window");
		return NULL;
	}

	wlf_window_init(&window->base, WLF_WINDOW_TYPE_POPUP,
		&xdg_popup_window_impl, backend, width, height);
	window->backend = backend;
	window->base.state.geometry.x = x;
	window->base.state.geometry.y = y;

	if (!create_base_objects(window, backend)) {
		wlf_window_destroy(&window->base);
		return NULL;
	}

	struct wlf_xdg_positioner *positioner =
		wlf_xdg_wm_base_create_positioner(window->wm_base);
	if (positioner == NULL) {
		wlf_window_destroy(&window->base);
		return NULL;
	}
	wlf_xdg_positioner_set_size(positioner, (int32_t)width, (int32_t)height);
	wlf_xdg_positioner_set_anchor_rect(positioner, x, y, 1, 1);
	wlf_xdg_positioner_set_anchor(positioner, WLF_XDG_POSITIONER_ANCHOR_TOP_LEFT);
	wlf_xdg_positioner_set_gravity(positioner,
		WLF_XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);

	window->xdg_popup = wlf_xdg_surface_get_popup(window->xdg_surface,
		parent_surface, positioner);
	wlf_xdg_positioner_destroy(positioner);
	if (window->xdg_popup == NULL) {
		wlf_window_destroy(&window->base);
		return NULL;
	}
	if (layer_parent != NULL) {
		/* Layer-shell popups use a NULL xdg parent and are associated with
		 * their parent by zwlr_layer_surface_v1.get_popup before the first
		 * popup commit. */
		wlf_zwlr_layer_surface_v1_get_popup(layer_parent->layer_surface,
			window->xdg_popup->base);
	}

	window->xdg_surface_configure.notify = handle_xdg_surface_configure;
	wlf_signal_add(&window->xdg_surface->events.configure,
		&window->xdg_surface_configure);
	window->has_xdg_surface_configure_listener = true;
	window->xdg_popup_configure.notify = handle_xdg_popup_configure;
	wlf_signal_add(&window->xdg_popup->events.configure,
		&window->xdg_popup_configure);
	window->has_xdg_popup_configure_listener = true;
	window->xdg_popup_done.notify = handle_xdg_popup_done;
	wlf_signal_add(&window->xdg_popup->events.popup_done,
		&window->xdg_popup_done);
	window->has_xdg_popup_done_listener = true;

	return &window->base;
}

bool wlf_window_is_xdg_popup(const struct wlf_window *window) {
	return window != NULL && window->impl == &xdg_popup_window_impl;
}

struct wlf_xdg_popup_window *wlf_xdg_popup_window_from_window(
		struct wlf_window *window) {
	return popup_from_window(window);
}
