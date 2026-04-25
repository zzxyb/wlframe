#ifndef WAYLAND_XDG_WINDOW_H
#define WAYLAND_XDG_WINDOW_H

#include "wlf/window/wlf_window.h"

#include <stdbool.h>
#include <stdint.h>

struct wl_surface;
struct xdg_surface;
struct xdg_toplevel;
struct wlf_backend;

struct wlf_xdg_window {
	struct wlf_window base;
	struct wlf_backend *backend;
	struct wl_surface *surface;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;
};

struct wlf_window *wlf_xdg_window_create_from_backend(
	struct wlf_backend *backend, enum wlf_window_type type,
	uint32_t width, uint32_t height);

bool wlf_window_is_xdg(const struct wlf_window *window);

struct wlf_xdg_window *wlf_xdg_window_from_window(struct wlf_window *window);

#endif // WAYLAND_XDG_WINDOW_H
