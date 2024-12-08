#ifndef WLF_WL_DISPLAY_H
#define WLF_WL_DISPLAY_H

#include "wlf/utils/wlf_double_list.h"
#include "wlf/utils/wlf_signal.h"

#include <stdint.h>
#include <stdbool.h>

struct wl_display;
struct wl_registry;
struct wl_compositor;
struct xdg_wm_base;

struct wlf_wl_interface {
	struct wlf_double_list link;
	uint32_t name;
	char *interface;
	uint32_t version;
	struct {
		struct wlf_signal destroy;
	} events;
};

struct wlf_wl_display {
	struct wl_display *display;
	struct wl_registry *registry;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal global_add;
		struct wlf_signal global_remove;
	} events;

	struct wlf_double_list interfaces;
};

struct wlf_wl_display *wlf_wl_display_create(void);
bool wlf_wl_display_init_registry(struct wlf_wl_display *display);
void wlf_wl_display_destroy(struct wlf_wl_display *display);
struct wlf_wl_interface *wlf_wl_display_get_registry_from_interface(
	const struct wlf_wl_display *display, const char *interface);

struct wlf_wl_interface *wlf_wl_interface_create(struct wlf_wl_display *display,
	const char *interface, uint32_t version, uint32_t name);
void wlf_wl_registry_destroy(struct wlf_wl_interface *registry);

bool client_interface_version_is_higher(const char *interface,
	uint32_t client_version, uint32_t remote_version);

#endif
