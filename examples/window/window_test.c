#include "wlf/platform/wlf_backend.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/scene/wlf_rect_node.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/scene/wlf_text_node.h"
#include "wlf/scene/wlf_texture_node.h"
#include "wlf/scene/wlf_rect_shape_node.h"
#include "wlf/scene/wlf_circle_node.h"
#include "wlf/scene/wlf_ellipse_node.h"
#include "wlf/scene/wlf_line_node.h"
#include "wlf/scene/wlf_poly_node.h"
#include "wlf/scene/wlf_path_node.h"
#include "wlf/image/wlf_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wayland/xdg_toplevel_window.h"
#include "wlf/window/wlf_window.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

struct render_state {
	struct wlf_rect_node *rect;
	struct wlf_text_node *text;
	struct wlf_texture_node *image;
	struct wlf_rect_shape_node *rounded_rect;
	struct wlf_circle_node *circle;
	struct wlf_ellipse_node *ellipse;
	struct wlf_line_node *line;
	struct wlf_poly_node *poly;
	struct wlf_path_node *path;
};

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
	wlf_window_set_title(window, "wlframe scene text, shapes and image test");
	wlf_window_set_background_color(window, &WLF_COLOR_DARK_GRAY);

	struct wlf_scene *scene = wlf_scene_create(window);
	if (scene == NULL) {
		wlf_log(WLF_ERROR, "Failed to create window scene");
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	struct wlf_scene_tree *tree = scene->tree;

	struct render_state render = {0};

	struct wlf_color rect_color = wlf_color_from_rgba8(64, 148, 255, 230);
	render.rect = wlf_rect_node_create(&tree->base, 25, 25, 300, 220,
		&rect_color);
	if (render.rect == NULL) {
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	struct wlf_color text_color = wlf_color_from_rgb8(245, 248, 255);
	render.text = wlf_text_node_create(&tree->base, 45, 55,
		"wlframe text node\n中文 fallback · ffi\nمرحبا بالعالم",
		"sans-serif", 28,
		&text_color);
	if (render.text == NULL) {
		wlf_log(WLF_ERROR, "Failed to create text scene node");
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	wlf_text_node_set_font_style(render.text, WLF_TEXT_FONT_SLANT_NORMAL,
		WLF_TEXT_FONT_WEIGHT_BOLD);
	wlf_text_node_set_max_width(render.text, 260);

	struct wlf_image *image = wlf_image_load(image_path);
	if (image == NULL) {
		wlf_log(WLF_ERROR, "Failed to load image: %s", image_path);
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
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
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	render.image = wlf_texture_node_create(&tree->base, texture,
		40.0 + (270.0 - image_width) / 2.0,
		270.0 + (190.0 - image_height) / 2.0,
		image_width, image_height);
	if (render.image == NULL) {
		wlf_texture_destroy(texture);
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
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
		wlf_window_destroy(window);
		wlf_renderer_destroy(renderer);
		wlf_backend_destroy(backend);
		return EXIT_FAILURE;
	}
	wlf_log(WLF_INFO, "Displaying image: %s", image_path);

	wlf_window_show(window);
	wlf_log(WLF_INFO, "Backend started successfully");

	wlf_backend_exe(backend);
	wlf_window_destroy(window);
	wlf_renderer_destroy(renderer);
	wlf_backend_destroy(backend);

	return EXIT_SUCCESS;
}
