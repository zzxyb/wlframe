/**
 * @file        wlf_wl_dnd.h
 * @brief       Wayland drag-and-drop implementation for wlframe.
 * @details     This file provides an isolated Wayland DnD object based on
 *              wl_data_device_manager/wl_data_device.
 */

#ifndef CLIPBOARD_WLF_WL_DND_H
#define CLIPBOARD_WLF_WL_DND_H

#include "wlf/platform/wayland/backend.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <wayland-client.h>

struct wl_data_device_manager;
struct wl_data_device;
struct wl_data_source;
struct wl_data_offer;
struct wl_surface;
struct wl_seat;

struct wlf_wl_dnd_data {
	struct wl_data_source *source;
	struct wlf_linked_list entries;
};

struct wlf_wl_dnd {
	struct wlf_backend_wayland *backend;
	struct wl_seat *seat;
	struct wl_data_device_manager *data_device_manager;
	struct wl_data_device *data_device;

	struct wl_data_offer *offer;
	struct wlf_linked_list offer_mime_types;
	struct wlf_wl_dnd_data source_data;

	struct {
		struct wlf_signal enter;
		struct wlf_signal leave;
		struct wlf_signal motion;
		struct wlf_signal drop;
	} events;

	uint32_t serial;
	wl_fixed_t x;
	wl_fixed_t y;
};

void wlf_wl_dnd_init(struct wlf_wl_dnd *dnd,
	struct wlf_backend_wayland *backend, struct wl_seat *seat);
void wlf_wl_dnd_finish(struct wlf_wl_dnd *dnd);

struct wlf_wl_dnd *wlf_wl_dnd_create(struct wlf_backend_wayland *backend,
	struct wl_seat *seat);
void wlf_wl_dnd_destroy(struct wlf_wl_dnd *dnd);

char **wlf_wl_dnd_get_mime_types(struct wlf_wl_dnd *dnd, size_t *count);
void *wlf_wl_dnd_get_data(struct wlf_wl_dnd *dnd,
	const char *mime_type, size_t *size);

bool wlf_wl_dnd_set_data(struct wlf_wl_dnd *dnd,
	const char *mime_type, const void *data, size_t size);
bool wlf_wl_dnd_start_drag(struct wlf_wl_dnd *dnd,
	struct wl_surface *origin, struct wl_surface *icon, uint32_t serial);
void wlf_wl_dnd_clear_data(struct wlf_wl_dnd *dnd);

#endif // CLIPBOARD_WLF_WL_DND_H
