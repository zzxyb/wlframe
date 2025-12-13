#include "wlf/wayland/wlf_wl_interface.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/platform/wayland/backend.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <wayland-client-protocol.h>

struct wlf_wl_interface *wlf_wl_interface_create(const char *interface,
		uint32_t version, uint32_t name) {
	struct wlf_wl_interface *reg = malloc(sizeof(struct wlf_wl_interface));
	if (reg == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_interface");
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

struct wlf_wl_interface *wlf_wl_backend_find_interface(
		const struct wlf_backend_wayland *backend, const char *interface) {
	struct wlf_wl_interface *reg;
	wlf_linked_list_for_each(reg, &backend->interfaces, link) {
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
