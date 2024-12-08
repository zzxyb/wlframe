#include "wlf/wayland/wlf_wl_backend.h"
#include "wlf/wayland/wlf_wl_seat.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/wayland/wlf_wl_display.h"

#include <stdlib.h>
#include <assert.h>

static bool backend_start(struct wlf_backend *backend) {
	struct wlf_wl_backend *wl = get_wl_backend_from_backend(backend);
	wlf_log(WLF_INFO, "Starting Wayland backend");

	wl->started = true;

	// struct wlf_wl_seat *seat;
	// wlf_double_list_for_each(seat, &wl->seats, link) {
	// 	if (seat->wl_keyboard) {
	// 		init_seat_keyboard(seat);
	// 	}

	// 	if (seat->wl_touch) {
	// 		init_seat_touch(seat);
	// 	}

	// 	if (wl->tablet_manager) {
	// 		init_seat_tablet(seat);
	// 	}
	// }

	// for (size_t i = 0; i < wl->requested_outputs; ++i) {
	// 	wlf_wl_output_create(&wl->backend);
	// }

	return true;
}

static void backend_destroy(struct wlf_backend *backend) {
	if (!backend) {
		return;
	}

	struct wlf_wl_backend *wl = get_wl_backend_from_backend(backend);
	if (wl->display) {
		wlf_wl_display_destroy(wl->display);
	}

	free(backend);
};

static int backend_get_drm_fd(struct wlf_backend *backend) {
	struct wlf_wl_backend *wl = get_wl_backend_from_backend(backend);
	return wl->drm_fd;
}

static uint32_t get_buffer_caps(struct wlf_backend *backend) {
	// struct wlf_wl_backend *wl = get_wl_backend_from_backend(backend);
	// return (wl->zwp_linux_dmabuf_v1 ? WLR_BUFFER_CAP_DMABUF : 0)
	// 	| (wl->shm ? WLR_BUFFER_CAP_SHM : 0);
	return 0;
}

static const struct wlf_backend_impl backend_impl = {
	.start = backend_start,
	.destroy = backend_destroy,
	.get_drm_fd = backend_get_drm_fd,
	.get_buffer_caps = get_buffer_caps,
};

// static bool client_protocol_is_higher(uint32_t client_version, uint32_t remote_version) {
// 	if (client_version > remote_version) {
// 		wlf_log(WLF_ERROR, "Client protocol version: v%" PRIu32 " is higher than Compositor protocol: v%" PRIu32,
// 				client_version, remote_version);
// 		wlf_log(WLF_ERROR, "using Compositor protocol version instead Client protocol");
// 		return true;
// 	}
// 	return false;
// }

// static void xdg_wm_base_handle_ping(void *data,
// 		struct xdg_wm_base *base, uint32_t serial) {
// 	xdg_wm_base_pong(base, serial);
// }

// static const struct xdg_wm_base_listener xdg_wm_base_listener = {
// 	xdg_wm_base_handle_ping,
// };

// static void registry_global(void *data, struct wl_registry *registry,
// 		uint32_t name, const char *interface_name, uint32_t version) {
// 	struct wlf_wl_backend *wl = data;

// 	wlf_log(WLF_DEBUG, "wayland registry global: %s v%" PRIu32, interface_name, version);
// 	uint32_t current_version = version;
// 	if (strcmp(interface_name, wl_compositor_interface.name) == 0) {
// 		if (client_protocol_is_higher(wl_compositor_interface.version, version)) {
// 			current_version = wl_compositor_interface.version;
// 		}
// 		wl->compositor = wl_registry_bind(registry, name,
// 			&wl_compositor_interface, current_version);
// 	} else if (strcmp(interface_name, wl_seat_interface.name) == 0) {
// 		if (client_protocol_is_higher(wl_seat_interface.version, version)) {
// 			current_version = wl_seat_interface.version;
// 		}
// 		struct wl_seat *wl_seat = wl_registry_bind(registry, name,
// 			&wl_seat_interface, current_version);
// 		if (!create_wlf_wl_seat(wl_seat, wl, name)) {
// 			wl_seat_destroy(wl_seat);
// 		}
// 	} else if (strcmp(interface_name, xdg_wm_base_interface.name) == 0) {
// 		if (client_protocol_is_higher(xdg_wm_base_interface.version, version)) {
// 			current_version = xdg_wm_base_interface.version;
// 		}
// 		wl->xdg_wm_base = wl_registry_bind(registry, name,
// 			&xdg_wm_base_interface, current_version);
// 		xdg_wm_base_add_listener(wl->xdg_wm_base, &xdg_wm_base_listener, NULL);
// 	} else if (strcmp(interface_name, zxdg_decoration_manager_v1_interface.name) == 0) {
// 		if (client_protocol_is_higher(zxdg_decoration_manager_v1_interface.version, version)) {
// 			current_version = zxdg_decoration_manager_v1_interface.version;
// 		}
// 		wl->zxdg_decoration_manager_v1 = wl_registry_bind(registry, name,
// 			&zxdg_decoration_manager_v1_interface, current_version);
// 	} else if (strcmp(interface_name, zwp_pointer_gestures_v1_interface.name) == 0) {
// 		if (client_protocol_is_higher(zwp_pointer_gestures_v1_interface.version, version)) {
// 			current_version = zwp_pointer_gestures_v1_interface.version;
// 		}
// 		wl->zwp_pointer_gestures_v1 = wl_registry_bind(registry, name,
// 			&zwp_pointer_gestures_v1_interface, current_version);
// 	} else if (strcmp(interface_name, wp_presentation_interface.name) == 0) {
// 		if (client_protocol_is_higher(wp_presentation_interface.version, version)) {
// 			current_version = wp_presentation_interface.version;
// 		}
// 		wl->presentation = wl_registry_bind(registry, name,
// 			&wp_presentation_interface, current_version);
// 	} else if (strcmp(interface_name, zwp_relative_pointer_manager_v1_interface.name) == 0) {
// 		if (client_protocol_is_higher(zwp_relative_pointer_manager_v1_interface.version, version)) {
// 			current_version = zwp_relative_pointer_manager_v1_interface.version;
// 		}
// 		wl->zwp_relative_pointer_manager_v1 = wl_registry_bind(registry, name,
// 			&zwp_relative_pointer_manager_v1_interface, current_version);
// 	} else if (strcmp(interface_name, zwp_tablet_manager_v2_interface.name) == 0) {
// 		if (client_protocol_is_higher(zwp_tablet_manager_v2_interface.version, version)) {
// 			current_version = zwp_tablet_manager_v2_interface.version;
// 		}
// 		wl->tablet_manager = wl_registry_bind(registry, name,
// 			&zwp_tablet_manager_v2_interface, current_version);
// 	} else if (strcmp(interface_name, zwp_relative_pointer_manager_v1_interface.name) == 0) {
// 		if (client_protocol_is_higher(zwp_relative_pointer_manager_v1_interface.version, version)) {
// 			current_version = zwp_relative_pointer_manager_v1_interface.version;
// 		}
// 		wl->zwp_relative_pointer_manager_v1 = wl_registry_bind(registry, name,
// 			&zwp_relative_pointer_manager_v1_interface, current_version);
// 	} else if (strcmp(interface_name, xdg_activation_v1_interface.name) == 0) {
// 		if (client_protocol_is_higher(xdg_activation_v1_interface.version, version)) {
// 			current_version = xdg_activation_v1_interface.version;
// 		}
// 		wl->activation_v1 = wl_registry_bind(registry, name,
// 			&xdg_activation_v1_interface, current_version);
// 	} else if (strcmp(interface_name, wl_subcompositor_interface.name) == 0) {
// 		if (client_protocol_is_higher(wl_subcompositor_interface.version, version)) {
// 			current_version = wl_subcompositor_interface.version;
// 		}
// 		wl->subcompositor = wl_registry_bind(registry, name,
// 			&wl_subcompositor_interface, current_version);
// 	} else if (strcmp(interface_name, wp_viewporter_interface.name) == 0) {
// 		if (client_protocol_is_higher(wp_viewporter_interface.version, version)) {
// 			current_version = wp_viewporter_interface.version;
// 		}
// 		wl->viewporter = wl_registry_bind(registry, name,
// 			&wp_viewporter_interface, current_version);
// 	} else if (strcmp(interface_name, wp_linux_drm_syncobj_manager_v1_interface.name) == 0) {
// 		if (client_protocol_is_higher(wp_linux_drm_syncobj_manager_v1_interface.version, version)) {
// 			current_version = wp_linux_drm_syncobj_manager_v1_interface.version;
// 		}
// 		wl->drm_syncobj_manager_v1 = wl_registry_bind(registry, name,
// 			&wp_linux_drm_syncobj_manager_v1_interface, current_version);
// 	}
// }

// static void registry_global_remove(void *data, struct wl_registry *registry,
// 		uint32_t name) {
// 	struct wlf_wl_backend *wl = data;

// 	struct wlf_wl_seat *seat;
// 	wlf_double_list_for_each(seat, &wl->seats, link) {
// 		if (seat->global_name == name) {
// 			wlf_wl_seat_destroy(seat);
// 			break;
// 		}
// 	}
// }

// static const struct wl_registry_listener wl_registry_listener = {
// 	.global = registry_global,
// 	.global_remove = registry_global_remove,
// };

struct wlf_backend *wlf_wl_backend_create(void) {
	wlf_log(WLF_INFO, "Creating wayland backend");

	struct wlf_wl_backend *wl = calloc(1, sizeof(*wl));
	if (!wl) {
		wlf_log(WLF_ERROR, "Allocation failed");
		return NULL;
	}

	wlf_backend_init(&wl->backend, &backend_impl);

	wl->display = wlf_wl_display_create();
	if (!wl->display) {
		wlf_log(WLF_ERROR, "Allocation struct wlf_wl_display failed!");
		goto error_wl;
	}

	// init build-in protocols

	if (!wlf_wl_display_init_registry(wl->display)) {
		wlf_log(WLF_ERROR, "wlf_wl_display_init_registry failed");
		goto error_display;
	}
	return &wl->backend;

error_display:
	wlf_wl_display_destroy(wl->display);
error_wl:
	wlf_backend_finish(&wl->backend);
	free(wl);
	return NULL;
}

bool wlf_backend_is_wl(struct wlf_backend *backend) {
	return backend->impl == &backend_impl;
}

struct wlf_wl_backend *get_wl_backend_from_backend(struct wlf_backend *wlf_backend) {
	assert(wlf_backend_is_wl(wlf_backend));
	struct wlf_wl_backend *backend = wlf_container_of(wlf_backend, backend, backend);
	return backend;
}
