#include "wlf/wayland/wlf_wl_data_device_manager.h"
#include "wlf/wayland/wlf_wl_data_device.h"
#include "wlf/wayland/wlf_wl_data_source.h"
#include "wlf/wayland/wlf_wl_seat.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

struct wlf_wl_data_device_manager *wlf_wl_data_device_manager_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wl_data_device_manager *manager = calloc(1, sizeof(*manager));
	if (manager == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_data_device_manager");
		return NULL;
	}

	uint32_t bind_version = version;
	if (version > (uint32_t)wl_data_device_manager_interface.version) {
		bind_version = (uint32_t)wl_data_device_manager_interface.version;
	}

	manager->wl_data_device_manager = wl_registry_bind(wl_registry, name,
		&wl_data_device_manager_interface, bind_version);
	if (manager->wl_data_device_manager == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind wl_data_device_manager");
		free(manager);
		return NULL;
	}

	wlf_signal_init(&manager->events.destroy);

	return manager;
}

void wlf_wl_data_device_manager_destroy(
		struct wlf_wl_data_device_manager *manager) {
	if (manager == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	wl_data_device_manager_destroy(manager->wl_data_device_manager);
	free(manager);
}

struct wlf_wl_data_source *wlf_wl_data_device_manager_create_data_source(
		struct wlf_wl_data_device_manager *manager) {
	assert(manager != NULL);
	return wlf_wl_data_source_create(manager);
}

struct wlf_wl_data_device *wlf_wl_data_device_manager_get_data_device(
		struct wlf_wl_data_device_manager *manager,
		struct wlf_wl_seat *seat) {
	assert(manager != NULL);
	assert(seat != NULL);
	return wlf_wl_data_device_create(manager, seat);
}
