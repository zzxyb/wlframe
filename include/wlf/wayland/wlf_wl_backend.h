#ifndef WLF_WL_BACKEND_H
#define WLF_WL_BACKEND_H

#include "wlf/types/wlf_backend.h"

#include <wayland-client-protocol.h>
#include <wayland-client-core.h>

struct wlf_wl_backend {
	struct wlf_backend backend;

	bool started;
	struct wl_event_loop *event_loop;
	struct wl_list outputs;
	int drm_fd;
	struct wl_list buffers;
	size_t requested_outputs;
	struct wl_listener event_loop_destroy;
	char *activation_token;

	struct wl_display *remote_display;
	struct wl_event_source *remote_display_src;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	// struct xdg_wm_base *xdg_wm_base;
	// struct zxdg_decoration_manager_v1 *zxdg_decoration_manager_v1;
	// struct zwp_pointer_gestures_v1 *zwp_pointer_gestures_v1;
	// struct wp_presentation *presentation;
	// struct wl_shm *shm;
	// struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf_v1;
	// struct zwp_relative_pointer_manager_v1 *zwp_relative_pointer_manager_v1;
	// struct wl_list seats; // wlf_wl_seat.link
	// struct zwp_tablet_manager_v2 *tablet_manager;
	// struct wlf_drm_format_set shm_formats;
	// struct wlf_drm_format_set linux_dmabuf_v1_formats;
	// struct wl_drm *legacy_drm;
	// struct xdg_activation_v1 *activation_v1;
	// struct wl_subcompositor *subcompositor;
	// struct wp_viewporter *viewporter;
	// char *drm_render_name;
};

struct wlf_backend *wlf_wl_backend_create(struct wl_event_loop *loop);
bool wlf_backend_is_wl(struct wlf_backend *backend);
struct wlf_wl_backend *get_wl_backend_from_backend(struct wlf_backend *wlf_backend);

#endif
