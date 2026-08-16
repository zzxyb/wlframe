#include "wlf/platform/wlf_backend.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/scene/wlf_rect_node.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/window/wlf_window.h"

#include <stdlib.h>

static void close_after_first_frame(struct wlf_listener *listener,
		void *data) {
	(void)listener;
	wlf_window_close(data);
}

int main(void) {
	wlf_log_init(WLF_DEBUG, NULL);
	struct wlf_backend *backend = wlf_backend_autocreate();
	if (backend == NULL) {
		return EXIT_FAILURE;
	}
	struct wlf_renderer *renderer = wlf_renderer_autocreate(backend);
	if (renderer == NULL) {
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	struct wlf_window *window = wlf_window_create_toplevel(backend, 640, 400);
	if (window == NULL) {
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	wlf_window_init_renderer(window, renderer);
	wlf_window_set_title(window, "wlframe native window");
	struct wlf_color background = wlf_color_from_rgb8(35, 42, 54);
	wlf_window_set_background_color(window, &background);
	struct wlf_scene *scene = wlf_scene_create(window);
	struct wlf_color foreground = wlf_color_from_rgb8(55, 145, 245);
	if (scene == NULL || wlf_rect_node_create(&scene->tree->base,
			80, 70, 480, 260, &foreground) == NULL) {
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	struct wlf_listener first_frame = {
		.notify = close_after_first_frame,
	};
	wlf_signal_add(&window->events.expose, &first_frame);
	wlf_window_show(window);
	wlf_backend_exe(backend);
	wlf_linked_list_remove(&first_frame.link);
	wlf_window_destroy(window);
	wlf_renderer_destroy(renderer);
	wlf_backend_destroy(backend);
	return EXIT_SUCCESS;
}
