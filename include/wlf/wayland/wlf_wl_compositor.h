#ifndef WLF_WL_COMPOSITOR_H
#define WLF_WL_COMPOSITOR_H

#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_wl_display;
struct wl_compositor;
struct wl_surface;
struct wl_region;

struct wlf_wl_compositor {
	struct wlf_wl_display *display;
	struct wl_compositor *compositor;

	struct {
		struct wlf_signal destroy;
	} events;

	struct wlf_listener global_add;
	struct wlf_listener global_remove;
};

struct wlf_wl_compositor *wlf_wl_compositor_create(struct wlf_wl_display *display);
bool wlf_wl_compositor_is_nil(struct wlf_wl_compositor *compositor);
void wlf_wl_compositor_destroy(struct wlf_wl_compositor *compositor);
struct wl_surface *wlf_wl_compositor_create_surface(struct wlf_wl_compositor *compositor);
struct wl_region *wlf_wl_compositor_create_region(struct wlf_wl_compositor *compositor);

#endif
