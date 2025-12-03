#include "wlf/wayland/wlf_wl_display.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void display_handle_global(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version) {
	wlf_log(WLF_DEBUG, "Wayland registry global: %s v%" PRIu32, interface, version);
	struct wlf_wl_display *display = data;

	struct wlf_wl_interface *new_reg = wlf_wl_interface_create(display, interface, version, name);
	if (new_reg == NULL) {
		return;
	}

	wlf_linked_list_insert(&display->interfaces, &new_reg->link);
	wlf_signal_emit_mutable(&display->events.global_add, new_reg);
}

static void display_handle_global_remove(void *data,
		struct wl_registry *wl_registry, uint32_t name) {
	struct wlf_wl_display *display = data;
	struct wlf_wl_interface *reg, *tmp;

	wlf_linked_list_for_each_safe(reg, tmp, &display->interfaces, link) {
		if (reg->name == name) {
			wlf_log(WLF_DEBUG, "Interface %s removed", reg->interface);
			wlf_signal_emit_mutable(&display->events.global_remove, reg);
			wlf_wl_interface_destroy(reg);
			break;
		}
	}
}

static const struct wl_registry_listener wl_registry_listener = {
	.global = display_handle_global,
	.global_remove = display_handle_global_remove,
};

struct wlf_wl_display *wlf_wl_display_create(void) {
	struct wlf_wl_display *display = malloc(sizeof(struct wlf_wl_display));
	if (display == NULL) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_wl_display failed!");
		return NULL;
	}

	wlf_linked_list_init(&display->interfaces);
	wlf_signal_init(&display->events.destroy);
	wlf_signal_init(&display->events.global_add);
	wlf_signal_init(&display->events.global_remove);
	display->base = wl_display_connect(NULL);
	if (display->base == NULL) {
		wlf_log(WLF_ERROR, "Failed to connect to Wayland display");
		free(display);
		return NULL;
	}

	return display;
}

bool wlf_wl_display_init_registry(struct wlf_wl_display *display) {
	assert(display);
	display->registry = wl_display_get_registry(display->base);
	if (display->registry == NULL) {
		wlf_log(WLF_ERROR, "Failed to get Wayland registry");
		wl_display_disconnect(display->base);
		free(display);
		return false;
	}
	wl_registry_add_listener(display->registry, &wl_registry_listener, display);
	wl_display_roundtrip(display->base);

	return true;
}

void wlf_wl_display_destroy(struct wlf_wl_display *display) {
	if (display == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&display->events.destroy, display);
	if (display->registry) {
		wl_registry_destroy(display->registry);
	}
	if (display->base) {
		wl_display_disconnect(display->base);
	}

	struct wlf_wl_interface *reg, *tmp;
	wlf_linked_list_for_each_safe(reg, tmp, &display->interfaces, link) {
		wlf_wl_interface_destroy(reg);
	}
	wlf_linked_list_remove(&display->interfaces);
	free(display);
}

struct wlf_wl_interface *wlf_wl_interface_create(struct wlf_wl_display *display,
		const char *interface, uint32_t version, uint32_t name) {
	struct wlf_wl_interface *reg = malloc(sizeof(struct wlf_wl_interface));
	if (reg == NULL) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_wl_interface failed!");
		return NULL;
	}

	wlf_linked_list_init(&reg->link);
	wlf_signal_init(&reg->events.destroy);
	reg->name = name;
	reg->interface = strdup(interface);
	reg->version = version;

	return reg;
}

void wlf_wl_interface_destroy(struct wlf_wl_interface *interface) {
	if (interface == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&interface->events.destroy, interface);
	wlf_linked_list_remove(&interface->link);
	free(interface->interface);
	free(interface);
}

struct wlf_wl_interface *wlf_wl_display_find_interface(
		const struct wlf_wl_display *display, const char *interface) {
	struct wlf_wl_interface *reg;
	wlf_linked_list_for_each(reg, &display->interfaces, link) {
		if (strcmp(reg->interface, interface) == 0) {
			return reg;
		}
	}

	return NULL;
}

bool client_interface_version_is_higher(const char *interface,
		uint32_t client_version, uint32_t remote_version) {
	if (client_version > remote_version) {
		wlf_log(WLF_ERROR, "interface name: %s, Client protocol version: v%" PRIu32 " is higher than Compositor protocol: v%" PRIu32,
				interface, client_version, remote_version);
		wlf_log(WLF_ERROR, "using Compositor protocol version instead Client protocol");

		return true;
	}

	return false;
}
