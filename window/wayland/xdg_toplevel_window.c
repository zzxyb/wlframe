#include "wlf/window/wayland/xdg_toplevel_window.h"

#include "wlf/platform/wayland/backend.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/swapchain/wlf_swapchain.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/wayland/wlf_wl_interface.h"
#include "wlf/wayland/wlf_wl_pointer.h"
#include "wlf/wayland/wlf_wl_surface.h"
#include "wlf/wayland/wlf_wp_fractional_scale_v1.h"
#include "wlf/wayland/wlf_wp_viewporter.h"
#include "wlf/wayland/wlf_xdg_wm_base.h"
#include "wlf/wayland/wlf_zxdg_decoration_v1.h"
#include "wayland/protocols/xdg-shell-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void handle_decoration_configure(struct wlf_listener *listener,
	void *data);

static void update_surface_scale(struct wlf_xdg_toplevel_window *window,
		double scale) {
	double old_scale = window->base.state.scale;
	if (window->fractional_scale != NULL && window->viewport != NULL) {
		wlf_wl_surface_set_buffer_scale(window->surface, 1);
		wlf_wp_viewport_set_destination(window->viewport,
			window->base.state.geometry.width,
			window->base.state.geometry.height);
	} else {
		int32_t integer_scale = (int32_t)scale;
		if (integer_scale < 1) {
			integer_scale = 1;
		}
		scale = integer_scale;
		wlf_wl_surface_set_buffer_scale(window->surface, integer_scale);
	}
	wlf_window_set_scale(&window->base, scale);
	if (old_scale != window->base.state.scale) {
		wlf_log(WLF_DEBUG, "Wayland window scale changed from %.2f to %.2f",
			old_scale, window->base.state.scale);
	}
}

static void handle_preferred_buffer_scale(struct wlf_listener *listener,
		void *data) {
	struct wlf_xdg_toplevel_window *window =
		wlf_container_of(listener, window, preferred_buffer_scale);
	struct wlf_wl_surface *surface = data;
	if (window->fractional_scale == NULL) {
		update_surface_scale(window, surface->preferred_buffer_scale);
	}
}

static void handle_preferred_fractional_scale(struct wlf_listener *listener,
		void *data) {
	struct wlf_xdg_toplevel_window *window =
		wlf_container_of(listener, window, preferred_fractional_scale);
	struct wlf_wp_fractional_scale_v1 *fractional_scale = data;
	update_surface_scale(window, fractional_scale->preferred_scale_double);
}

static bool create_scale_objects(struct wlf_xdg_toplevel_window *window) {
	window->preferred_buffer_scale.notify = handle_preferred_buffer_scale;
	wlf_signal_add(&window->surface->events.preferred_buffer_scale,
		&window->preferred_buffer_scale);
	window->has_preferred_buffer_scale_listener = true;

	struct wlf_wl_backend *wayland =
		wlf_wl_backend_from_backend(window->backend);
	if (wayland->wp_fractional_scale_manager_v1.
			fractional_scale_manager_v1 == NULL ||
			wayland->wp_viewporter.viewporter == NULL) {
		update_surface_scale(window, window->surface->preferred_buffer_scale);
		return true;
	}

	window->fractional_scale_manager =
		wlf_wp_fractional_scale_manager_v1_create(wayland->registry,
			wayland->wp_fractional_scale_manager_v1.name,
			wayland->wp_fractional_scale_manager_v1.bind_version);
	window->viewporter = wlf_wp_viewporter_create(wayland->registry,
		wayland->wp_viewporter.name, wayland->wp_viewporter.bind_version);
	if (window->fractional_scale_manager == NULL ||
			window->viewporter == NULL) {
		return false;
	}
	window->fractional_scale =
		wlf_wp_fractional_scale_manager_v1_get_fractional_scale(
			window->fractional_scale_manager, window->surface->wl_surface);
	window->viewport = wlf_wp_viewporter_get_viewport(window->viewporter,
		window->surface->wl_surface);
	if (window->fractional_scale == NULL || window->viewport == NULL) {
		return false;
	}

	window->preferred_fractional_scale.notify =
		handle_preferred_fractional_scale;
	wlf_signal_add(&window->fractional_scale->events.preferred_scale,
		&window->preferred_fractional_scale);
	window->has_preferred_fractional_scale_listener = true;
	update_surface_scale(window, 1.0);
	return true;
}

static void destroy_scale_objects(struct wlf_xdg_toplevel_window *window) {
	if (window->has_preferred_fractional_scale_listener) {
		wlf_linked_list_remove(&window->preferred_fractional_scale.link);
		window->has_preferred_fractional_scale_listener = false;
	}
	if (window->has_preferred_buffer_scale_listener) {
		wlf_linked_list_remove(&window->preferred_buffer_scale.link);
		window->has_preferred_buffer_scale_listener = false;
	}
	wlf_wp_viewport_destroy(window->viewport);
	window->viewport = NULL;
	wlf_wp_fractional_scale_v1_destroy(window->fractional_scale);
	window->fractional_scale = NULL;
	wlf_wp_viewporter_destroy(window->viewporter);
	window->viewporter = NULL;
	wlf_wp_fractional_scale_manager_v1_destroy(
		window->fractional_scale_manager);
	window->fractional_scale_manager = NULL;
}

static struct wlf_xdg_toplevel_window *toplevel_from_window(
		struct wlf_window *window) {
	if (!wlf_window_is_xdg_toplevel(window)) {
		return NULL;
	}

	struct wlf_xdg_toplevel_window *toplevel = NULL;
	return wlf_container_of(window, toplevel, base);
}

static bool update_client_side_decoration(
		struct wlf_xdg_toplevel_window *window) {
	bool client_side = !window->base.state.server_side_decorated &&
		!(window->base.state.state & WLF_WINDOW_FULLSCREEN);
	if (window->base.scene == NULL) {
		return true;
	}
	return wlf_scene_set_client_side_decorated(window->base.scene,
		client_side);
}

static void destroy_toplevel_decoration(
		struct wlf_xdg_toplevel_window *window) {
	if (window->has_decoration_configure_listener) {
		wlf_linked_list_remove(&window->decoration_configure.link);
		window->has_decoration_configure_listener = false;
	}
	wlf_zxdg_toplevel_decoration_v1_destroy(window->decoration);
	window->decoration = NULL;
}

static bool ensure_decoration_manager(
		struct wlf_xdg_toplevel_window *window) {
	if (window->decoration_manager != NULL) {
		return true;
	}
	struct wlf_wl_backend *wayland =
		wlf_wl_backend_from_backend(window->backend);
	window->decoration_manager = wlf_zxdg_decoration_manager_v1_create(
		wayland->registry, wayland->zxdg_decoration_manager_v1.name,
		wayland->zxdg_decoration_manager_v1.bind_version);
	return window->decoration_manager != NULL;
}

static bool request_server_side_decoration(
		struct wlf_xdg_toplevel_window *window) {
	if (window->decoration != NULL) {
		return true;
	}
	if (!ensure_decoration_manager(window)) {
		return false;
	}
	window->decoration =
		wlf_zxdg_decoration_manager_v1_get_toplevel_decoration(
			window->decoration_manager, window->xdg_toplevel->base);
	if (window->decoration == NULL) {
		return false;
	}
	window->decoration_configure.notify = handle_decoration_configure;
	wlf_signal_add(&window->decoration->events.configure,
		&window->decoration_configure);
	window->has_decoration_configure_listener = true;
	wlf_zxdg_toplevel_decoration_v1_set_mode(window->decoration,
		WLF_DECORATION_MODE_SERVER_SIDE);
	return true;
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
	wlf_wl_surface_set_window(window->surface, &window->base);

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
	uint32_t states = toplevel->configure_states;
	bool focused = states & (1u << WLF_XDG_TOPLEVEL_STATE_ACTIVATED);
	enum wlf_window_state_flags state = WLF_WINDOW_NORMAL;
	if (focused) {
		state |= WLF_WINDOW_ACTIVE;
	}
	if (states & (1u << WLF_XDG_TOPLEVEL_STATE_MAXIMIZED)) {
		state |= WLF_WINDOW_MAXIMIZED;
	}
	if (states & (1u << WLF_XDG_TOPLEVEL_STATE_FULLSCREEN)) {
		state |= WLF_WINDOW_FULLSCREEN;
	}
	if (states & (1u << WLF_XDG_TOPLEVEL_STATE_SUSPENDED)) {
		state |= WLF_WINDOW_SUSPENDED;
	}
	bool focus_changed = window->base.state.focused != focused;
	window->base.state.focused = focused;
	window->base.state.state = state;
	if (!update_client_side_decoration(window)) {
		wlf_log(WLF_ERROR, "Failed to update client-side decoration state");
	}
	if (focus_changed) {
		wlf_signal_emit_mutable(focused ? &window->base.events.focus_in :
			&window->base.events.focus_out, &window->base);
	}

	if (toplevel->configure_width <= 0 || toplevel->configure_height <= 0) {
		return;
	}

	window->base.state.geometry.width = toplevel->configure_width;
	window->base.state.geometry.height = toplevel->configure_height;
	if (window->viewport != NULL) {
		wlf_wp_viewport_set_destination(window->viewport,
			toplevel->configure_width, toplevel->configure_height);
	}
	wlf_signal_emit_mutable(&window->base.events.resize, &window->base);
}

static void handle_xdg_toplevel_close(struct wlf_listener *listener,
		void *data) {
	WLF_UNUSED(data);
	struct wlf_xdg_toplevel_window *window =
		wlf_container_of(listener, window, xdg_toplevel_close);
	wlf_window_close(&window->base);
}

static void handle_decoration_configure(struct wlf_listener *listener,
		void *data) {
	struct wlf_xdg_toplevel_window *window =
		wlf_container_of(listener, window, decoration_configure);
	struct wlf_zxdg_toplevel_decoration_v1 *decoration = data;
	bool server_side = !window->force_client_side_decorations &&
		decoration->mode == WLF_DECORATION_MODE_SERVER_SIDE;
	window->base.state.server_side_decorated = server_side;
	if (!update_client_side_decoration(window)) {
		wlf_log(WLF_ERROR, "Failed to update negotiated window decoration");
	}
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
	destroy_toplevel_decoration(window);
	wlf_zxdg_decoration_manager_v1_destroy(window->decoration_manager);
	destroy_scale_objects(window);

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
		if (window->viewport != NULL) {
			wlf_wp_viewport_set_destination(window->viewport, width, height);
		}
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

static void xdg_toplevel_window_begin_move(struct wlf_window *base,
		struct wlf_pointer *pointer, uint32_t serial) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window == NULL || !wlf_pointer_is_wayland(pointer)) {
		return;
	}
	struct wlf_wl_pointer *wl_pointer =
		wlf_wl_pointer_from_pointer(pointer);
	wlf_xdg_toplevel_move(window->xdg_toplevel, wl_pointer->seat, serial);
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

static void xdg_toplevel_window_schedule_frame(struct wlf_window *base) {
	struct wlf_xdg_toplevel_window *window = toplevel_from_window(base);
	if (window != NULL) {
		wlf_wl_surface_schedule_frame(window->surface, base);
	}
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
	.begin_move = xdg_toplevel_window_begin_move,
	.set_state = xdg_toplevel_window_set_state,
	.set_flags = NULL,
	.set_input_region = xdg_toplevel_window_set_input_region,
	.set_opaque_region = xdg_toplevel_window_set_opaque_region,
	.set_opacity = NULL,
	.set_mask = NULL,
	.set_background_color = NULL,
	.native_handle = xdg_toplevel_window_native_handle,
	.schedule_frame = xdg_toplevel_window_schedule_frame,
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
	/* SSD is not active until the compositor confirms it for this toplevel. */
	window->base.state.server_side_decorated = false;
	window->backend = backend;

	if (!create_base_objects(window, backend)) {
		wlf_window_destroy(&window->base);
		return NULL;
	}
	if (!create_scale_objects(window)) {
		wlf_log(WLF_ERROR, "Failed to create Wayland surface scale objects");
		wlf_window_destroy(&window->base);
		return NULL;
	}

	window->xdg_toplevel = wlf_xdg_surface_get_toplevel(window->xdg_surface);
	if (window->xdg_toplevel == NULL) {
		wlf_window_destroy(&window->base);
		return NULL;
	}

	if (wlf_backend_supports_server_side_decorations(backend) &&
			!request_server_side_decoration(window)) {
		wlf_log(WLF_ERROR, "Failed to request server-side decoration");
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

bool wlf_xdg_toplevel_window_set_force_client_side_decorations(
		struct wlf_xdg_toplevel_window *window, bool force) {
	assert(window != NULL);
	if (window->force_client_side_decorations == force) {
		return true;
	}

	window->force_client_side_decorations = force;
	window->base.state.server_side_decorated = false;
	if (force) {
		destroy_toplevel_decoration(window);
		if (window->surface != NULL) {
			wlf_wl_surface_commit(window->surface);
		}
		return update_client_side_decoration(window);
	}

	if (!update_client_side_decoration(window)) {
		return false;
	}
	if (!wlf_backend_supports_server_side_decorations(window->backend)) {
		return true;
	}
	return request_server_side_decoration(window);
}

bool wlf_xdg_toplevel_window_uses_client_side_decorations(
		const struct wlf_xdg_toplevel_window *window) {
	assert(window != NULL);
	return !window->base.state.server_side_decorated &&
		!(window->base.state.state & WLF_WINDOW_FULLSCREEN);
}

struct wlf_titlebar *wlf_xdg_toplevel_window_get_titlebar(
		struct wlf_xdg_toplevel_window *window) {
	assert(window != NULL);
	return window->base.scene != NULL ? window->base.scene->titlebar : NULL;
}

struct wlf_xdg_toplevel_window *wlf_xdg_toplevel_window_from_window(
		struct wlf_window *window) {
	return toplevel_from_window(window);
}
