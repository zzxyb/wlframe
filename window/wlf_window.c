#include "wlf/window/wlf_window.h"
#include "wlf/item/wlf_item.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

// Default implementation for basic window operations
static bool default_create(struct wlf_window *window) {
	wlf_log(WLF_DEBUG, "Default window create");
	return true;
}

static void default_destroy(struct wlf_window *window) {
	wlf_log(WLF_DEBUG, "Default window destroy");
}

static void default_show(struct wlf_window *window) {
	wlf_log(WLF_DEBUG, "Default window show");
	window->visible = true;
	window->visibility = WLF_WINDOW_NORMAL;
}

static void default_hide(struct wlf_window *window) {
	wlf_log(WLF_DEBUG, "Default window hide");
	window->visible = false;
	window->visibility = WLF_WINDOW_SUSPENDED;
}

static void default_set_title(struct wlf_window *window, const char *title) {
	wlf_log(WLF_DEBUG, "Default set title: %s", title);
}

static void default_set_geometry(struct wlf_window *window, const struct wlf_rect *geometry) {
	wlf_log(WLF_DEBUG, "Default set geometry: %d,%d %dx%d",
		geometry->x, geometry->y, geometry->width, geometry->height);
	window->geometry = *geometry;
}

static void default_set_visibility(struct wlf_window *window, enum wlf_window_state_flags visibility) {
	wlf_log(WLF_DEBUG, "Default set visibility: %d", visibility);
	window->visibility = visibility;
	window->visible = (visibility != WLF_WINDOW_SUSPENDED);
}

static bool default_is_visible(const struct wlf_window *window) {
	return window->visible;
}

static struct wlf_rect default_geometry(const struct wlf_window *window) {
	return window->geometry;
}

// Default implementation structure
static const struct wlf_window_impl default_impl = {
	.create = default_create,
	.destroy = default_destroy,
	.show = default_show,
	.hide = default_hide,
	.set_title = default_set_title,
	.set_geometry = default_set_geometry,
	.set_visibility = default_set_visibility,
	.is_visible = default_is_visible,
	.geometry = default_geometry,
};

struct wlf_window *wlf_window_create(enum wlf_window_type type) {
	struct wlf_window *window = calloc(1, sizeof(struct wlf_window));
	if (!window) {
		wlf_log(WLF_ERROR, "Failed to allocate memory for window");
		return NULL;
	}

	// Initialize with default implementation
	window->impl = &default_impl;
	window->type = type;
	window->visibility = WLF_WINDOW_SUSPENDED;
	window->flags = WLF_WINDOW_FLAG_DECORATED | WLF_WINDOW_FLAG_RESIZABLE;
	window->geometry = wlf_rect_make(100, 100, 800, 600); // Default size and position

	// Initialize signals
	wlf_signal_init(&window->events.expose);
	wlf_signal_init(&window->events.resize);
	wlf_signal_init(&window->events.move);
	wlf_signal_init(&window->events.close);
	wlf_signal_init(&window->events.focus_in);
	wlf_signal_init(&window->events.focus_out);
	wlf_signal_init(&window->events.show);
	wlf_signal_init(&window->events.hide);

	// Call platform-specific create
	if (window->impl->create && !window->impl->create(window)) {
		wlf_log(WLF_ERROR, "Failed to create window");
		wlf_window_destroy(window);
		return NULL;
	}

	// Create root item (using basic item for now instead of tree)
	struct wlf_item *root_item = wlf_item_create(window);
	if (!root_item) {
		wlf_log(WLF_ERROR, "Failed to create root item for window");
		wlf_window_destroy(window);
		return NULL;
	}

	// Set root item to cover the entire window
	struct wlf_rect root_geometry = {0, 0, window->geometry.width, window->geometry.height};
	wlf_item_set_geometry(root_item, &root_geometry);
	wlf_item_set_visible(root_item, true);

	window->root_item = (struct wlf_item_tree*)root_item; // Cast for compatibility

	wlf_log(WLF_INFO, "Created window of type %d", type);
	return window;
}

void wlf_window_destroy(struct wlf_window *window) {
	if (!window) {
		return;
	}

	// Call platform-specific destroy
	if (window->impl && window->impl->destroy) {
		window->impl->destroy(window);
	}

	// Destroy root item
	if (window->root_item) {
		// Cast back to basic item and destroy
		struct wlf_item *root_item = (struct wlf_item*)window->root_item;
		wlf_item_destroy(root_item);
		window->root_item = NULL;
	}

	// Free title
	free(window->title);

	// Free the window structure
	free(window);
	wlf_log(WLF_DEBUG, "Window destroyed");
}

void wlf_window_show(struct wlf_window *window) {
	if (!window) {
		return;
	}

	if (window->impl && window->impl->show) {
		window->impl->show(window);
	}

	// Emit show event
	struct wlf_window_event event = {
		.type = WLF_WINDOW_EVENT_SHOW,
		.window = window
	};
	wlf_window_emit_event(window, &event);
}

void wlf_window_hide(struct wlf_window *window) {
	if (!window) {
		return;
	}

	if (window->impl && window->impl->hide) {
		window->impl->hide(window);
	}

	// Emit hide event
	struct wlf_window_event event = {
		.type = WLF_WINDOW_EVENT_HIDE,
		.window = window
	};
	wlf_window_emit_event(window, &event);
}

void wlf_window_set_title(struct wlf_window *window, const char *title) {
	if (!window || !title) {
		return;
	}

	// Update internal title
	free(window->title);
	window->title = strdup(title);

	// Call platform-specific implementation
	if (window->impl && window->impl->set_title) {
		window->impl->set_title(window, title);
	}
}

const char *wlf_window_get_title(const struct wlf_window *window) {
	return window ? window->title : NULL;
}

void wlf_window_set_geometry(struct wlf_window *window, const struct wlf_rect *geometry) {
	if (!window || !geometry) {
		return;
	}

	struct wlf_rect old_geometry = window->geometry;

	// Call platform-specific implementation
	if (window->impl && window->impl->set_geometry) {
		window->impl->set_geometry(window, geometry);
	}

	// Emit events if size or position changed
	if (old_geometry.width != geometry->width || old_geometry.height != geometry->height) {
		struct wlf_window_event event = {
			.type = WLF_WINDOW_EVENT_RESIZE,
			.window = window,
			.data.resize = { geometry->width, geometry->height }
		};
		wlf_window_emit_event(window, &event);
	}

	if (old_geometry.x != geometry->x || old_geometry.y != geometry->y) {
		struct wlf_window_event event = {
			.type = WLF_WINDOW_EVENT_MOVE,
			.window = window,
			.data.move = { geometry->x, geometry->y }
		};
		wlf_window_emit_event(window, &event);
	}
}

struct wlf_rect wlf_window_get_geometry(const struct wlf_window *window) {
	if (!window) {
		return wlf_rect_make(0, 0, 0, 0);
	}

	if (window->impl && window->impl->geometry) {
		return window->impl->geometry(window);
	}

	return window->geometry;
}

void wlf_window_set_size(struct wlf_window *window, int width, int height) {
	if (!window) {
		return;
	}

	struct wlf_rect geometry = window->geometry;
	geometry.width = width;
	geometry.height = height;
	wlf_window_set_geometry(window, &geometry);
}

void wlf_window_get_size(const struct wlf_window *window, int *width, int *height) {
	if (!window) {
		if (width) *width = 0;
		if (height) *height = 0;
		return;
	}

	struct wlf_rect geometry = wlf_window_get_geometry(window);
	if (width) *width = geometry.width;
	if (height) *height = geometry.height;
}

void wlf_window_set_position(struct wlf_window *window, int x, int y) {
	if (!window) {
		return;
	}

	struct wlf_rect geometry = window->geometry;
	geometry.x = x;
	geometry.y = y;
	wlf_window_set_geometry(window, &geometry);
}

void wlf_window_get_position(const struct wlf_window *window, int *x, int *y) {
	if (!window) {
		if (x) *x = 0;
		if (y) *y = 0;
		return;
	}

	struct wlf_rect geometry = wlf_window_get_geometry(window);
	if (x) *x = geometry.x;
	if (y) *y = geometry.y;
}

void wlf_window_set_visibility(struct wlf_window *window, enum wlf_window_state_flags visibility) {
	if (!window) {
		return;
	}

	if (window->impl && window->impl->set_visibility) {
		window->impl->set_visibility(window, visibility);
	}
}

enum wlf_window_state_flags wlf_window_get_visibility(const struct wlf_window *window) {
	return window ? window->visibility : WLF_WINDOW_SUSPENDED;
}

bool wlf_window_is_visible(const struct wlf_window *window) {
	if (!window) {
		return false;
	}

	if (window->impl && window->impl->is_visible) {
		return window->impl->is_visible(window);
	}

	return window->visible;
}

void wlf_window_set_flags(struct wlf_window *window, uint32_t flags) {
	if (!window) {
		return;
	}

	window->flags = flags;
}

uint32_t wlf_window_get_flags(const struct wlf_window *window) {
	return window ? window->flags : 0;
}

bool wlf_window_has_focus(const struct wlf_window *window) {
	return window ? window->focused : false;
}

void wlf_window_request_focus(struct wlf_window *window) {
	if (!window) {
		return;
	}

	// Platform-specific focus implementation would go here
	window->focused = true;

	// Emit focus event
	struct wlf_window_event event = {
		.type = WLF_WINDOW_EVENT_FOCUS_IN,
		.window = window
	};
	wlf_window_emit_event(window, &event);
}

void wlf_window_raise(struct wlf_window *window) {
	if (!window) {
		return;
	}

	// Platform-specific raise implementation would go here
	wlf_log(WLF_DEBUG, "Window raised");
}

void wlf_window_lower(struct wlf_window *window) {
	if (!window) {
		return;
	}

	// Platform-specific lower implementation would go here
	wlf_log(WLF_DEBUG, "Window lowered");
}

void wlf_window_close(struct wlf_window *window) {
	if (!window) {
		return;
	}

	// Emit close event
	struct wlf_window_event event = {
		.type = WLF_WINDOW_EVENT_CLOSE,
		.window = window
	};
	wlf_window_emit_event(window, &event);
}

void wlf_window_process_events(struct wlf_window *window) {
	if (!window) {
		return;
	}

	// Platform-specific event processing would go here
	// For now, this is a no-op
}

void wlf_window_emit_event(struct wlf_window *window, const struct wlf_window_event *event) {
	if (!window || !event) {
		return;
	}

	struct wlf_signal *signal = NULL;

	switch (event->type) {
		case WLF_WINDOW_EVENT_EXPOSE:
			signal = &window->events.expose;
			break;
		case WLF_WINDOW_EVENT_RESIZE:
			signal = &window->events.resize;
			break;
		case WLF_WINDOW_EVENT_MOVE:
			signal = &window->events.move;
			break;
		case WLF_WINDOW_EVENT_CLOSE:
			signal = &window->events.close;
			break;
		case WLF_WINDOW_EVENT_FOCUS_IN:
			signal = &window->events.focus_in;
			break;
		case WLF_WINDOW_EVENT_FOCUS_OUT:
			signal = &window->events.focus_out;
			break;
		case WLF_WINDOW_EVENT_SHOW:
			signal = &window->events.show;
			break;
		case WLF_WINDOW_EVENT_HIDE:
			signal = &window->events.hide;
			break;
	}

	if (signal) {
		wlf_signal_emit(signal, (void*)event);
	}
}

// ===== UI System - Item Management =====

struct wlf_item_tree *wlf_window_get_root_item(const struct wlf_window *window) {
	return window ? window->root_item : NULL;
}

void wlf_window_set_root_item(struct wlf_window *window, struct wlf_item_tree *root_item) {
	if (!window) return;

	// 销毁旧的root_item
	if (window->root_item) {
		// Cast back to basic item and destroy
		struct wlf_item *old_root = (struct wlf_item*)window->root_item;
		wlf_item_destroy(old_root);
	}

	// 设置新的root_item
	window->root_item = root_item;

	wlf_log(WLF_DEBUG, "Set new root item for window");
}

void wlf_window_render(struct wlf_window *window, struct wlf_renderer *renderer) {
	if (!window || !renderer || !window->root_item) {
		return;
	}

	// 设置渲染裁剪区域为整个窗口
	struct wlf_rect window_rect = {
		.x = 0,
		.y = 0,
		.width = window->geometry.width,
		.height = window->geometry.height
	};

	// 递归渲染整个item树 (cast root_item back to basic item)
	struct wlf_item *root_item = (struct wlf_item*)window->root_item;
	wlf_item_render_recursive(root_item, renderer, &window_rect);

	wlf_log(WLF_DEBUG, "Rendered window with root item");
}
