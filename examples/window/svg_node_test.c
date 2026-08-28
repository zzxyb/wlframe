#include "wlf/platform/wlf_backend.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/node/wlf_scene_tree.h"
#include "wlf/node/wlf_svg_node.h"
#include "wlf/svg/wlf_svg.h"
#include "wlf/utils/wlf_cmd_parser.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wayland/xdg_toplevel_window.h"
#include "wlf/window/wlf_window.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct test_state {
	struct wlf_backend *backend;
	struct wlf_listener close;
};

static void handle_close(struct wlf_listener *listener, void *data) {
	(void)data;
	struct test_state *state = wlf_container_of(listener, state, close);
	wlf_backend_quit(state->backend);
}

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("  -i, --input <path>  SVG file to display\n");
	printf("  -h, --help          Show this help message\n");
}

int main(int argc, char *argv[]) {
	char *input_path = NULL;
	bool show_help = false;
	const struct wlf_cmd_option options[] = {
		{WLF_OPTION_STRING, "input", 'i', &input_path},
		{WLF_OPTION_BOOLEAN, "help", 'h', &show_help},
	};
	wlf_cmd_parse_options(options, 2, &argc, argv);
	if (show_help) {
		print_usage(argv[0]);
		free(input_path);
		return EXIT_SUCCESS;
	}
	if (argc != 1) {
		print_usage(argv[0]);
		free(input_path);
		return EXIT_FAILURE;
	}
	const char *svg_path = input_path != NULL ? input_path :
		WLF_SVG_NODE_TEST_IMAGE;

	wlf_log_init(WLF_DEBUG, NULL);
	wlf_log(WLF_INFO, "Loading SVG: %s", svg_path);
	struct wlf_backend *backend = wlf_backend_autocreate();
	if (backend == NULL) {
		free(input_path);
		return EXIT_FAILURE;
	}
	struct wlf_renderer *renderer = wlf_renderer_autocreate(backend);
	struct wlf_window *window = wlf_xdg_toplevel_window_create_from_backend(
		backend, 720, 480);
	if (renderer == NULL || window == NULL) {
		free(input_path);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}

	wlf_window_init_renderer(window, renderer);
	struct wlf_scene *scene = wlf_scene_create(window);
	if (scene == NULL) {
		free(input_path);
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	wlf_window_set_title(window, "wlframe SVG scene node test");
	wlf_window_set_background_color(window, &WLF_COLOR_DARK_GRAY);

	struct wlf_svg_node *svg = wlf_svg_node_create_from_file(
		&scene->tree->base, 60, 60, svg_path, "px", 96.0f);
	if (svg == NULL) {
		wlf_log(WLF_ERROR, "Failed to load SVG: %s", svg_path);
		free(input_path);
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	free(input_path);

	struct test_state state = {
		.backend = backend,
		.close.notify = handle_close,
	};
	wlf_signal_add(&window->events.close, &state.close);
	wlf_window_show(window);
	wlf_backend_exe(backend);

	wlf_linked_list_remove(&state.close.link);
	wlf_window_destroy(window);
	wlf_renderer_destroy(renderer);
	wlf_backend_destroy(backend);
	return EXIT_SUCCESS;
}
