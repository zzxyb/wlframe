#include "wlf/window/wlf_window.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void wlf_window_init(struct wlf_window *window, enum wlf_window_type type,
		const struct wlf_window_impl *impl, uint32_t width, uint32_t height) {
	assert(impl->destroy);

	*window = (struct wlf_window){
		.impl = impl,
		.state.type = type,
		.state.opacity = 1.0f,
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

	wlf_signal_init(&window->events.destroy);
	wlf_signal_init(&window->events.expose);
	wlf_signal_init(&window->events.resize);
	wlf_signal_init(&window->events.move);
	wlf_signal_init(&window->events.close);
	wlf_signal_init(&window->events.focus_in);
	wlf_signal_init(&window->events.focus_out);
	wlf_signal_init(&window->events.show);
	wlf_signal_init(&window->events.hide);
}

void wlf_window_destroy(struct wlf_window *window) {
	if (window == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&window->events.destroy, window);
	free(window->state.title);

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
}

void wlf_window_set_geometry(struct wlf_window *window,
		const struct wlf_rect *geometry) {
	window->state.geometry = *geometry;
	if (window->impl->set_geometry) {
		window->impl->set_geometry(window, &window->state.geometry);
	}
}

void wlf_window_set_size(struct wlf_window *window, int width, int height) {
	window->state.geometry.width = width;
	window->state.geometry.height = height;

	if (window->impl->set_size) {
		window->impl->set_size(window, width, height);
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

void wlf_window_set_state(struct wlf_window *window,
		enum wlf_window_state_flags state) {
	if (window->impl->set_state) {
		window->impl->set_state(window, state);
	}

	window->state.state = state;
}

void wlf_window_set_flags(struct wlf_window *window, uint32_t flags) {
	window->state.flags = flags;

	if (window->impl->set_flags) {
		window->impl->set_flags(window, flags);
	}
}

void wlf_window_set_input_region(struct wlf_window *window,
		const struct wlf_region *region) {
	if (window->impl->set_input_region) {
		window->impl->set_input_region(window, region);
	}
}

void wlf_window_set_opaque_region(struct wlf_window *window,
		const struct wlf_region *region) {
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
		const struct wlf_region *mask) {
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
}
