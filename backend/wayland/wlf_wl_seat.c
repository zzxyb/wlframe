#include "wlf/wayland/wlf_wl_seat.h"
#include "wlf/wayland/wlf_wl_pointer.h"
#include "wlf/wayland/wlf_wl_keyboard.h"
#include "wlf/wayland/wlf_wl_display.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat,
		uint32_t capabilities) {
	WLF_UNUSED(data);
	struct wlf_wl_seat *seat = wlf_container_of(wl_seat, seat, base);

	if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
		struct wl_pointer *pointer = wl_seat_get_pointer(wl_seat);
		if (pointer == NULL) {
			wlf_log(WLF_ERROR, "Failed to get wl_pointer!");
			return;
		}
		struct wlf_wl_pointer *wlf_pointer = create_wlf_wl_pointer(pointer, seat);
		if (pointer == NULL) {
			wlf_log(WLF_ERROR, "Failed to create pointer!");
			return;
		}
		wlf_double_list_insert(&seat->pointers, &wlf_pointer->link);
	}

	// if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
	// 	struct wlf_wl_keyboard *keyboard = create_wlf_wl_keyboard(wl_seat);
	// 	if (keyboard == NULL) {
	// 		wlf_log(WLF_ERROR, "Failed to create keyboard!");
	// 		return;
	// 	}
	// 	wlf_double_list_insert(&seat->keyboards, &keyboard->link);
	// } else {
	// 	wlf_double_list_remove(&seat->keyboards);
	// }
}

static void seat_handle_name(void *data, struct wl_seat *wl_seat,
		const char *name) {
	struct wlf_wl_seat_manager *manager = data;
	struct wlf_wl_seat *seat = wlf_container_of(wl_seat, seat, base);
	seat->name = strdup(name);

	if (manager->default_seat == NULL) {
		manager->default_seat = seat;
	}
	wlf_log(WLF_DEBUG, "Seat name: %s", name);
}

static const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

struct wlf_wl_seat_manager *wlf_wl_seat_manager_create(
		struct wlf_wl_display *display) {
	struct wlf_wl_seat_manager *manager = malloc(sizeof(struct wlf_wl_seat_manager));
	if (!manager) {
		wlf_log(WLF_ERROR, "Allocation wlf_wl_seat_manager failed");
		return NULL;
	}

	manager->display = display;
	wlf_double_list_init(&manager->seats);
	manager->default_seat = NULL;

	struct wlf_wl_interface *registry;
	wlf_double_list_for_each(registry, &display->interfaces, link) {
		if (strcmp(registry->interface, wl_seat_interface.name) == 0) {
			client_interface_version_is_higher(wl_seat_interface.name, registry->version,
					wl_seat_interface.version);
			struct wl_seat *seat = wl_registry_bind(display->registry, registry->name,
					&wl_seat_interface, wl_seat_interface.version);
			if (seat == NULL) {
				wlf_log(WLF_ERROR, "Failed to bind wl_seat!");
				free(manager);
				return NULL;
			}
			wl_seat_add_listener(seat, &seat_listener, manager);

			struct wlf_wl_seat *wlf_seat = wlf_wl_seat_create(seat);
			if (wlf_seat == NULL) {
				wl_seat_destroy(seat);
				free(manager);
				return NULL;
			}
			wlf_seat->base = seat;
			wlf_double_list_insert(&manager->seats, &wlf_seat->link);
		}
	}

	return manager;
}

void wlf_wl_seat_manager_destroy(
		struct wlf_wl_seat_manager *manager) {
	if (manager == NULL) {
		return;
	}

	struct wlf_wl_seat *seat;
	wlf_double_list_for_each(seat, &manager->seats, link) {
		wlf_wl_seat_destroy(seat);
	}

	wlf_double_list_remove(&manager->seats);
	free(manager);
}

struct wlf_wl_seat *wlf_wl_seat_manager_get_seat(
		struct wlf_wl_seat_manager *manager, const char *name) {
	if (manager == NULL) {
		return NULL;
	}

	struct wlf_wl_seat *seat;
	wlf_double_list_for_each(seat, &manager->seats, link) {
		if (strcmp(seat->name, name) == 0) {
			return seat;
		}
	}

	return NULL;
}

struct wlf_wl_seat *wlf_wl_seat_manager_get_default_seat(
		struct wlf_wl_seat_manager *manager) {
	if (manager == NULL) {
		return NULL;
	}

	return manager->default_seat;
}

struct wlf_wl_seat *wlf_wl_seat_create(struct wl_seat *seat) {
	struct wlf_wl_seat *wlf_seat = malloc(sizeof(struct wlf_wl_seat));
	if (wlf_seat == NULL) {
		wlf_log(WLF_ERROR, "Allocation wlf_wl_seat failed");
		return NULL;
	}

	wlf_double_list_init(&wlf_seat->link);
	wlf_double_list_init(&wlf_seat->pointers);
	wlf_double_list_init(&wlf_seat->keyboards);
	wlf_signal_init(&wlf_seat->events.destroy);
	wlf_seat->base = seat;

	return wlf_seat;
}

void wlf_wl_seat_destroy(struct wlf_wl_seat *seat) {
	if (seat == NULL) {
		return;
	}

	wlf_signal_emit(&seat->events.destroy, seat);

	struct wlf_wl_pointer *pointer;
	wlf_double_list_for_each(pointer, &seat->pointers, link) {
		wlf_wl_pointer_destroy(pointer);
	}

	// struct wlf_wl_keyboard *keyboard;
	// wlf_double_list_for_each(keyboard, &seat->keyboards, link) {
	// 	wlf_wl_keyboard_destroy(keyboard);
	// }

	if (seat->base) {
		wl_seat_destroy(seat->base);
		seat->base = NULL;
	}

	wlf_double_list_remove(&seat->link);
	free(seat);
}
