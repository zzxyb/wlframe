#ifndef WLF_WL_SEAT_H
#define WLF_WL_SEAT_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_double_list.h"

#include <wayland-client-protocol.h>

struct wl_seat;
struct wlf_wl_display;

struct wlf_wl_seat_manager {
	struct wlf_wl_display *display;
	struct wlf_wl_seat *default_seat;
	struct wlf_double_list seats;
};

struct wlf_wl_seat {
	struct wl_seat *base;
	char *name;

	struct wlf_wl_pointer *active_pointer;

	struct {
		struct wlf_signal destroy;
	} events;

	struct wlf_double_list link;
	struct wlf_double_list pointers;
	struct wlf_double_list keyboards;
};

struct wlf_wl_seat_manager *wlf_wl_seat_manager_create(
	struct wlf_wl_display *display);
void wlf_wl_seat_manager_destroy(
	struct wlf_wl_seat_manager *manager);
struct wlf_wl_seat *wlf_wl_seat_manager_get_seat(
	struct wlf_wl_seat_manager *manager, const char *name);
struct wlf_wl_seat *wlf_wl_seat_manager_get_default_seat(
	struct wlf_wl_seat_manager *manager);
struct wlf_wl_seat *wlf_wl_seat_create(struct wl_seat *seat);
void wlf_wl_seat_destroy(struct wlf_wl_seat *seat);

#endif
