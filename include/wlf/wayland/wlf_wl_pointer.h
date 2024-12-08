#ifndef WLF_WL_POINTER_H
#define WLF_WL_POINTER_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_double_list.h"

#include <wayland-client-protocol.h>

struct wl_pointer;
struct wlf_wl_seat;
struct wlf_pointer;

struct wlf_wl_pointer {
	struct wlf_pointer *base;
	struct wl_pointer *wl_pointer;
	struct wlf_wl_seat *seat;

	struct {
		struct wlf_signal destroy;
	} events;

	struct wlf_double_list link;

	enum wl_pointer_axis_source axis_source;
	enum wl_pointer_axis_relative_direction axis_relative_direction;
	int32_t axis_discrete;
	uint32_t fingers;
};

struct wlf_wl_pointer *create_wlf_wl_pointer(struct wl_pointer *pointer,
	struct wlf_wl_seat *seat);
void wlf_wl_pointer_destroy(struct wlf_wl_pointer *pointer);

#endif
