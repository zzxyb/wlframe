#include "wlf/wayland/wlf_wl_seat.h"
#include "wlf/types/wlf_seat.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/wayland/wlf_wl_pointer.h"
#include "wlf/wayland/wlf_wl_keyboard.h"
#include "wlf/wayland/wlf_wl_display.h"

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
	seat->base->name = strdup(name);

	if (manager->default_seat == NULL) {
		manager->default_seat = seat;
	}
	wlf_log(WLF_DEBUG, "handle wl_seat name: %s", name);
}

static const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

struct wlf_wl_seat_manager *wlf_wl_seat_manager_create(
		struct wlf_wl_display *display) {
	struct wlf_wl_seat_manager *manager = malloc(sizeof(*manager));
	if (!manager) {
		return NULL;
	}
	manager->display = display;
	manager->default_seat = NULL;
	wlf_linked_list_init(&manager->seats);

	struct wlf_wl_interface *registry;
	wlf_linked_list_for_each(registry, &display->interfaces, link) {
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
			wlf_linked_list_insert(&manager->seats, &wlf_seat->link);
		}
	}

	return manager;
}

void wlf_wl_seat_manager_destroy(
		struct wlf_wl_seat_manager *manager) {
	if (manager == NULL) {
		return;
	}
	struct wlf_wl_seat *seat, *tmp;
	wlf_linked_list_for_each_entry_safe(seat, tmp, &manager->seats, link) {
		wlf_wl_seat_destroy(seat);
	}
	wlf_linked_list_destroy(&manager->seats);
	free(manager);
}

struct wlf_wl_seat *wlf_wl_seat_manager_get_seat(
		struct wlf_wl_seat_manager *manager, const char *name) {
	if (manager == NULL || name == NULL) {
		return NULL;
	}

	struct wlf_wl_seat *seat;
	wlf_linked_list_for_each_entry(seat, &manager->seats, link) {
		if (strcmp(seat->base->name, name) == 0) {
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

	if (manager->default_seat == NULL) {
		// If no default seat is set, use the first seat in the list
		// as the default seat.
		struct wlf_wl_seat *seat;
		wlf_linked_list_for_each_entry(seat, &manager->seats, link) {
			manager->default_seat = seat;
			break;
		}
	}

	return manager->default_seat;
}

struct wlf_wl_seat *wlf_wl_seat_create(struct wl_seat *seat) {
	if (seat == NULL) {
		return NULL;
	}

	struct wlf_wl_seat *wlf_seat = malloc(sizeof(struct wlf_wl_seat));
	if (!wlf_seat) {
		return NULL;
	}

	wlf_seat->seat = seat;
	wlf_linked_list_init(&wlf_seat->link);
	wlf_linked_list_init(&wlf_seat->pointers);
	wlf_linked_list_init(&wlf_seat->keyboards);
	wlf_seat->capabilities = 0;
	wlf_seat->accumulated_capabilities = 0;

	return wlf_seat;
}

void wlf_wl_seat_destroy(struct wlf_wl_seat *seat) {
	if (seat == NULL) {
		return;
	}

	struct wlf_wl_pointer *pointer, *tmp;
	wlf_linked_list_for_each_entry_safe(pointer, tmp, &seat->pointers, link) {
		wlf_wl_pointer_destroy(pointer);
	}
	wlf_linked_list_destroy(&seat->pointers);

	struct wlf_wl_keyboard *keyboard, *tmp_kb;
	wlf_linked_list_for_each_entry_safe(keyboard, tmp_kb, &seat->keyboards, link) {
		wlf_wl_keyboard_destroy(keyboard);
	}
	wlf_linked_list_destroy(&seat->keyboards);

	if (seat->seat != NULL) {
		wl_seat_destroy(seat->seat);
	}
	if (seat->base != NULL) {
		wlf_seat_destroy(seat->base);
	}

	free(seat);
}
