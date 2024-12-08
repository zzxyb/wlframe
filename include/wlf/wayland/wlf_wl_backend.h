#ifndef WLF_WL_BACKEND_H
#define WLF_WL_BACKEND_H

#include "wlf/backend/wlf_backend.h"
#include "wlf_wl_display.h"

struct wlf_wl_display;

struct wlf_wl_backend {
	bool started;
	int drm_fd;
	struct wlf_backend backend;

	struct wlf_wl_display *display;
};

struct wlf_backend *wlf_wl_backend_create(void);
bool wlf_backend_is_wl(struct wlf_backend *backend);
struct wlf_wl_backend *get_wl_backend_from_backend(struct wlf_backend *wlf_backend);

#endif
