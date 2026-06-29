#include "wlf/wayland/wlf_wp_cursor_shape_manager_v1.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"
#include "wayland/protocols/cursor-shape-v1-client-protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

struct wlf_wp_cursor_shape_manager_v1 *wlf_wp_cursor_shape_manager_v1_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wp_cursor_shape_manager_v1 *manager =
		malloc(sizeof(struct wlf_wp_cursor_shape_manager_v1));
	if (manager == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate wlf_wp_cursor_shape_manager_v1");
		return NULL;
	}

	manager->base = NULL;
	manager->version = 0;
	wlf_signal_init(&manager->events.destroy);

	uint32_t bind_version = version;
	if (version > (uint32_t)wp_cursor_shape_manager_v1_interface.version) {
		wlf_log(WLF_DEBUG,
			"Server wp_cursor_shape_manager_v1 version %u is "
			"higher than "
			"client version %u, using client version",
			version,
			(uint32_t)wp_cursor_shape_manager_v1_interface.version);
		bind_version =
			(uint32_t)wp_cursor_shape_manager_v1_interface.version;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&wp_cursor_shape_manager_v1_interface, bind_version);
	if (manager->base == NULL) {
		wlf_log(WLF_ERROR,
			"Failed to bind wp_cursor_shape_manager_v1 interface "
			"(name: %u)",
			name);
		free(manager);
		return NULL;
	}
	manager->version = bind_version;

	wlf_log(WLF_DEBUG,
		"Successfully bound wp_cursor_shape_manager_v1 "
		"(name: %u, version: %u)",
		name, bind_version);

	return manager;
}

void wlf_wp_cursor_shape_manager_v1_destroy(struct wlf_wp_cursor_shape_manager_v1 *manager) {
	if (manager == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	assert(wlf_linked_list_empty(&manager->events.destroy.listener_list));

	if (manager->base != NULL) {
		wp_cursor_shape_manager_v1_destroy(manager->base);
		manager->base = NULL;
	}

	free(manager);
}

static struct wlf_wp_cursor_shape_device_v1 *alloc_device(void) {
	struct wlf_wp_cursor_shape_device_v1 *device =
		malloc(sizeof(struct wlf_wp_cursor_shape_device_v1));
	if (device == NULL) {
		wlf_log_errno(WLF_ERROR,
			"Failed to allocate wlf_wp_cursor_shape_device_v1");
		return NULL;
	}

	device->base = NULL;
	device->version = 0;
	wlf_signal_init(&device->events.destroy);
	return device;
}

struct wlf_wp_cursor_shape_device_v1 *wlf_wp_cursor_shape_manager_v1_get_pointer(
		struct wlf_wp_cursor_shape_manager_v1 *manager,
		struct wl_pointer *pointer) {
	assert(manager != NULL);
	assert(manager->base != NULL);
	assert(pointer != NULL);

	struct wlf_wp_cursor_shape_device_v1 *device = alloc_device();
	if (device == NULL) {
		return NULL;
	}

	device->base =
		wp_cursor_shape_manager_v1_get_pointer(manager->base, pointer);
	if (device->base == NULL) {
		wlf_log(WLF_ERROR,
			"wp_cursor_shape_manager_v1_get_pointer() returned "
			"NULL");
		free(device);
		return NULL;
	}
	device->version = wl_proxy_get_version((struct wl_proxy *)device->base);

	return device;
}

struct wlf_wp_cursor_shape_device_v1 *wlf_wp_cursor_shape_manager_v1_get_tablet_tool_v2(
		struct wlf_wp_cursor_shape_manager_v1 *manager,
		struct zwp_tablet_tool_v2 *tablet_tool) {
	assert(manager != NULL);
	assert(manager->base != NULL);
	assert(tablet_tool != NULL);

	struct wlf_wp_cursor_shape_device_v1 *device = alloc_device();
	if (device == NULL) {
		return NULL;
	}

	device->base = wp_cursor_shape_manager_v1_get_tablet_tool_v2(
		manager->base, tablet_tool);
	if (device->base == NULL) {
		wlf_log(WLF_ERROR,
			"wp_cursor_shape_manager_v1_get_tablet_tool_v2() "
			"returned NULL");
		free(device);
		return NULL;
	}
	device->version = wl_proxy_get_version((struct wl_proxy *)device->base);

	return device;
}

void wlf_wp_cursor_shape_device_v1_set_shape(
		struct wlf_wp_cursor_shape_device_v1 *device, uint32_t serial,
		enum wlf_cursor_shape shape) {
	assert(device != NULL);
	assert(device->base != NULL);
	assert(shape >= WLF_CURSOR_SHAPE_DEFAULT &&
		shape <= WLF_CURSOR_SHAPE_ALL_RESIZE);

	if (device->version < 2 && shape >= WLF_CURSOR_SHAPE_DND_ASK) {
		wlf_log(WLF_ERROR,
			"wp_cursor_shape_device_v1 shape %u requires version 2",
			(uint32_t)shape);
		return;
	}

	wp_cursor_shape_device_v1_set_shape(
		device->base, serial, (uint32_t)shape);
}

void wlf_wp_cursor_shape_device_v1_destroy(struct wlf_wp_cursor_shape_device_v1 *device) {
	if (device == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&device->events.destroy, device);
	assert(wlf_linked_list_empty(&device->events.destroy.listener_list));

	if (device->base != NULL) {
		wp_cursor_shape_device_v1_destroy(device->base);
	}

	free(device);
}
