#include "wlf/platform/wlf_backend.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/scene/wlf_rect_node.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wayland/xdg_toplevel_window.h"
#include "wlf/window/wlf_window.h"

#include <stdlib.h>

struct test_state {
	struct wlf_backend *backend;
	struct wlf_window *window;
	struct wlf_rect_node *rect;
	struct wlf_listener frame_done;
	struct wlf_listener close;
};

static void handle_frame_done(struct wlf_listener *listener, void *data) {
	(void)data;
	struct test_state *state =
		wlf_container_of(listener, state, frame_done);
	double x = state->rect->base.state.x + 1.0;
	if (x >= state->window->state.geometry.width) {
		x = -state->rect->base.state.width;
	}
	wlf_scene_node_set_position(&state->rect->base, x,
		state->rect->base.state.y);
	/* Keep the animation clock running even while the moving rectangle is fully
	 * occluded and therefore produces no rendering damage. */
	wlf_window_schedule_frame(state->window);
}

static void handle_close(struct wlf_listener *listener, void *data) {
	(void)data;
	struct test_state *state = wlf_container_of(listener, state, close);
	wlf_backend_quit(state->backend);
}

int main(void) {
	wlf_log_init(WLF_DEBUG, NULL);
	struct wlf_backend *backend = wlf_backend_autocreate();
	if (backend == NULL) {
		return EXIT_FAILURE;
	}

	struct wlf_renderer *renderer = wlf_renderer_autocreate(backend);
	struct wlf_window *window = wlf_xdg_toplevel_window_create_from_backend(
		backend, 640, 360);
	if (renderer == NULL || window == NULL) {
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}

	wlf_window_init_renderer(window, renderer);
	struct wlf_scene *scene = wlf_scene_create(window);
	if (scene == NULL) {
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}

	wlf_window_set_title(window, "wlframe scene damage test");
	wlf_window_set_background_color(window, &WLF_COLOR_DARK_GRAY);
	struct wlf_color color = wlf_color_from_rgb8(64, 148, 255);
	struct wlf_rect_node *rect = wlf_rect_node_create(&scene->tree->base,
		0, 130, 120, 100, &color);
	/* Created after the moving rectangle, so this opaque rectangle is above it
	 * in scene stacking order. */
	struct wlf_color occluder_color = wlf_color_from_rgb8(40, 190, 105);
	struct wlf_rect_node *occluder = wlf_rect_node_create(&scene->tree->base,
		240, 90, 160, 180, &occluder_color);
	if (rect == NULL || occluder == NULL) {
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}

	struct test_state state = {
		.backend = backend,
		.window = window,
		.rect = rect,
		.frame_done.notify = handle_frame_done,
		.close.notify = handle_close,
	};
	wlf_signal_add(&scene->events.frame_done, &state.frame_done);
	wlf_signal_add(&window->events.close, &state.close);

	wlf_window_show(window);
	wlf_backend_exe(backend);

	wlf_linked_list_remove(&state.frame_done.link);
	wlf_linked_list_remove(&state.close.link);
	wlf_window_destroy(window);
	wlf_renderer_destroy(renderer);
	wlf_backend_destroy(backend);
	return EXIT_SUCCESS;
}
