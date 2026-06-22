#include "wlf/wayland/wlf_xdg_wm_base.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_utils.h"
#include "wayland/protocols/xdg-shell-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
		uint32_t serial) {

	struct wlf_xdg_wm_base *wm_base = data;
	xdg_wm_base_pong(xdg_wm_base, serial);
	wlf_signal_emit_mutable(&wm_base->events.ping, (void *)(uintptr_t)serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {
	.ping = wm_base_ping,
};

static void surface_configure(void *data, struct xdg_surface *xdg_surface,
		uint32_t serial) {
	WLF_UNUSED(xdg_surface);

	struct wlf_xdg_surface *surface = data;
	surface->configure_serial = serial;
	surface->has_pending_configure = true;
	wlf_signal_emit_mutable(&surface->events.configure,
		(void *)(uintptr_t)serial);
}

static const struct xdg_surface_listener surface_listener = {
	.configure = surface_configure,
};

static void toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel,
		int32_t width, int32_t height, struct wl_array *states) {
	WLF_UNUSED(xdg_toplevel);

	struct wlf_xdg_toplevel *toplevel = data;
	toplevel->configure_width  = width;
	toplevel->configure_height = height;

	uint32_t mask = 0;
	uint32_t *s;
	wl_array_for_each(s, states) {
		if (*s < 32) {
			mask |= (1u << *s);
		}
	}

	toplevel->configure_states = mask;
	wlf_signal_emit_mutable(&toplevel->events.configure, toplevel);
}

static void toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel) {
	WLF_UNUSED(xdg_toplevel);

	struct wlf_xdg_toplevel *toplevel = data;
	wlf_signal_emit_mutable(&toplevel->events.close, toplevel);
}

static void toplevel_configure_bounds(void *data,
		struct xdg_toplevel *xdg_toplevel,
		int32_t width, int32_t height) {
	WLF_UNUSED(xdg_toplevel);

	struct wlf_xdg_toplevel *toplevel = data;
	toplevel->bounds_width  = width;
	toplevel->bounds_height = height;
	wlf_signal_emit_mutable(&toplevel->events.configure_bounds, toplevel);
}

static void toplevel_wm_capabilities(void *data,
		struct xdg_toplevel *xdg_toplevel,
		struct wl_array *capabilities) {
	WLF_UNUSED(xdg_toplevel);

	struct wlf_xdg_toplevel *toplevel = data;

	uint32_t mask = 0;
	uint32_t *c;
	wl_array_for_each(c, capabilities) {
		if (*c < 32) {
			mask |= (1u << *c);
		}
	}
	toplevel->wm_capabilities = mask;
	wlf_signal_emit_mutable(&toplevel->events.wm_capabilities, toplevel);
}

static const struct xdg_toplevel_listener toplevel_listener = {
	.configure        = toplevel_configure,
	.close            = toplevel_close,
	.configure_bounds = toplevel_configure_bounds,
	.wm_capabilities  = toplevel_wm_capabilities,
};

static void popup_configure(void *data, struct xdg_popup *xdg_popup,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	WLF_UNUSED(xdg_popup);

	struct wlf_xdg_popup *popup = data;
	popup->configure_x      = x;
	popup->configure_y      = y;
	popup->configure_width  = width;
	popup->configure_height = height;
	wlf_signal_emit_mutable(&popup->events.configure, popup);
}

static void popup_done(void *data, struct xdg_popup *xdg_popup) {
	WLF_UNUSED(xdg_popup);

	struct wlf_xdg_popup *popup = data;
	wlf_signal_emit_mutable(&popup->events.popup_done, popup);
}

static void popup_repositioned(void *data, struct xdg_popup *xdg_popup,
		uint32_t token) {
	WLF_UNUSED(xdg_popup);

	struct wlf_xdg_popup *popup = data;
	popup->repositioned_token = token;
	wlf_signal_emit_mutable(&popup->events.repositioned,
		(void *)(uintptr_t)token);
}

static const struct xdg_popup_listener popup_listener = {
	.configure    = popup_configure,
	.popup_done   = popup_done,
	.repositioned = popup_repositioned,
};

struct wlf_xdg_wm_base *wlf_xdg_wm_base_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_xdg_wm_base *wm_base = malloc(sizeof(struct wlf_xdg_wm_base));
	if (wm_base == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_xdg_wm_base");
		return NULL;
	}

	wm_base->base = NULL;
	wlf_signal_init(&wm_base->events.ping);
	wlf_signal_init(&wm_base->events.destroy);

	uint32_t bind_version = version;
	if (version > (uint32_t)xdg_wm_base_interface.version) {
		wlf_log(WLF_DEBUG,
			"Server xdg_wm_base version %u is higher than client "
			"version %u, using client version",
			version, (uint32_t)xdg_wm_base_interface.version);
		bind_version = (uint32_t)xdg_wm_base_interface.version;
	}

	wm_base->base = wl_registry_bind(wl_registry, name,
		&xdg_wm_base_interface, bind_version);
	if (wm_base->base == NULL) {
		wlf_log(WLF_ERROR,
			"Failed to bind xdg_wm_base interface (name: %u)", name);
		free(wm_base);
		return NULL;
	}

	wm_base->version = bind_version;
	xdg_wm_base_add_listener(wm_base->base, &wm_base_listener, wm_base);

	wlf_log(WLF_DEBUG,
		"Successfully bound xdg_wm_base (name: %u, version: %u)",
		name, bind_version);

	return wm_base;
}

void wlf_xdg_wm_base_destroy(struct wlf_xdg_wm_base *wm_base) {
	if (wm_base == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&wm_base->events.destroy, wm_base);

	if (wm_base->base != NULL) {
		xdg_wm_base_destroy(wm_base->base);
		wm_base->base = NULL;
	}

	free(wm_base);
}

struct wlf_xdg_positioner *wlf_xdg_wm_base_create_positioner(
		struct wlf_xdg_wm_base *wm_base) {
	assert(wm_base != NULL);
	assert(wm_base->base != NULL);

	struct wlf_xdg_positioner *positioner =
		malloc(sizeof(struct wlf_xdg_positioner));
	if (positioner == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_xdg_positioner");
		return NULL;
	}

	positioner->base = xdg_wm_base_create_positioner(wm_base->base);
	if (positioner->base == NULL) {
		wlf_log(WLF_ERROR, "xdg_wm_base_create_positioner() returned NULL");
		free(positioner);
		return NULL;
	}
	positioner->version =
		wl_proxy_get_version((struct wl_proxy *)positioner->base);

	return positioner;
}

struct wlf_xdg_surface *wlf_xdg_wm_base_get_xdg_surface(
		struct wlf_xdg_wm_base *wm_base, struct wl_surface *surface) {
	assert(wm_base != NULL);
	assert(wm_base->base != NULL);
	assert(surface != NULL);

	struct wlf_xdg_surface *xdg_surface =
		malloc(sizeof(struct wlf_xdg_surface));
	if (xdg_surface == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_xdg_surface");
		return NULL;
	}

	xdg_surface->base = xdg_wm_base_get_xdg_surface(wm_base->base, surface);
	if (xdg_surface->base == NULL) {
		wlf_log(WLF_ERROR,
			"xdg_wm_base_get_xdg_surface() returned NULL");
		free(xdg_surface);
		return NULL;
	}

	wlf_signal_init(&xdg_surface->events.configure);
	wlf_signal_init(&xdg_surface->events.destroy);
	xdg_surface->version =
		wl_proxy_get_version((struct wl_proxy *)xdg_surface->base);
	xdg_surface->configure_serial = 0;
	xdg_surface->acked_configure_serial = 0;
	xdg_surface->has_pending_configure = false;

	xdg_surface_add_listener(xdg_surface->base, &surface_listener,
		xdg_surface);

	return xdg_surface;
}

void wlf_xdg_positioner_set_size(struct wlf_xdg_positioner *positioner,
		int32_t width, int32_t height) {
	assert(positioner != NULL && positioner->base != NULL);

	xdg_positioner_set_size(positioner->base, width, height);
}

void wlf_xdg_positioner_set_anchor_rect(struct wlf_xdg_positioner *positioner,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	assert(positioner != NULL && positioner->base != NULL);

	xdg_positioner_set_anchor_rect(positioner->base, x, y, width, height);
}

void wlf_xdg_positioner_set_anchor(struct wlf_xdg_positioner *positioner,
		enum wlf_xdg_positioner_anchor anchor) {
	assert(positioner != NULL && positioner->base != NULL);

	xdg_positioner_set_anchor(positioner->base, (uint32_t)anchor);
}

void wlf_xdg_positioner_set_gravity(struct wlf_xdg_positioner *positioner,
		enum wlf_xdg_positioner_gravity gravity) {
	assert(positioner != NULL && positioner->base != NULL);

	xdg_positioner_set_gravity(positioner->base, (uint32_t)gravity);
}

void wlf_xdg_positioner_set_constraint_adjustment(
		struct wlf_xdg_positioner *positioner,
		uint32_t constraint_adjustment) {
	assert(positioner != NULL && positioner->base != NULL);

	xdg_positioner_set_constraint_adjustment(positioner->base,
		constraint_adjustment);
}

void wlf_xdg_positioner_set_offset(struct wlf_xdg_positioner *positioner,
		int32_t x, int32_t y) {
	assert(positioner != NULL && positioner->base != NULL);

	xdg_positioner_set_offset(positioner->base, x, y);
}

void wlf_xdg_positioner_set_reactive(struct wlf_xdg_positioner *positioner) {
	assert(positioner != NULL && positioner->base != NULL);

	if (positioner->version < XDG_POSITIONER_SET_REACTIVE_SINCE_VERSION) {
		wlf_log(WLF_ERROR, "xdg_positioner.set_reactive requires version 3");
		return;
	}
	xdg_positioner_set_reactive(positioner->base);
}

void wlf_xdg_positioner_set_parent_size(struct wlf_xdg_positioner *positioner,
		int32_t parent_width, int32_t parent_height) {
	assert(positioner != NULL && positioner->base != NULL);

	if (positioner->version < XDG_POSITIONER_SET_PARENT_SIZE_SINCE_VERSION) {
		wlf_log(WLF_ERROR, "xdg_positioner.set_parent_size requires version 3");
		return;
	}
	xdg_positioner_set_parent_size(positioner->base,
		parent_width, parent_height);
}

void wlf_xdg_positioner_set_parent_configure(
		struct wlf_xdg_positioner *positioner, uint32_t serial) {
	assert(positioner != NULL && positioner->base != NULL);

	if (positioner->version < XDG_POSITIONER_SET_PARENT_CONFIGURE_SINCE_VERSION) {
		wlf_log(WLF_ERROR,
			"xdg_positioner.set_parent_configure requires version 3");
		return;
	}
	xdg_positioner_set_parent_configure(positioner->base, serial);
}

void wlf_xdg_positioner_destroy(struct wlf_xdg_positioner *positioner) {
	if (positioner == NULL) {
		return;
	}

	if (positioner->base != NULL) {
		xdg_positioner_destroy(positioner->base);
		positioner->base = NULL;
	}

	free(positioner);
}

struct wlf_xdg_toplevel *wlf_xdg_surface_get_toplevel(
		struct wlf_xdg_surface *surface) {
	assert(surface != NULL);
	assert(surface->base != NULL);

	struct wlf_xdg_toplevel *toplevel =
		calloc(1, sizeof(struct wlf_xdg_toplevel));
	if (toplevel == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_xdg_toplevel");
		return NULL;
	}

	toplevel->base = xdg_surface_get_toplevel(surface->base);
	if (toplevel->base == NULL) {
		wlf_log(WLF_ERROR, "xdg_surface_get_toplevel() returned NULL");
		free(toplevel);
		return NULL;
	}

	wlf_signal_init(&toplevel->events.configure);
	wlf_signal_init(&toplevel->events.close);
	wlf_signal_init(&toplevel->events.configure_bounds);
	wlf_signal_init(&toplevel->events.wm_capabilities);
	wlf_signal_init(&toplevel->events.destroy);
	toplevel->version =
		wl_proxy_get_version((struct wl_proxy *)toplevel->base);

	xdg_toplevel_add_listener(toplevel->base, &toplevel_listener, toplevel);

	return toplevel;
}

struct wlf_xdg_popup *wlf_xdg_surface_get_popup(
		struct wlf_xdg_surface *surface,
		struct wlf_xdg_surface *parent,
		struct wlf_xdg_positioner *positioner) {
	assert(surface != NULL && surface->base != NULL);
	assert(positioner != NULL && positioner->base != NULL);

	struct wlf_xdg_popup *popup = calloc(1, sizeof(struct wlf_xdg_popup));
	if (popup == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_xdg_popup");
		return NULL;
	}

	struct xdg_surface *parent_base = parent ? parent->base : NULL;
	popup->base = xdg_surface_get_popup(surface->base, parent_base,
		positioner->base);
	if (popup->base == NULL) {
		wlf_log(WLF_ERROR, "xdg_surface_get_popup() returned NULL");
		free(popup);
		return NULL;
	}

	wlf_signal_init(&popup->events.configure);
	wlf_signal_init(&popup->events.popup_done);
	wlf_signal_init(&popup->events.repositioned);
	wlf_signal_init(&popup->events.destroy);
	popup->version = wl_proxy_get_version((struct wl_proxy *)popup->base);

	xdg_popup_add_listener(popup->base, &popup_listener, popup);
	return popup;
}

void wlf_xdg_surface_set_window_geometry(struct wlf_xdg_surface *surface,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	assert(surface != NULL && surface->base != NULL);

	xdg_surface_set_window_geometry(surface->base, x, y, width, height);
}

void wlf_xdg_surface_ack_configure(struct wlf_xdg_surface *surface,
		uint32_t serial) {
	assert(surface != NULL && surface->base != NULL);

	xdg_surface_ack_configure(surface->base, serial);
	surface->acked_configure_serial = serial;
	if (surface->configure_serial == serial) {
		surface->has_pending_configure = false;
	}
}

void wlf_xdg_surface_destroy(struct wlf_xdg_surface *surface) {
	if (surface == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&surface->events.destroy, surface);
	assert(wlf_linked_list_empty(&surface->events.destroy.listener_list));
	if (surface->base != NULL) {
		xdg_surface_destroy(surface->base);
		surface->base = NULL;
	}

	free(surface);
}

bool wlf_xdg_toplevel_has_state(const struct wlf_xdg_toplevel *toplevel,
		enum wlf_xdg_toplevel_state state) {
	if (toplevel == NULL || state >= 32) {
		return false;
	}

	return (toplevel->configure_states & (1u << state)) != 0;
}

bool wlf_xdg_toplevel_has_wm_capability(
		const struct wlf_xdg_toplevel *toplevel,
		enum wlf_xdg_toplevel_wm_capabilities capability) {
	if (toplevel == NULL || capability >= 32) {
		return false;
	}

	return (toplevel->wm_capabilities & (1u << capability)) != 0;
}

void wlf_xdg_toplevel_set_parent(struct wlf_xdg_toplevel *toplevel,
		struct wlf_xdg_toplevel *parent) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_set_parent(toplevel->base,
		parent ? parent->base : NULL);
}

void wlf_xdg_toplevel_set_title(struct wlf_xdg_toplevel *toplevel,
		const char *title) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_set_title(toplevel->base, title);
}

void wlf_xdg_toplevel_set_app_id(struct wlf_xdg_toplevel *toplevel,
		const char *app_id) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_set_app_id(toplevel->base, app_id);
}

void wlf_xdg_toplevel_show_window_menu(struct wlf_xdg_toplevel *toplevel,
		struct wl_seat *seat, uint32_t serial, int32_t x, int32_t y) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_show_window_menu(toplevel->base, seat, serial, x, y);
}

void wlf_xdg_toplevel_move(struct wlf_xdg_toplevel *toplevel,
		struct wl_seat *seat, uint32_t serial) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_move(toplevel->base, seat, serial);
}

void wlf_xdg_toplevel_resize(struct wlf_xdg_toplevel *toplevel,
		struct wl_seat *seat, uint32_t serial,
		enum wlf_xdg_toplevel_resize_edge edge) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_resize(toplevel->base, seat, serial, (uint32_t)edge);
}

void wlf_xdg_toplevel_set_max_size(struct wlf_xdg_toplevel *toplevel,
		int32_t width, int32_t height) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_set_max_size(toplevel->base, width, height);
}

void wlf_xdg_toplevel_set_min_size(struct wlf_xdg_toplevel *toplevel,
		int32_t width, int32_t height) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_set_min_size(toplevel->base, width, height);
}

void wlf_xdg_toplevel_set_maximized(struct wlf_xdg_toplevel *toplevel) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_set_maximized(toplevel->base);
}

void wlf_xdg_toplevel_unset_maximized(struct wlf_xdg_toplevel *toplevel) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_unset_maximized(toplevel->base);
}

void wlf_xdg_toplevel_set_fullscreen(struct wlf_xdg_toplevel *toplevel,
		struct wl_output *output) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_set_fullscreen(toplevel->base, output);
}

void wlf_xdg_toplevel_unset_fullscreen(struct wlf_xdg_toplevel *toplevel) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_unset_fullscreen(toplevel->base);
}

void wlf_xdg_toplevel_set_minimized(struct wlf_xdg_toplevel *toplevel) {
	assert(toplevel != NULL && toplevel->base != NULL);

	xdg_toplevel_set_minimized(toplevel->base);
}

void wlf_xdg_toplevel_destroy(struct wlf_xdg_toplevel *toplevel) {
	if (toplevel == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&toplevel->events.destroy, toplevel);
	assert(wlf_linked_list_empty(&toplevel->events.destroy.listener_list));
	if (toplevel->base != NULL) {
		xdg_toplevel_destroy(toplevel->base);
		toplevel->base = NULL;
	}

	free(toplevel);
}

void wlf_xdg_popup_grab(struct wlf_xdg_popup *popup,
		struct wl_seat *seat, uint32_t serial) {
	assert(popup != NULL && popup->base != NULL);

	xdg_popup_grab(popup->base, seat, serial);
}

void wlf_xdg_popup_reposition(struct wlf_xdg_popup *popup,
		struct wlf_xdg_positioner *positioner, uint32_t token) {
	assert(popup != NULL && popup->base != NULL);
	assert(positioner != NULL && positioner->base != NULL);

	if (popup->version < XDG_POPUP_REPOSITION_SINCE_VERSION) {
		wlf_log(WLF_ERROR, "xdg_popup.reposition requires version 3");
		return;
	}
	xdg_popup_reposition(popup->base, positioner->base, token);
}

void wlf_xdg_popup_destroy(struct wlf_xdg_popup *popup) {
	if (popup == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&popup->events.destroy, popup);
	assert(wlf_linked_list_empty(&popup->events.destroy.listener_list));
	if (popup->base != NULL) {
		xdg_popup_destroy(popup->base);
		popup->base = NULL;
	}

	free(popup);
}
