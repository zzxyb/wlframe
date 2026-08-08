#include "wlf/platform/wlf_backend.h"
#include "wlf/pass/gles/rect_pass.h"
#include "wlf/pass/gles/render_target_info.h"
#include "wlf/pass/pixman/rect_pass.h"
#include "wlf/pass/pixman/render_target_info.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/renderer/pixman/renderer.h"
#include "wlf/scene/wlf_rect_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/swapchain/egl/swapchain.h"
#include "wlf/swapchain/shm/swapchain.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/window/wayland/xdg_toplevel_window.h"
#include "wlf/window/wlf_window.h"
#include "wlf/utils/wlf_env.h"

#include <stdlib.h>

struct render_state {
	struct wlf_listener expose;
	struct wlf_rect_pass *rect_pass;
	struct wlf_rect_node *rect;
};

static void render_scene(struct render_state *state, struct wlf_window *window,
		struct wlf_render_target_info *target, const pixman_region32_t *damage) {
	struct wlf_render_rect_options background = {
		.box = {
			.x = 0,
			.y = 0,
			.width = window->state.geometry.width,
			.height = window->state.geometry.height,
		},
		.color = WLF_COLOR_DARK_GRAY,
		.clip = damage,
		.blend_mode = WLF_RENDER_BLEND_MODE_NONE,
	};
	wlf_render_pass_add_rect(state->rect_pass, target, &background);
	wlf_rect_node_render(state->rect, state->rect_pass, target, damage);
}

static void handle_expose(struct wlf_listener *listener, void *data) {
	struct render_state *state =
		wlf_container_of(listener, state, expose);
	struct wlf_window *window = data;

	pixman_region32_t damage;
	pixman_region32_init_rect(&damage, 0, 0,
		window->state.geometry.width, window->state.geometry.height);

	if (wlf_renderer_is_gles(window->state.renderer) &&
			wlf_swapchain_is_egl(window->state.swapchain)) {
		struct wlf_egl_swapchain *swapchain =
			wlf_egl_swapchain_from_swapchain(window->state.swapchain);
		struct wlf_gles_render_target_info *target =
			wlf_gles_begin_egl_render_pass(swapchain);
		if (target != NULL) {
			render_scene(state, window, &target->base, &damage);
			wlf_render_target_info_destroy(&target->base);
			wlf_swapchain_present(window->state.swapchain, &damage);
		}
	} else if (wlf_renderer_is_pixman(window->state.renderer) &&
			wlf_swapchain_is_shm(window->state.swapchain)) {
		struct wlf_shm_swapchain *swapchain =
			wlf_shm_swapchain_from_swapchain(window->state.swapchain);
		struct wlf_pixman_renderer *renderer =
			wlf_pixman_renderer_from_renderer(window->state.renderer);
		struct wlf_pixman_buffer *buffer =
			wlf_pixman_buffer_get(renderer, swapchain->back);
		if (buffer == NULL) {
			buffer = wlf_pixman_buffer_create(renderer, swapchain->back);
		}
		if (buffer != NULL) {
			struct wlf_pixman_render_target_info *target =
				wlf_pixman_begin_pixman_render_pass(buffer);
			if (target != NULL) {
				render_scene(state, window, &target->base, &damage);
				wlf_render_target_info_destroy(&target->base);
				wlf_swapchain_present(window->state.swapchain, &damage);
			}
		}
	}
	pixman_region32_fini(&damage);
}

static struct wlf_rect_pass *create_rect_pass(struct wlf_renderer *renderer) {
	if (wlf_renderer_is_gles(renderer)) {
		return wlf_gles_rect_pass_create();
	}
	if (wlf_renderer_is_pixman(renderer)) {
		return wlf_pixman_rect_pass_create();
	}

	wlf_log(WLF_ERROR, "No rect pass for selected renderer");
	return NULL;
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	wlf_log_init(WLF_DEBUG, NULL);
	struct wlf_backend *backend = wlf_backend_autocreate();
	if (backend == NULL) {
		wlf_log(WLF_ERROR, "Failed to auto-create backend");
		return EXIT_FAILURE;
	}

	struct wlf_renderer *renderer = wlf_renderer_autocreate(backend);
	if (renderer == NULL) {
		wlf_log(WLF_ERROR, "Failed to create render");
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}

	struct wlf_window *window =
		wlf_xdg_toplevel_window_create_from_backend(backend, 400, 300);
	wlf_window_init_renderer(window, renderer);

	struct wlf_scene_tree *tree = wlf_root_scene_tree_create();
	if (tree == NULL) {
		wlf_log(WLF_ERROR, "Failed to create scene tree");
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	window->tree = tree;
	tree->base.window = window;

	struct render_state render = {
		.expose = {
			.notify = handle_expose,
		},
		.rect_pass = create_rect_pass(renderer),
	};
	if (render.rect_pass == NULL) {
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}

	struct wlf_color rect_color = wlf_color_from_rgba8(64, 148, 255, 230);
	render.rect = wlf_rect_node_create(&tree->base, 80, 70, 240, 140,
		&rect_color);
	if (render.rect == NULL) {
		wlf_render_rect_pass_destroy(render.rect_pass);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	wlf_signal_add(&window->events.expose, &render.expose);

	wlf_window_show(window);
	wlf_log(WLF_INFO, "Backend started successfully");

	wlf_backend_exe(backend);
	wlf_linked_list_remove(&render.expose.link);
	wlf_render_rect_pass_destroy(render.rect_pass);
	wlf_backend_destroy(backend);

	return EXIT_SUCCESS;
}
