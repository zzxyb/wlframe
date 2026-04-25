#include "wlf/platform/wayland/backend.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/wayland/wlf_wl_output.h"
#include "wlf/wayland/wlf_wl_interface.h"
#include "wlf/types/wlf_pixel_format.h"

#include "wayland/protocols/xdg-shell-client-protocol.h"
#include "wayland/protocols/viewporter-client-protocol.h"
#include "wayland/protocols/cursor-shape-v1-client-protocol.h"
#include "wayland/protocols/content-type-v1-client-protocol.h"
#include "wayland/protocols/xdg-activation-v1-client-protocol.h"
#include "wayland/protocols/alpha-modifier-v1-client-protocol.h"
#include "wayland/protocols/xdg-decoration-unstable-v1-client-protocol.h"
#include "wayland/protocols/xdg-toplevel-icon-v1-client-protocol.h"
#include "wayland/protocols/fractional-scale-v1-client-protocol.h"
#include "wayland/protocols/xdg-output-unstable-v1-client-protocol.h"
#include "wayland/protocols/text-input-unstable-v3-client-protocol.h"
#include "wayland/protocols/single-pixel-buffer-v1-client-protocol.h"
#include "wayland/protocols/wlr-layer-shell-unstable-v1-client-protocol.h"
#include "wayland/protocols/primary-selection-unstable-v1-client-protocol.h"
#include "wayland/protocols/pointer-gestures-unstable-v1-client-protocol.h"
#include "wayland/protocols/keyboard-shortcuts-inhibit-unstable-v1-client-protocol.h"

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <poll.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
		uint32_t serial) {
	WLF_UNUSED(data);
	xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
	.ping = xdg_wm_base_ping,
};

static void shm_handle_format(void *data, struct wl_shm *shm,
		uint32_t shm_format) {
	struct wlf_wl_backend *wl_backend = data;
	uint32_t drm_format = convert_wl_shm_format_to_wlf(shm_format);
	wlf_render_format_set_add(&wl_backend->wl_shm.shm_formats, drm_format, WLF_FORMAT_INVALID);
}

static const struct wl_shm_listener shm_listener = {
	.format = shm_handle_format,
};

static void destroy_wl_compositor(struct wlf_wl_backend *wayland) {
	if (wayland->wl_compositor.compositor == NULL) {
		return;
	}

	wl_compositor_destroy(wayland->wl_compositor.compositor);
	wayland->wl_compositor.compositor = NULL;
	wayland->wl_compositor.bind_version = 0;
	wayland->wl_compositor.name = 0;
}

static void destroy_wl_shm(struct wlf_wl_backend *wayland) {
	if (wayland->wl_shm.shm == NULL) {
		return;
	}

	wlf_render_format_set_finish(&wayland->wl_shm.shm_formats);
	wl_shm_destroy(wayland->wl_shm.shm);
	wayland->wl_shm.shm = NULL;
	wayland->wl_shm.bind_version = 0;
	wayland->wl_shm.name = 0;
}

static void destroy_wl_subcompositor(struct wlf_wl_backend *wayland) {
	if (wayland->wl_subcompositor.subcompositor == NULL) {
		return;
	}

	wl_subcompositor_destroy(wayland->wl_subcompositor.subcompositor);
	wayland->wl_subcompositor.subcompositor = NULL;
	wayland->wl_subcompositor.bind_version = 0;
	wayland->wl_subcompositor.name = 0;
}

static void destroy_wl_fixes(struct wlf_wl_backend *wayland) {
	if (wayland->wl_fixes.fixes == NULL) {
		return;
	}

	wl_fixes_destroy(wayland->wl_fixes.fixes);
	wayland->wl_fixes.fixes = NULL;
	wayland->wl_fixes.bind_version = 0;
	wayland->wl_fixes.name = 0;
}

static void destroy_wl_data_device_manager(struct wlf_wl_backend *wayland) {
	if (wayland->wl_data_device_manager.data_device_manager == NULL) {
		return;
	}

	wl_data_device_manager_destroy(wayland->wl_data_device_manager.data_device_manager);
	wayland->wl_data_device_manager.data_device_manager = NULL;
	wayland->wl_data_device_manager.bind_version = 0;
	wayland->wl_data_device_manager.name = 0;
}

static void destroy_xdg_wm_base(struct wlf_wl_backend *wayland) {
	if (wayland->xdg_wm_base.wm_base == NULL) {
		return;
	}

	xdg_wm_base_destroy(wayland->xdg_wm_base.wm_base);
	wayland->xdg_wm_base.wm_base = NULL;
	wayland->xdg_wm_base.bind_version = 0;
	wayland->xdg_wm_base.name = 0;
}

static void destroy_wp_viewporter(struct wlf_wl_backend *wayland) {
	if (wayland->wp_viewporter.viewporter == NULL) {
		return;
	}

	wp_viewporter_destroy(wayland->wp_viewporter.viewporter);
	wayland->wp_viewporter.viewporter = NULL;
	wayland->wp_viewporter.bind_version = 0;
	wayland->wp_viewporter.name = 0;
}

static void destroy_zwlr_layer_shell_v1(struct wlf_wl_backend *wayland) {
	if (wayland->zwlr_layer_shell_v1.layer_shell_v1 == NULL) {
		return;
	}

	zwlr_layer_shell_v1_destroy(wayland->zwlr_layer_shell_v1.layer_shell_v1);
	wayland->zwlr_layer_shell_v1.layer_shell_v1 = NULL;
	wayland->zwlr_layer_shell_v1.bind_version = 0;
	wayland->zwlr_layer_shell_v1.name = 0;
}

static void destroy_xdg_activation_v1(struct wlf_wl_backend *wayland) {
	if (wayland->xdg_activation_v1.xdg_activation_v1 == NULL) {
		return;
	}

	xdg_activation_v1_destroy(wayland->xdg_activation_v1.xdg_activation_v1);
	wayland->xdg_activation_v1.xdg_activation_v1 = NULL;
	wayland->xdg_activation_v1.bind_version = 0;
	wayland->xdg_activation_v1.name = 0;
}

static void destroy_zxdg_output_manager_v1(struct wlf_wl_backend *wayland) {
	if (wayland->zxdg_output_manager_v1.output_manager_v1 == NULL) {
		return;
	}

	zxdg_output_manager_v1_destroy(wayland->zxdg_output_manager_v1.output_manager_v1);
	wayland->zxdg_output_manager_v1.output_manager_v1 = NULL;
	wayland->zxdg_output_manager_v1.bind_version = 0;
	wayland->zxdg_output_manager_v1.name = 0;
}

static void destroy_wp_alpha_modifier_v1(struct wlf_wl_backend *wayland) {
	if (wayland->wp_alpha_modifier_v1.alpha_modifier_v1 == NULL) {
		return;
	}

	wp_alpha_modifier_v1_destroy(wayland->wp_alpha_modifier_v1.alpha_modifier_v1);
	wayland->wp_alpha_modifier_v1.alpha_modifier_v1 = NULL;
	wayland->wp_alpha_modifier_v1.bind_version = 0;
	wayland->wp_alpha_modifier_v1.name = 0;
}

static void destroy_wp_content_type_manager_v1(struct wlf_wl_backend *wayland) {
	if (wayland->wp_content_type_manager_v1.content_type_manager_v1 == NULL) {
		return;
	}

	wp_content_type_manager_v1_destroy(wayland->wp_content_type_manager_v1.content_type_manager_v1);
	wayland->wp_content_type_manager_v1.content_type_manager_v1 = NULL;
	wayland->wp_content_type_manager_v1.bind_version = 0;
	wayland->wp_content_type_manager_v1.name = 0;
}

static void destroy_zwp_pointer_gestures_v1(struct wlf_wl_backend *wayland) {
	if (wayland->zwp_pointer_gestures_v1.pointer_gestures_v1 == NULL) {
		return;
	}

	zwp_pointer_gestures_v1_destroy(wayland->zwp_pointer_gestures_v1.pointer_gestures_v1);
	wayland->zwp_pointer_gestures_v1.pointer_gestures_v1 = NULL;
	wayland->zwp_pointer_gestures_v1.bind_version = 0;
	wayland->zwp_pointer_gestures_v1.name = 0;
}

static void destroy_zwp_text_input_manager_v3(struct wlf_wl_backend *wayland) {
	if (wayland->zwp_text_input_manager_v3.text_input_manager_v3 == NULL) {
		return;
	}

	zwp_text_input_manager_v3_destroy(wayland->zwp_text_input_manager_v3.text_input_manager_v3);
	wayland->zwp_text_input_manager_v3.text_input_manager_v3 = NULL;
	wayland->zwp_text_input_manager_v3.bind_version = 0;
	wayland->zwp_text_input_manager_v3.name = 0;
}

static void destroy_wp_single_pixel_buffer_manager_v1(struct wlf_wl_backend *wayland) {
	if (wayland->wp_single_pixel_buffer_manager_v1.single_pixel_buffer_manager_v1 == NULL) {
		return;
	}

	wp_single_pixel_buffer_manager_v1_destroy(
		wayland->wp_single_pixel_buffer_manager_v1.single_pixel_buffer_manager_v1);
	wayland->wp_single_pixel_buffer_manager_v1.single_pixel_buffer_manager_v1 = NULL;
	wayland->wp_single_pixel_buffer_manager_v1.bind_version = 0;
	wayland->wp_single_pixel_buffer_manager_v1.name = 0;
}

static void destroy_zxdg_decoration_manager_v1(struct wlf_wl_backend *wayland) {
	if (wayland->zxdg_decoration_manager_v1.decoration_manager == NULL) {
		return;
	}

	zxdg_decoration_manager_v1_destroy(wayland->zxdg_decoration_manager_v1.decoration_manager);
	wayland->zxdg_decoration_manager_v1.decoration_manager = NULL;
	wayland->zxdg_decoration_manager_v1.bind_version = 0;
	wayland->zxdg_decoration_manager_v1.name = 0;
}

static void destroy_wp_cursor_shape_manager_v1(struct wlf_wl_backend *wayland) {
	if (wayland->wp_cursor_shape_manager_v1.cursor_shape_manager_v1 == NULL) {
		return;
	}

	wp_cursor_shape_manager_v1_destroy(wayland->wp_cursor_shape_manager_v1.cursor_shape_manager_v1);
	wayland->wp_cursor_shape_manager_v1.cursor_shape_manager_v1 = NULL;
	wayland->wp_cursor_shape_manager_v1.bind_version = 0;
	wayland->wp_cursor_shape_manager_v1.name = 0;
}

static void destroy_xdg_toplevel_icon_manager_v1(struct wlf_wl_backend *wayland) {
	if (wayland->xdg_toplevel_icon_manager_v1.toplevel_icon_manager_v1 == NULL) {
		return;
	}

	xdg_toplevel_icon_manager_v1_destroy(wayland->xdg_toplevel_icon_manager_v1.toplevel_icon_manager_v1);
	wayland->xdg_toplevel_icon_manager_v1.toplevel_icon_manager_v1 = NULL;
	wayland->xdg_toplevel_icon_manager_v1.bind_version = 0;
	wayland->xdg_toplevel_icon_manager_v1.name = 0;
}

static void destroy_wp_fractional_scale_manager_v1(struct wlf_wl_backend *wayland) {
	if (wayland->wp_fractional_scale_manager_v1.fractional_scale_manager_v1 == NULL) {
		return;
	}

	wp_fractional_scale_manager_v1_destroy(wayland->wp_fractional_scale_manager_v1.fractional_scale_manager_v1);
	wayland->wp_fractional_scale_manager_v1.fractional_scale_manager_v1 = NULL;
	wayland->wp_fractional_scale_manager_v1.bind_version = 0;
	wayland->wp_fractional_scale_manager_v1.name = 0;
}

static void destroy_zwp_primary_selection_device_manager_v1(struct wlf_wl_backend *wayland) {
	if (wayland->zwp_primary_selection_device_manager_v1.primary_selection_device_manager_v1 == NULL) {
		return;
	}

	zwp_primary_selection_device_manager_v1_destroy(
		wayland->zwp_primary_selection_device_manager_v1.primary_selection_device_manager_v1);
	wayland->zwp_primary_selection_device_manager_v1.primary_selection_device_manager_v1 = NULL;
	wayland->zwp_primary_selection_device_manager_v1.bind_version = 0;
	wayland->zwp_primary_selection_device_manager_v1.name = 0;
}

static void destroy_zwp_keyboard_shortcuts_inhibit_manager_v1(struct wlf_wl_backend *wayland) {
	if (wayland->zwp_keyboard_shortcuts_inhibit_manager_v1.keyboard_shortcuts_inhibit_manager_v1 == NULL) {
		return;
	}

	zwp_keyboard_shortcuts_inhibit_manager_v1_destroy(
		wayland->zwp_keyboard_shortcuts_inhibit_manager_v1.keyboard_shortcuts_inhibit_manager_v1);
	wayland->zwp_keyboard_shortcuts_inhibit_manager_v1.keyboard_shortcuts_inhibit_manager_v1 = NULL;
	wayland->zwp_keyboard_shortcuts_inhibit_manager_v1.bind_version = 0;
	wayland->zwp_keyboard_shortcuts_inhibit_manager_v1.name = 0;
}

static void display_global_added(void *data, struct wl_registry *wl_registry,
		uint32_t name, const char *interface, uint32_t version) {
	wlf_log(WLF_DEBUG, "Wayland registry global: %s v%" PRIu32, interface, version);
	struct wlf_wl_backend *wayland = data;
	uint32_t bind_version = version;

	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		if (version > (uint32_t)wl_compositor_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s interface version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wl_compositor_interface.version);
			bind_version = (uint32_t)wl_compositor_interface.version;
		}

		wayland->wl_compositor.compositor = wl_registry_bind(wl_registry,
			name,
			&wl_compositor_interface,
			bind_version);
		if (wayland->wl_compositor.compositor == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->wl_compositor.bind_version = bind_version;
		wayland->wl_compositor.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
		if (version > (uint32_t)wl_subcompositor_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s interface version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wl_subcompositor_interface.version);
			bind_version = (uint32_t)wl_subcompositor_interface.version;
		}

		wayland->wl_subcompositor.subcompositor = wl_registry_bind(wl_registry,
			name,
			&wl_subcompositor_interface,
			bind_version);
		if (wayland->wl_subcompositor.subcompositor == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->wl_subcompositor.bind_version = bind_version;
		wayland->wl_subcompositor.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, wl_fixes_interface.name) == 0) {
		if (version > (uint32_t)wl_fixes_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s interface version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wl_fixes_interface.version);
			bind_version = (uint32_t)wl_fixes_interface.version;
		}

		wayland->wl_fixes.fixes = wl_registry_bind(wl_registry,
			name,
			&wl_fixes_interface,
			bind_version);
		if (wayland->wl_fixes.fixes == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->wl_fixes.bind_version = bind_version;
		wayland->wl_fixes.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		if (version > (uint32_t)wl_shm_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wl_shm_interface.version);
			bind_version = (uint32_t)wl_shm_interface.version;
		}

		wayland->wl_shm.shm = wl_registry_bind(wl_registry,
			name,
			&wl_shm_interface,
			bind_version);
		if (wayland->wl_shm.shm == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wl_shm_add_listener(wayland->wl_shm.shm, &shm_listener, wayland);
		wayland->wl_shm.bind_version = bind_version;
		wayland->wl_shm.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, wl_data_device_manager_interface.name) == 0) {
		if (version > (uint32_t)wl_data_device_manager_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wl_data_device_manager_interface.version);
			bind_version = (uint32_t)wl_data_device_manager_interface.version;
		}

		wayland->wl_data_device_manager.data_device_manager = wl_registry_bind(wl_registry,
			name,
			&wl_data_device_manager_interface,
			bind_version);
		if (wayland->wl_data_device_manager.data_device_manager == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->wl_data_device_manager.bind_version = bind_version;
		wayland->wl_data_device_manager.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		if (version > (uint32_t)xdg_wm_base_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)xdg_wm_base_interface.version);
			bind_version = (uint32_t)xdg_wm_base_interface.version;
		}

		wayland->xdg_wm_base.wm_base = wl_registry_bind(wl_registry,
			name,
			&xdg_wm_base_interface,
			bind_version);
		if (wayland->xdg_wm_base.wm_base == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->xdg_wm_base.bind_version = bind_version;
		wayland->xdg_wm_base.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
		xdg_wm_base_add_listener(wayland->xdg_wm_base.wm_base, &xdg_wm_base_listener, wayland);
	} else if (strcmp(interface, wp_viewporter_interface.name) == 0) {
		if (version > (uint32_t)wp_viewporter_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wp_viewporter_interface.version);
			bind_version = (uint32_t)wp_viewporter_interface.version;
		}

		wayland->wp_viewporter.viewporter = wl_registry_bind(wl_registry,
			name,
			&wp_viewporter_interface,
			bind_version);
		if (wayland->wp_viewporter.viewporter == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->wp_viewporter.bind_version = bind_version;
		wayland->wp_viewporter.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		if (version > (uint32_t)zwlr_layer_shell_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)zwlr_layer_shell_v1_interface.version);
			bind_version = (uint32_t)zwlr_layer_shell_v1_interface.version;
		}

		wayland->zwlr_layer_shell_v1.layer_shell_v1 = wl_registry_bind(wl_registry,
			name,
			&zwlr_layer_shell_v1_interface,
			bind_version);
		if (wayland->zwlr_layer_shell_v1.layer_shell_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->zwlr_layer_shell_v1.bind_version = bind_version;
		wayland->zwlr_layer_shell_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, xdg_activation_v1_interface.name) == 0) {
		if (version > (uint32_t)xdg_activation_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)xdg_activation_v1_interface.version);
			bind_version = (uint32_t)xdg_activation_v1_interface.version;
		}

		wayland->xdg_activation_v1.xdg_activation_v1 = wl_registry_bind(wl_registry,
			name,
			&xdg_activation_v1_interface,
			bind_version);
		if (wayland->xdg_activation_v1.xdg_activation_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->xdg_activation_v1.bind_version = bind_version;
		wayland->xdg_activation_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
		if (version > (uint32_t)zxdg_output_manager_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)zxdg_output_manager_v1_interface.version);
			bind_version = (uint32_t)zxdg_output_manager_v1_interface.version;
		}

		wayland->zxdg_output_manager_v1.output_manager_v1 = wl_registry_bind(wl_registry,
			name,
			&zxdg_output_manager_v1_interface,
			bind_version);
		if (wayland->zxdg_output_manager_v1.output_manager_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->zxdg_output_manager_v1.bind_version = bind_version;
		wayland->zxdg_output_manager_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, wp_alpha_modifier_v1_interface.name) == 0) {
		if (version > (uint32_t)wp_alpha_modifier_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wp_alpha_modifier_v1_interface.version);
			bind_version = (uint32_t)wp_alpha_modifier_v1_interface.version;
		}

		wayland->wp_alpha_modifier_v1.alpha_modifier_v1 = wl_registry_bind(wl_registry,
			name,
			&wp_alpha_modifier_v1_interface,
			bind_version);
		if (wayland->wp_alpha_modifier_v1.alpha_modifier_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->wp_alpha_modifier_v1.bind_version = bind_version;
		wayland->wp_alpha_modifier_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, wp_content_type_manager_v1_interface.name) == 0) {
		if (version > (uint32_t)wp_content_type_manager_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wp_content_type_manager_v1_interface.version);
			bind_version = (uint32_t)wp_content_type_manager_v1_interface.version;
		}

		wayland->wp_content_type_manager_v1.content_type_manager_v1 = wl_registry_bind(wl_registry,
			name,
			&wp_content_type_manager_v1_interface,
			bind_version);
		if (wayland->wp_content_type_manager_v1.content_type_manager_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->wp_content_type_manager_v1.bind_version = bind_version;
		wayland->wp_content_type_manager_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, zwp_pointer_gestures_v1_interface.name) == 0) {
		if (version > (uint32_t)zwp_pointer_gestures_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)zwp_pointer_gestures_v1_interface.version);
			bind_version = (uint32_t)zwp_pointer_gestures_v1_interface.version;
		}

		wayland->zwp_pointer_gestures_v1.pointer_gestures_v1 = wl_registry_bind(wl_registry,
			name,
			&zwp_pointer_gestures_v1_interface,
			bind_version);
		if (wayland->zwp_pointer_gestures_v1.pointer_gestures_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->zwp_pointer_gestures_v1.bind_version = bind_version;
		wayland->zwp_pointer_gestures_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, zwp_text_input_manager_v3_interface.name) == 0) {
		if (version > (uint32_t)zwp_text_input_manager_v3_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)zwp_text_input_manager_v3_interface.version);
			bind_version = (uint32_t)zwp_text_input_manager_v3_interface.version;
		}

		wayland->zwp_text_input_manager_v3.text_input_manager_v3 = wl_registry_bind(wl_registry,
			name,
			&zwp_text_input_manager_v3_interface,
			bind_version);
		if (wayland->zwp_text_input_manager_v3.text_input_manager_v3 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->zwp_text_input_manager_v3.bind_version = bind_version;
		wayland->zwp_text_input_manager_v3.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, wp_single_pixel_buffer_manager_v1_interface.name) == 0) {
		if (version > (uint32_t)wp_single_pixel_buffer_manager_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wp_single_pixel_buffer_manager_v1_interface.version);
			bind_version = (uint32_t)wp_single_pixel_buffer_manager_v1_interface.version;
		}

		wayland->wp_single_pixel_buffer_manager_v1.single_pixel_buffer_manager_v1 = wl_registry_bind(wl_registry,
			name,
			&wp_single_pixel_buffer_manager_v1_interface,
			bind_version);
		if (wayland->wp_single_pixel_buffer_manager_v1.single_pixel_buffer_manager_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->wp_single_pixel_buffer_manager_v1.bind_version = bind_version;
		wayland->wp_single_pixel_buffer_manager_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
		if (version > (uint32_t)zxdg_decoration_manager_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)zxdg_decoration_manager_v1_interface.version);
			bind_version = (uint32_t)zxdg_decoration_manager_v1_interface.version;
		}

		wayland->zxdg_decoration_manager_v1.decoration_manager = wl_registry_bind(wl_registry,
			name,
			&zxdg_decoration_manager_v1_interface,
			bind_version);
		if (wayland->zxdg_decoration_manager_v1.decoration_manager == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->zxdg_decoration_manager_v1.bind_version = bind_version;
		wayland->zxdg_decoration_manager_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, wp_cursor_shape_manager_v1_interface.name) == 0) {
		if (version > (uint32_t)wp_cursor_shape_manager_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wp_cursor_shape_manager_v1_interface.version);
			bind_version = (uint32_t)wp_cursor_shape_manager_v1_interface.version;
		}

		wayland->wp_cursor_shape_manager_v1.cursor_shape_manager_v1 = wl_registry_bind(wl_registry,
			name,
			&wp_cursor_shape_manager_v1_interface,
			bind_version);
		if (wayland->wp_cursor_shape_manager_v1.cursor_shape_manager_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->wp_cursor_shape_manager_v1.bind_version = bind_version;
		wayland->wp_cursor_shape_manager_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, xdg_toplevel_icon_manager_v1_interface.name) == 0) {
		if (version > (uint32_t)xdg_toplevel_icon_manager_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)xdg_toplevel_icon_manager_v1_interface.version);
			bind_version = (uint32_t)xdg_toplevel_icon_manager_v1_interface.version;
		}

		wayland->xdg_toplevel_icon_manager_v1.toplevel_icon_manager_v1 = wl_registry_bind(wl_registry,
			name,
			&xdg_toplevel_icon_manager_v1_interface,
			bind_version);
		if (wayland->xdg_toplevel_icon_manager_v1.toplevel_icon_manager_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->xdg_toplevel_icon_manager_v1.bind_version = bind_version;
		wayland->xdg_toplevel_icon_manager_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, wp_fractional_scale_manager_v1_interface.name) == 0) {
		if (version > (uint32_t)wp_fractional_scale_manager_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)wp_fractional_scale_manager_v1_interface.version);
			bind_version = (uint32_t)wp_fractional_scale_manager_v1_interface.version;
		}

		wayland->wp_fractional_scale_manager_v1.fractional_scale_manager_v1 = wl_registry_bind(wl_registry,
			name,
			&wp_fractional_scale_manager_v1_interface,
			bind_version);
		if (wayland->wp_fractional_scale_manager_v1.fractional_scale_manager_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->wp_fractional_scale_manager_v1.bind_version = bind_version;
		wayland->wp_fractional_scale_manager_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, zwp_primary_selection_device_manager_v1_interface.name) == 0) {
		if (version > (uint32_t)zwp_primary_selection_device_manager_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)zwp_primary_selection_device_manager_v1_interface.version);
			bind_version = (uint32_t)zwp_primary_selection_device_manager_v1_interface.version;
		}

		wayland->zwp_primary_selection_device_manager_v1.primary_selection_device_manager_v1 =
			wl_registry_bind(wl_registry,
				name,
				&zwp_primary_selection_device_manager_v1_interface,
				bind_version);
		if (wayland->zwp_primary_selection_device_manager_v1.primary_selection_device_manager_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->zwp_primary_selection_device_manager_v1.bind_version = bind_version;
		wayland->zwp_primary_selection_device_manager_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	} else if (strcmp(interface, zwp_keyboard_shortcuts_inhibit_manager_v1_interface.name) == 0) {
		if (version > (uint32_t)zwp_keyboard_shortcuts_inhibit_manager_v1_interface.version) {
			wlf_log(WLF_DEBUG, "Server %s version %u is higher than client version %u, "
				"using client version", interface, version,
				(uint32_t)zwp_keyboard_shortcuts_inhibit_manager_v1_interface.version);
			bind_version = (uint32_t)zwp_keyboard_shortcuts_inhibit_manager_v1_interface.version;
		}

		wayland->zwp_keyboard_shortcuts_inhibit_manager_v1.keyboard_shortcuts_inhibit_manager_v1 =
			wl_registry_bind(wl_registry,
				name,
				&zwp_keyboard_shortcuts_inhibit_manager_v1_interface,
				bind_version);
		if (wayland->zwp_keyboard_shortcuts_inhibit_manager_v1.keyboard_shortcuts_inhibit_manager_v1 == NULL) {
			wlf_log(WLF_ERROR, "Failed to bind %s interface", interface);
			return;
		}

		wayland->zwp_keyboard_shortcuts_inhibit_manager_v1.bind_version = bind_version;
		wayland->zwp_keyboard_shortcuts_inhibit_manager_v1.name = name;
		wlf_log(WLF_DEBUG, "Successfully bound interface (name: %s, version: %u)",
			interface, bind_version);
	}

	struct wlf_wl_interface *new_reg = wlf_wl_interface_create(interface, version, name);
	if (new_reg == NULL) {
		return;
	}

	wlf_linked_list_insert(&wayland->interfaces, &new_reg->link);
	wlf_signal_emit_mutable(&wayland->events.global_add, new_reg);
}

static void display_global_remove(void *data,
		struct wl_registry *wl_registry, uint32_t name) {
	struct wlf_wl_backend *wayland = data;

	if (name == wayland->zwp_keyboard_shortcuts_inhibit_manager_v1.name) {
		destroy_zwp_keyboard_shortcuts_inhibit_manager_v1(wayland);
	}

	if (name == wayland->zwp_primary_selection_device_manager_v1.name) {
		destroy_zwp_primary_selection_device_manager_v1(wayland);
	}

	if (name == wayland->wp_fractional_scale_manager_v1.name) {
		destroy_wp_fractional_scale_manager_v1(wayland);
	}

	if (name == wayland->xdg_toplevel_icon_manager_v1.name) {
		destroy_xdg_toplevel_icon_manager_v1(wayland);
	}

	if (name == wayland->wp_cursor_shape_manager_v1.name) {
		destroy_wp_cursor_shape_manager_v1(wayland);
	}

	if (name == wayland->zxdg_decoration_manager_v1.name) {
		destroy_zxdg_decoration_manager_v1(wayland);
	}

	if (name == wayland->wp_single_pixel_buffer_manager_v1.name) {
		destroy_wp_single_pixel_buffer_manager_v1(wayland);
	}

	if (name == wayland->zwp_text_input_manager_v3.name) {
		destroy_zwp_text_input_manager_v3(wayland);
	}

	if (name == wayland->zwp_pointer_gestures_v1.name) {
		destroy_zwp_pointer_gestures_v1(wayland);
	}

	if (name == wayland->wp_content_type_manager_v1.name) {
		destroy_wp_content_type_manager_v1(wayland);
	}

	if (name == wayland->wp_alpha_modifier_v1.name) {
		destroy_wp_alpha_modifier_v1(wayland);
	}

	if (name == wayland->zxdg_output_manager_v1.name) {
		destroy_zxdg_output_manager_v1(wayland);
	}

	if (name == wayland->xdg_activation_v1.name) {
		destroy_xdg_activation_v1(wayland);
	}

	if (name == wayland->zwlr_layer_shell_v1.name) {
		destroy_zwlr_layer_shell_v1(wayland);
	}

	if (name == wayland->wp_viewporter.name) {
		destroy_wp_viewporter(wayland);
	}

	if (name == wayland->xdg_wm_base.name) {
		destroy_xdg_wm_base(wayland);
	}

	if (name == wayland->wl_data_device_manager.name) {
		destroy_wl_data_device_manager(wayland);
	}

	if (name == wayland->wl_shm.name) {
		destroy_wl_shm(wayland);
	}

	if (name == wayland->wl_compositor.name) {
		destroy_wl_compositor(wayland);
	}

	if (name == wayland->wl_subcompositor.name) {
		destroy_wl_subcompositor(wayland);
	}

	if (name == wayland->wl_fixes.name) {
		destroy_wl_fixes(wayland);
	}

	struct wlf_wl_interface *reg, *tmp;
	wlf_linked_list_for_each_safe(reg, tmp, &wayland->interfaces, link) {
		if (reg->name == name) {
			wlf_log(WLF_DEBUG, "Interface %s removed", reg->interface);
			wlf_signal_emit_mutable(&wayland->events.global_remove, reg);
			wlf_wl_interface_destroy(reg);
			break;
		}
	}
}

static const struct wl_registry_listener wl_base_registry_listener = {
	.global = display_global_added,
	.global_remove = display_global_remove,
};

static void backend_destroy(struct wlf_backend *backend) {
	wlf_log(WLF_DEBUG, "Destroying %s backend", backend->impl->name);

	struct wlf_wl_backend *wayland = wlf_wl_backend_from_backend(backend);

	destroy_zwp_keyboard_shortcuts_inhibit_manager_v1(wayland);
	destroy_zwp_primary_selection_device_manager_v1(wayland);
	destroy_wp_fractional_scale_manager_v1(wayland);
	destroy_xdg_toplevel_icon_manager_v1(wayland);
	destroy_wp_cursor_shape_manager_v1(wayland);
	destroy_zxdg_decoration_manager_v1(wayland);
	destroy_wp_single_pixel_buffer_manager_v1(wayland);
	destroy_zwp_text_input_manager_v3(wayland);
	destroy_zwp_pointer_gestures_v1(wayland);
	destroy_wp_content_type_manager_v1(wayland);
	destroy_wp_alpha_modifier_v1(wayland);
	destroy_zxdg_output_manager_v1(wayland);
	destroy_xdg_activation_v1(wayland);
	destroy_zwlr_layer_shell_v1(wayland);
	destroy_wp_viewporter(wayland);
	destroy_xdg_wm_base(wayland);
	destroy_wl_data_device_manager(wayland);
	destroy_wl_fixes(wayland);
	destroy_wl_shm(wayland);
	destroy_wl_compositor(wayland);
	destroy_wl_subcompositor(wayland);

	struct wlf_wl_interface *reg, *tmp;
	wlf_linked_list_for_each_safe(reg, tmp, &wayland->interfaces, link) {
		wlf_signal_emit_mutable(&wayland->events.global_remove, reg);
		wlf_wl_interface_destroy(reg);
	}

	if (wayland->registry != NULL) {
		wl_registry_destroy(wayland->registry);
	}

	if (wayland->display != NULL) {
		wl_display_disconnect(wayland->display);
	}

	free(wayland);
}

static void backend_exe(struct wlf_backend *backend) {
	struct wlf_wl_backend *wayland_backend =
		wlf_wl_backend_from_backend(backend);
	int wayland_fd = wl_display_get_fd(wayland_backend->display);
	if (wayland_fd < 0) {
		wlf_log(WLF_ERROR, "Failed to get Wayland display fd");
		return;
	}

	backend->running = true;
	while (backend->running) {
		if (wl_display_dispatch_pending(wayland_backend->display) == -1) {
			wlf_log(WLF_ERROR, "Failed to dispatch Wayland pending events");
			break;
		}

		if (wl_display_flush(wayland_backend->display) == -1 && errno != EAGAIN) {
			wlf_log_errno(WLF_ERROR, "Failed to flush Wayland display");
			break;
		}

		while (wl_display_prepare_read(wayland_backend->display) != 0) {
			if (wl_display_dispatch_pending(wayland_backend->display) == -1) {
				wlf_log(WLF_ERROR, "Failed to dispatch Wayland pending events");
				goto out;
			}
		}

		size_t nfds = 1 + backend->event_source_count;
		struct pollfd *fds = calloc(nfds, sizeof(struct pollfd));
		if (fds == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate pollfd array");
			wl_display_cancel_read(wayland_backend->display);
			break;
		}

		fds[0].fd = wayland_fd;
		fds[0].events = POLLIN;

		for (size_t i = 0; i < backend->event_source_count; ++i) {
			fds[i + 1].fd = backend->event_sources[i].fd;
			fds[i + 1].events = backend->event_sources[i].events;
		}

		int poll_ret = poll(fds, nfds, -1);
		if (poll_ret < 0) {
			if (errno == EINTR) {
				wl_display_cancel_read(wayland_backend->display);
				free(fds);
				continue;
			}

			wlf_log_errno(WLF_ERROR, "poll() failed in Wayland event loop");
			wl_display_cancel_read(wayland_backend->display);
			free(fds);
			break;
		}

		if ((fds[0].revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL)) != 0) {
			if (wl_display_read_events(wayland_backend->display) == -1) {
				wlf_log(WLF_ERROR, "Failed to read Wayland events");
				free(fds);
				break;
			}
		} else {
			wl_display_cancel_read(wayland_backend->display);
		}

		for (size_t i = 0; i < backend->event_source_count; ++i) {
			if (fds[i + 1].revents == 0) {
				continue;
			}

			if (backend->event_sources[i].dispatch != NULL) {
				backend->event_sources[i].dispatch(
					backend,
					backend->event_sources[i].fd,
					(uint32_t)fds[i + 1].revents,
					backend->event_sources[i].data);
			}
		}

		free(fds);
	}

out:
	backend->running = false;
}

static const struct wlf_backend_impl wayland_backend_impl = {
	.name = "Wayland",
	.destroy = backend_destroy,
	.exe = backend_exe,
};

struct wlf_backend *wayland_backend_create(void) {
	struct wlf_wl_backend *backend = calloc(1, sizeof(struct wlf_wl_backend));
	if (backend == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_backend");
		return NULL;
	}

	wlf_backend_init(&backend->base, &wayland_backend_impl);
	wlf_linked_list_init(&backend->interfaces);
	backend->display = wl_display_connect(NULL);
	if (backend->display == NULL) {
		wlf_log(WLF_ERROR, "No Wayland display connection");
		goto failed;
	}

	backend->registry = wl_display_get_registry(backend->display);
	if (backend->registry == NULL) {
		wlf_log(WLF_ERROR, "Failed to get %s registry", backend->base.impl->name);
		goto failed;
	}

	wlf_signal_init(&backend->events.global_add);
	wlf_signal_init(&backend->events.global_remove);
	wl_registry_add_listener(backend->registry, &wl_base_registry_listener, backend);
	wl_display_roundtrip(backend->display);

	wlf_log(WLF_DEBUG, "Created %s backend", backend->base.impl->name);

	return &backend->base;

failed:
	wlf_backend_destroy(&backend->base);

	return NULL;
}

bool wlf_backend_is_wayland(const struct wlf_backend *backend) {
	return backend->impl == &wayland_backend_impl;
}

struct wlf_wl_backend *wlf_wl_backend_from_backend(struct wlf_backend *wlf_backend) {
	assert(wlf_backend && wlf_backend->impl == &wayland_backend_impl);

	struct wlf_wl_backend *backend = wlf_container_of(wlf_backend, backend, base);

	return backend;
}
