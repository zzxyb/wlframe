#include "wlf/platform/wlf_backend.h"
#include "wlf/pass/gles/rect_pass.h"
#include "wlf/pass/gles/texture_pass.h"
#include "wlf/pass/gles/vector_pass.h"
#include "wlf/pass/gles/render_target_info.h"
#include "wlf/pass/pixman/rect_pass.h"
#include "wlf/pass/pixman/texture_pass.h"
#include "wlf/pass/pixman/vector_pass.h"
#include "wlf/pass/pixman/render_target_info.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/renderer/pixman/renderer.h"
#include "wlf/scene/wlf_rect_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/scene/wlf_texture_node.h"
#include "wlf/scene/wlf_rect_shape_node.h"
#include "wlf/scene/wlf_circle_node.h"
#include "wlf/scene/wlf_ellipse_node.h"
#include "wlf/scene/wlf_line_node.h"
#include "wlf/scene/wlf_poly_node.h"
#include "wlf/scene/wlf_path_node.h"
#include "wlf/image/wlf_image.h"
#include "wlf/swapchain/egl/swapchain.h"
#include "wlf/swapchain/shm/swapchain.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/window/wayland/xdg_toplevel_window.h"
#include "wlf/window/wlf_window.h"
#include "wlf/utils/wlf_env.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

struct render_state {
	struct wlf_listener expose;
	struct wlf_rect_pass *rect_pass;
	struct wlf_texture_pass *texture_pass;
	struct wlf_rect_shape_pass *rect_shape_pass;
	struct wlf_circle_pass *circle_pass;
	struct wlf_ellipse_pass *ellipse_pass;
	struct wlf_line_pass *line_pass;
	struct wlf_poly_pass *poly_pass;
	struct wlf_path_pass *path_pass;
	struct wlf_rect_node *rect;
	struct wlf_texture_node *image;
	struct wlf_rect_shape_node *rounded_rect;
	struct wlf_circle_node *circle;
	struct wlf_ellipse_node *ellipse;
	struct wlf_line_node *line;
	struct wlf_poly_node *poly;
	struct wlf_path_node *path;
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
	wlf_texture_node_render(state->image, state->texture_pass, target, damage);
	wlf_rect_shape_node_render(state->rounded_rect,
		state->rect_shape_pass, target, damage);
	wlf_circle_node_render(state->circle,
		state->circle_pass, target, damage);
	wlf_ellipse_node_render(state->ellipse,
		state->ellipse_pass, target, damage);
	wlf_line_node_render(state->line,
		state->line_pass, target, damage);
	wlf_poly_node_render(state->poly,
		state->poly_pass, target, damage);
	wlf_path_node_render(state->path,
		state->path_pass, target, damage);
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

static struct wlf_texture_pass *create_texture_pass(
		struct wlf_renderer *renderer) {
	if (wlf_renderer_is_gles(renderer)) {
		return wlf_gles_texture_pass_create();
	}
	if (wlf_renderer_is_pixman(renderer)) {
		return wlf_pixman_texture_pass_create();
	}
	wlf_log(WLF_ERROR, "No texture pass for selected renderer");
	return NULL;
}

static struct wlf_vector_pass *create_vector_pass(
		struct wlf_renderer *renderer) {
	if (wlf_renderer_is_gles(renderer)) {
		return wlf_gles_vector_pass_create();
	}
	if (wlf_renderer_is_pixman(renderer)) {
		return wlf_pixman_vector_pass_create();
	}
	wlf_log(WLF_ERROR, "No vector pass for selected renderer");
	return NULL;
}

static void destroy_shape_passes(struct render_state *state) {
	wlf_render_rect_shape_pass_destroy(state->rect_shape_pass);
	wlf_render_circle_pass_destroy(state->circle_pass);
	wlf_render_ellipse_pass_destroy(state->ellipse_pass);
	wlf_render_line_pass_destroy(state->line_pass);
	wlf_render_poly_pass_destroy(state->poly_pass);
	wlf_render_path_pass_destroy(state->path_pass);
}

static void set_shape_style(struct wlf_shape_state *state,
		struct wlf_color fill, struct wlf_color stroke, float stroke_width) {
	state->fill_color = fill;
	state->stroke_color = stroke;
	state->stroke_width = stroke_width;
	state->has_fill = true;
	state->has_stroke = stroke_width > 0;
}

int main(int argc, char *argv[]) {
	const char *image_path = argc > 1 ? argv[1] : WLF_WINDOW_TEST_IMAGE;

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
		wlf_xdg_toplevel_window_create_from_backend(backend, 760, 500);
	if (window == NULL) {
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	wlf_window_init_renderer(window, renderer);
	wlf_window_set_title(window, "wlframe scene shapes and image test");

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
		.texture_pass = create_texture_pass(renderer),
		.rect_shape_pass = wlf_rect_shape_pass_create(create_vector_pass(renderer)),
		.circle_pass = wlf_circle_pass_create(create_vector_pass(renderer)),
		.ellipse_pass = wlf_ellipse_pass_create(create_vector_pass(renderer)),
		.line_pass = wlf_line_pass_create(create_vector_pass(renderer)),
		.poly_pass = wlf_poly_pass_create(create_vector_pass(renderer)),
		.path_pass = wlf_path_pass_create(create_vector_pass(renderer)),
	};
	if (render.rect_pass == NULL || render.texture_pass == NULL ||
			render.rect_shape_pass == NULL || render.circle_pass == NULL ||
			render.ellipse_pass == NULL || render.line_pass == NULL ||
			render.poly_pass == NULL || render.path_pass == NULL) {
		wlf_render_rect_pass_destroy(render.rect_pass);
		wlf_render_texture_pass_destroy(render.texture_pass);
		destroy_shape_passes(&render);
		wlf_scene_node_destroy(&tree->base);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}

	struct wlf_color rect_color = wlf_color_from_rgba8(64, 148, 255, 230);
	render.rect = wlf_rect_node_create(&tree->base, 25, 25, 300, 220,
		&rect_color);
	if (render.rect == NULL) {
		wlf_scene_node_destroy(&tree->base);
		destroy_shape_passes(&render);
		wlf_render_texture_pass_destroy(render.texture_pass);
		wlf_render_rect_pass_destroy(render.rect_pass);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}

	struct wlf_image *image = wlf_image_load(image_path);
	if (image == NULL) {
		wlf_log(WLF_ERROR, "Failed to load image: %s", image_path);
		wlf_scene_node_destroy(&tree->base);
		destroy_shape_passes(&render);
		wlf_render_texture_pass_destroy(render.texture_pass);
		wlf_render_rect_pass_destroy(render.rect_pass);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	struct wlf_texture *texture = wlf_texture_from_image(renderer, image);
	double scale = fmin(270.0 / image->width, 190.0 / image->height);
	double image_width = image->width * scale;
	double image_height = image->height * scale;
	wlf_image_finish(image);
	free(image);
	if (texture == NULL) {
		wlf_log(WLF_ERROR, "Failed to create texture for: %s", image_path);
		wlf_scene_node_destroy(&tree->base);
		destroy_shape_passes(&render);
		wlf_render_texture_pass_destroy(render.texture_pass);
		wlf_render_rect_pass_destroy(render.rect_pass);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	render.image = wlf_texture_node_create(&tree->base, texture,
		40.0 + (270.0 - image_width) / 2.0,
		40.0 + (190.0 - image_height) / 2.0,
		image_width, image_height);
	if (render.image == NULL) {
		wlf_texture_destroy(texture);
		wlf_scene_node_destroy(&tree->base);
		destroy_shape_passes(&render);
		wlf_render_texture_pass_destroy(render.texture_pass);
		wlf_render_rect_pass_destroy(render.rect_pass);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}

	struct wlf_rect_shape *rounded = wlf_rect_shape_from_shape(
		wlf_rect_shape_create(0, 0, 115, 65, 18, 18));
	set_shape_style(&rounded->state,
		wlf_color_from_rgb8(55, 130, 245), WLF_COLOR_CYAN, 3);
	render.rounded_rect =
		wlf_rect_shape_node_create(&tree->base, 370, 35, rounded);

	struct wlf_circle_shape *circle = wlf_circle_shape_from_shape(
		wlf_circle_shape_create(45, 45, 42));
	set_shape_style(&circle->state,
		wlf_color_from_rgb8(245, 92, 105), WLF_COLOR_WHITE, 3);
	render.circle = wlf_circle_node_create(&tree->base, 555, 25, circle);

	struct wlf_ellipse_shape *ellipse = wlf_ellipse_shape_from_shape(
		wlf_ellipse_shape_create(64, 35, 62, 32));
	set_shape_style(&ellipse->state,
		wlf_color_from_rgb8(65, 190, 125), WLF_COLOR_YELLOW, 3);
	render.ellipse = wlf_ellipse_node_create(&tree->base, 365, 140, ellipse);

	struct wlf_line_shape *line = wlf_line_shape_from_shape(
		wlf_line_shape_create(0, 5, 125, 70));
	line->state.has_fill = false;
	line->state.has_stroke = true;
	line->state.stroke_color = WLF_COLOR_ORANGE;
	line->state.stroke_width = 9;
	render.line = wlf_line_node_create(&tree->base, 545, 135, line);

	const float star_points[] = {
		60, 0, 74, 40, 118, 40, 82, 65, 96, 108,
		60, 82, 24, 108, 38, 65, 2, 40, 46, 40,
	};
	struct wlf_poly_shape *poly = wlf_poly_shape_from_shape(
		wlf_poly_shape_create(star_points, 10, true));
	set_shape_style(&poly->state,
		wlf_color_from_rgb8(150, 85, 220), WLF_COLOR_MAGENTA, 3);
	render.poly = wlf_poly_node_create(&tree->base, 370, 275, poly);

	struct wlf_path *path = calloc(1, sizeof(*path));
	path->npts = 7;
	path->closed = true;
	path->pts = malloc((size_t)path->npts * 2 * sizeof(*path->pts));
	const float path_points[] = {
		0, 35, 28, 0, 62, 22, 95, 0, 125, 38, 95, 78, 25, 78,
	};
	memcpy(path->pts, path_points, sizeof(path_points));
	struct wlf_path_shape *path_shape = wlf_path_shape_from_shape(
		wlf_path_shape_create(path, true));
	set_shape_style(&path_shape->state,
		wlf_color_from_rgb8(30, 175, 190), WLF_COLOR_WHITE, 4);
	render.path = wlf_path_node_create(&tree->base, 550, 285, path_shape);

	if (render.rounded_rect == NULL || render.circle == NULL ||
			render.ellipse == NULL || render.line == NULL ||
			render.poly == NULL || render.path == NULL) {
		wlf_log(WLF_ERROR, "Failed to create shape scene node");
		wlf_scene_node_destroy(&tree->base);
		destroy_shape_passes(&render);
		wlf_render_texture_pass_destroy(render.texture_pass);
		wlf_render_rect_pass_destroy(render.rect_pass);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	wlf_log(WLF_INFO, "Displaying image: %s", image_path);
	wlf_signal_add(&window->events.expose, &render.expose);

	wlf_window_show(window);
	wlf_log(WLF_INFO, "Backend started successfully");

	wlf_backend_exe(backend);
	wlf_linked_list_remove(&render.expose.link);
	wlf_scene_node_destroy(&tree->base);
	destroy_shape_passes(&render);
	wlf_render_texture_pass_destroy(render.texture_pass);
	wlf_render_rect_pass_destroy(render.rect_pass);
	wlf_backend_destroy(backend);

	return EXIT_SUCCESS;
}
