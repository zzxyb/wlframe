#include "wlf/window/wlf_window.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/window/wlf_titlebar.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static uint32_t get_render_format(bool has_alpha) {
	if (has_alpha) {
		return WLF_FORMAT_ARGB8888;
	}

	return WLF_FORMAT_XRGB8888;
}

void wlf_window_init(struct wlf_window *window, enum wlf_window_type type,
		const struct wlf_window_impl *impl, struct wlf_backend *backend,
		uint32_t width, uint32_t height) {
	assert(impl->destroy);

	*window = (struct wlf_window){
		.impl = impl,
		.state.type = type,
		.state.opacity = 1.0f,
		.state.scale = 1.0,
		.state.background_color = WLF_COLOR_BLACK,
		.state.backend = backend,
		.state.server_side_decorated =
			wlf_backend_supports_server_side_decorations(backend),
		.state.geometry = {
			.width = (int)width,
			.height = (int)height,
		},
		.features = {
			.enable_set_position = impl->set_position != NULL,
			.enable_set_min_size = impl->set_min_size != NULL,
			.enable_set_max_size = impl->set_max_size != NULL,
		},
	};

	wlf_render_format_init(&window->state.format, get_render_format(false));

	wlf_signal_init(&window->events.destroy);
	wlf_signal_init(&window->events.expose);
	wlf_signal_init(&window->events.resize);
	wlf_signal_init(&window->events.move);
	wlf_signal_init(&window->events.close);
	wlf_signal_init(&window->events.focus_in);
	wlf_signal_init(&window->events.focus_out);
	wlf_signal_init(&window->events.scale);
	wlf_signal_init(&window->events.show);
	wlf_signal_init(&window->events.hide);
}

void wlf_window_destroy(struct wlf_window *window) {
	if (window == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&window->events.destroy, window);
	if (window->scene != NULL) {
		wlf_scene_destroy(window->scene);
	}
	wlf_swapchain_destroy(window->state.swapchain);
	window->state.swapchain = NULL;
	free(window->state.title);
	wlf_render_format_finish(&window->state.format);
	if (window->impl->destroy) {
		window->impl->destroy(window);
	} else {
		free(window);
	}
}

void wlf_window_close(struct wlf_window *window) {
	if (window->impl->close) {
		window->impl->close(window);
	}

	window->state.visible = false;
	wlf_signal_emit_mutable(&window->events.close, window);
}

void wlf_window_show(struct wlf_window *window) {
	if (window->impl->show) {
		window->impl->show(window);
	}

	window->state.visible = true;
	wlf_signal_emit_mutable(&window->events.show, window);
}

void wlf_window_hide(struct wlf_window *window) {
	if (window->impl->hide) {
		window->impl->hide(window);
	}

	window->state.visible = false;
	wlf_signal_emit_mutable(&window->events.hide, window);
}

void wlf_window_set_title(struct wlf_window *window, const char *title) {
	char *new_title = strdup(title);
	if (new_title == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to duplicate window title");
		return;
	}

	free(window->state.title);
	window->state.title = new_title;

	if (window->impl->set_title) {
		window->impl->set_title(window, new_title);
	}
	if (window->scene != NULL && window->scene->titlebar != NULL) {
		wlf_titlebar_set_title(window->scene->titlebar, new_title);
	}
}

void wlf_window_set_geometry(struct wlf_window *window,
		const struct wlf_rect *geometry) {
	bool resized = window->state.geometry.width != geometry->width ||
		window->state.geometry.height != geometry->height;
	window->state.geometry = *geometry;
	if (window->impl->set_geometry) {
		window->impl->set_geometry(window, &window->state.geometry);
	}
	if (resized) {
		wlf_signal_emit_mutable(&window->events.resize, window);
	}
}

void wlf_window_set_size(struct wlf_window *window, int width, int height) {
	bool resized = window->state.geometry.width != width ||
		window->state.geometry.height != height;
	window->state.geometry.width = width;
	window->state.geometry.height = height;

	if (window->impl->set_size) {
		window->impl->set_size(window, width, height);
	}
	if (resized) {
		wlf_signal_emit_mutable(&window->events.resize, window);
	}
}

void wlf_window_set_min_size(struct wlf_window *window, int width, int height) {
	window->state.min_size.width = width;
	window->state.min_size.height = height;

	if (window->impl->set_min_size) {
		window->impl->set_min_size(window, width, height);
	}
}

void wlf_window_set_max_size(struct wlf_window *window, int width, int height) {
	window->state.max_size.width = width;
	window->state.max_size.height = height;

	if (window->impl->set_max_size) {
		window->impl->set_max_size(window, width, height);
	}
}

void wlf_window_set_position(struct wlf_window *window, int x, int y) {
	if (window->impl->set_position) {
		window->impl->set_position(window, x, y);
	}

	window->state.geometry.x = x;
	window->state.geometry.y = y;
	wlf_signal_emit_mutable(&window->events.move, window);
}

uint32_t wlf_window_scale_length(const struct wlf_window *window,
		uint32_t logical_length) {
	assert(window != NULL);
	double scaled = ceil(logical_length * window->state.scale);
	return scaled >= INT_MAX ? INT_MAX : (uint32_t)scaled;
}

void wlf_window_set_scale(struct wlf_window *window, double scale) {
	assert(window != NULL);
	if (!isfinite(scale) || scale <= 0.0) {
		wlf_log(WLF_ERROR, "Ignoring invalid window scale %.2f", scale);
		return;
	}
	if (window->state.scale == scale) {
		return;
	}

	window->state.scale = scale;
	wlf_signal_emit_mutable(&window->events.scale, window);
	if (window->scene != NULL) {
		wlf_scene_damage_whole(window->scene);
	}
}

void wlf_window_set_state(struct wlf_window *window,
		enum wlf_window_state_flags state) {
	if (window->impl->set_state) {
		window->impl->set_state(window, state);
	}

	window->state.state = state;
	if (window->scene != NULL) {
		bool client_side = !window->state.server_side_decorated &&
			!(state & WLF_WINDOW_FULLSCREEN);
		if (!wlf_scene_set_client_side_decorated(window->scene, client_side)) {
			wlf_log(WLF_ERROR, "Failed to update client-side decoration state");
		}
	}
}

void wlf_window_set_flags(struct wlf_window *window, uint32_t flags) {
	window->state.flags = flags;

	if (window->impl->set_flags) {
		window->impl->set_flags(window, flags);
	}
}

void wlf_window_set_input_region(struct wlf_window *window,
		const pixman_region32_t *region) {
	if (window->impl->set_input_region) {
		window->impl->set_input_region(window, region);
	}
}

void wlf_window_set_opaque_region(struct wlf_window *window,
		const pixman_region32_t *region) {
	if (window->impl->set_opaque_region) {
		window->impl->set_opaque_region(window, region);
	}
}

void wlf_window_set_opacity(struct wlf_window *window, float opacity) {
	window->state.opacity = opacity;

	if (window->impl->set_opacity) {
		window->impl->set_opacity(window, opacity);
	}
}

void wlf_window_set_mask(struct wlf_window *window,
		const pixman_region32_t *mask) {
	if (window->impl->set_mask) {
		window->impl->set_mask(window, mask);
	}
}

void wlf_window_set_background_color(struct wlf_window *window,
		const struct wlf_color *color) {
	window->state.background_color = *color;

	if (window->impl->set_background_color) {
		window->impl->set_background_color(window, &window->state.background_color);
	}
	if (window->scene != NULL) {
		wlf_scene_damage_whole(window->scene);
	}
}

void *wlf_window_native_handle(struct wlf_window *window) {
	if (window == NULL || window->impl->native_handle == NULL) {
		return NULL;
	}

	return window->impl->native_handle(window);
}

void wlf_window_init_renderer(struct wlf_window *window, struct wlf_renderer *renderer) {
	window->state.renderer = renderer;
	window->state.swapchain =
		wlf_swapchain_auto_create(window,
			(int)wlf_window_scale_length(window,
				(uint32_t)window->state.geometry.width),
			(int)wlf_window_scale_length(window,
				(uint32_t)window->state.geometry.height),
			&window->state.format);
}

void wlf_window_schedule_frame(struct wlf_window *window) {
	if (window == NULL) {
		return;
	}

	if (window->impl->schedule_frame != NULL) {
		window->impl->schedule_frame(window);
		return;
	}

	/* Backends without explicit frame callbacks render on the expose signal. */
	wlf_signal_emit_mutable(&window->events.expose, window);
}
