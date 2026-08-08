#include "wlf/wayland/wlf_wl_cursor.h"

#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_utils.h"
#include "wayland/protocols/cursor-shape-v1-client-protocol.h"

#include <stdlib.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>

struct wlf_wl_cursor {
	struct wlf_cursor base;
	struct wl_pointer *pointer;
	struct wp_cursor_shape_device_v1 *shape_device;
	struct wl_cursor_theme *theme;
	struct wl_surface *surface;
};

static const char *cursor_shape_name(enum wlf_cursor_shape shape) {
	switch (shape) {
	case WLF_CURSOR_SHAPE_POINTER: return "pointer";
	case WLF_CURSOR_SHAPE_TEXT: return "text";
	case WLF_CURSOR_SHAPE_CROSSHAIR: return "crosshair";
	case WLF_CURSOR_SHAPE_MOVE: return "move";
	case WLF_CURSOR_SHAPE_GRAB: return "grab";
	case WLF_CURSOR_SHAPE_GRABBING: return "grabbing";
	case WLF_CURSOR_SHAPE_E_RESIZE: return "e-resize";
	case WLF_CURSOR_SHAPE_N_RESIZE: return "n-resize";
	case WLF_CURSOR_SHAPE_NE_RESIZE: return "ne-resize";
	case WLF_CURSOR_SHAPE_NW_RESIZE: return "nw-resize";
	case WLF_CURSOR_SHAPE_S_RESIZE: return "s-resize";
	case WLF_CURSOR_SHAPE_SE_RESIZE: return "se-resize";
	case WLF_CURSOR_SHAPE_SW_RESIZE: return "sw-resize";
	case WLF_CURSOR_SHAPE_W_RESIZE: return "w-resize";
	case WLF_CURSOR_SHAPE_EW_RESIZE: return "ew-resize";
	case WLF_CURSOR_SHAPE_NS_RESIZE: return "ns-resize";
	case WLF_CURSOR_SHAPE_NESW_RESIZE: return "nesw-resize";
	case WLF_CURSOR_SHAPE_NWSE_RESIZE: return "nwse-resize";
	case WLF_CURSOR_SHAPE_NOT_ALLOWED: return "not-allowed";
	case WLF_CURSOR_SHAPE_WAIT: return "wait";
	case WLF_CURSOR_SHAPE_HELP: return "help";
	default: return "default";
	}
}

static bool wl_cursor_set_shape(struct wlf_cursor *base, uint32_t serial,
		enum wlf_cursor_shape shape) {
	struct wlf_wl_cursor *cursor =
		wlf_container_of(base, cursor, base);
	if (cursor->shape_device != NULL) {
		uint32_t version = wl_proxy_get_version(
			(struct wl_proxy *)cursor->shape_device);
		if (version < 2 && shape >= WLF_CURSOR_SHAPE_DND_ASK) {
			shape = WLF_CURSOR_SHAPE_DEFAULT;
		}
		wp_cursor_shape_device_v1_set_shape(cursor->shape_device,
			serial, (uint32_t)shape);
		return true;
	}
	if (cursor->theme == NULL || cursor->surface == NULL) {
		return false;
	}
	struct wl_cursor *theme_cursor = wl_cursor_theme_get_cursor(cursor->theme,
		cursor_shape_name(shape));
	if (theme_cursor == NULL || theme_cursor->image_count == 0) {
		theme_cursor = wl_cursor_theme_get_cursor(cursor->theme, "left_ptr");
	}
	if (theme_cursor == NULL || theme_cursor->image_count == 0) {
		return false;
	}
	struct wl_cursor_image *image = theme_cursor->images[0];
	struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
	if (buffer == NULL) {
		return false;
	}
	wl_pointer_set_cursor(cursor->pointer, serial, cursor->surface,
		(int32_t)image->hotspot_x, (int32_t)image->hotspot_y);
	wl_surface_attach(cursor->surface, buffer, 0, 0);
	wl_surface_damage_buffer(cursor->surface, 0, 0,
		(int32_t)image->width, (int32_t)image->height);
	wl_surface_commit(cursor->surface);
	return true;
}

static void wl_cursor_destroy(struct wlf_cursor *base) {
	struct wlf_wl_cursor *cursor =
		wlf_container_of(base, cursor, base);
	if (cursor->shape_device != NULL) {
		wp_cursor_shape_device_v1_destroy(cursor->shape_device);
	}
	if (cursor->surface != NULL) {
		wl_surface_destroy(cursor->surface);
	}
	if (cursor->theme != NULL) {
		wl_cursor_theme_destroy(cursor->theme);
	}
	free(cursor);
}

static const struct wlf_cursor_impl cursor_impl = {
	.destroy = wl_cursor_destroy,
	.set_shape = wl_cursor_set_shape,
};

struct wlf_cursor *wlf_wl_cursor_create(struct wl_pointer *pointer,
		struct wp_cursor_shape_manager_v1 *shape_manager,
		struct wl_compositor *compositor, struct wl_shm *shm) {
	if (pointer == NULL || (shape_manager == NULL &&
			(compositor == NULL || shm == NULL))) {
		return NULL;
	}
	struct wlf_wl_cursor *cursor = calloc(1, sizeof(*cursor));
	if (cursor == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate Wayland cursor");
		return NULL;
	}
	wlf_cursor_init(&cursor->base, &cursor_impl);
	cursor->pointer = pointer;
	if (shape_manager != NULL) {
		cursor->shape_device = wp_cursor_shape_manager_v1_get_pointer(
			shape_manager, pointer);
	} else {
		cursor->theme = wl_cursor_theme_load(NULL, 24, shm);
		cursor->surface = wl_compositor_create_surface(compositor);
		if (cursor->theme == NULL || cursor->surface == NULL) {
			wlf_cursor_destroy(&cursor->base);
			return NULL;
		}
	}
	return &cursor->base;
}
