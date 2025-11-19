#ifndef VA_WAYLAND_DISPLAY_H
#define VA_WAYLAND_DISPLAY_H

#include "wlf/va/wlf_va_display.h"
#include "wlf/platform/wayland/backend.h"

struct wlf_wl_va_display {
	struct wlf_va_display base;
};

struct wlf_va_display *wl_va_display_create(struct wlf_backend_wayland *backend);
bool wlf_va_display_is_wayland(const struct wlf_va_display *display);
struct wlf_wl_va_display *wlf_wl_va_display_from_va_display(struct wlf_va_display *display);

#endif // VA_WAYLAND_DISPLAY_H
