#include "wlf/platform/wlf_backend.h"
#include "wlf/config.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/scene/wlf_rect_node.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/scene/wlf_text_node.h"
#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/window/wlf_window.h"

#include <stdlib.h>
#include <string.h>

#if WLF_HAS_WINDOWS_PLATFORM
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if !WLF_HAS_WINDOWS_PLATFORM
static void close_after_first_frame(struct wlf_listener *listener,
		void *data) {
	(void)listener;
	wlf_window_close(data);
}
#endif

#if WLF_HAS_WINDOWS_PLATFORM
static void close_after_text_input(struct wlf_listener *listener,
		void *data) {
	(void)listener;
	const struct wlf_text_input_commit_event *event = data;
	if (strcmp(event->text, "A") == 0) {
		wlf_window_close(event->window);
	}
}
#endif

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
	struct wlf_color text_color = WLF_COLOR_WHITE;
	if (scene == NULL || wlf_rect_node_create(&scene->tree->base,
			80, 70, 480, 260, &foreground) == NULL ||
			wlf_text_node_create(&scene->tree->base, 120, 160,
				"wlframe Windows / \xE4\xB8\xAD\xE6\x96\x87", "sans-serif", 28,
				&text_color) == NULL) {
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
#if WLF_HAS_WINDOWS_PLATFORM
	struct wlf_listener text_input = {.notify = close_after_text_input};
	wlf_signal_add(&window->events.text_input_commit, &text_input);
#else
	struct wlf_listener first_frame = {.notify = close_after_first_frame};
	wlf_signal_add(&window->events.expose, &first_frame);
#endif
	wlf_window_show(window);
#if WLF_HAS_WINDOWS_PLATFORM
	PostMessageW(wlf_window_native_handle(window), WM_CHAR, L'A', 0);
#endif
	wlf_backend_exe(backend);
#if WLF_HAS_WINDOWS_PLATFORM
	wlf_linked_list_remove(&text_input.link);
#else
	wlf_linked_list_remove(&first_frame.link);
#endif
	wlf_window_destroy(window);
	wlf_renderer_destroy(renderer);
	wlf_backend_destroy(backend);
	return EXIT_SUCCESS;
}
