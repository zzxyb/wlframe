#include "wlf/scene/wlf_scene.h"

#include "wlf/buffer/pixman/buffer.h"
#include "wlf/pass/gles/rect_pass.h"
#include "wlf/pass/gles/render_target_info.h"
#include "wlf/pass/gles/texture_pass.h"
#include "wlf/pass/gles/vector_pass.h"
#include "wlf/pass/pixman/rect_pass.h"
#include "wlf/pass/pixman/render_target_info.h"
#include "wlf/pass/pixman/texture_pass.h"
#include "wlf/pass/pixman/vector_pass.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/renderer/pixman/renderer.h"
#include "wlf/scene/wlf_circle_node.h"
#include "wlf/scene/wlf_ellipse_node.h"
#include "wlf/scene/wlf_line_node.h"
#include "wlf/scene/wlf_path_node.h"
#include "wlf/scene/wlf_poly_node.h"
#include "wlf/scene/wlf_rect_node.h"
#include "wlf/scene/wlf_rect_shape_node.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/scene/wlf_texture_node.h"
#include "wlf/swapchain/egl/swapchain.h"
#include "wlf/swapchain/shm/swapchain.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_time.h"
#include "wlf/window/wlf_window.h"

#include <stdlib.h>

#define HIGHLIGHT_DAMAGE_FADEOUT_TIME 250

struct highlight_region {
	pixman_region32_t region;
	struct timespec when;
	struct wlf_linked_list link;
};

static void highlight_region_destroy(struct highlight_region *highlight) {
	wlf_linked_list_remove(&highlight->link);
	pixman_region32_fini(&highlight->region);
	free(highlight);
}

static void clear_highlight_regions(struct wlf_scene *scene) {
	struct highlight_region *highlight, *tmp;
	wlf_linked_list_for_each_safe(highlight, tmp,
			&scene->damage_highlight_regions, link) {
		highlight_region_destroy(highlight);
	}
}

static void prepare_highlight_damage(struct wlf_scene *scene,
		pixman_region32_t *damage, struct timespec *now) {
	wlf_get_monotonic_time(now);

	/* Only application/scene damage becomes a new highlight. Repaints used to
	 * fade existing highlights never enter this branch. */
	if (!pixman_region32_empty(damage)) {
		struct highlight_region *highlight = calloc(1, sizeof(*highlight));
		if (highlight != NULL) {
			pixman_region32_init(&highlight->region);
			pixman_region32_copy(&highlight->region, damage);
			highlight->when = *now;
			wlf_linked_list_insert(&scene->damage_highlight_regions,
				&highlight->link);
		}
	}

	pixman_region32_t accumulated;
	pixman_region32_init(&accumulated);
	struct highlight_region *highlight, *tmp;
	wlf_linked_list_for_each_safe(highlight, tmp,
			&scene->damage_highlight_regions, link) {
		/* Newer highlights win in overlapping areas. */
		pixman_region32_subtract(&highlight->region, &highlight->region,
			&accumulated);
		pixman_region32_union(&accumulated, &accumulated,
			&highlight->region);

		struct timespec elapsed;
		timespec_sub(&elapsed, now, &highlight->when);
		if (timespec_to_msec(&elapsed) >= HIGHLIGHT_DAMAGE_FADEOUT_TIME ||
				pixman_region32_empty(&highlight->region)) {
			highlight_region_destroy(highlight);
		}
	}

	/* This also repaints the last pixels of highlights removed above, restoring
	 * the scene underneath them. */
	pixman_region32_union(damage, damage, &accumulated);
	pixman_region32_fini(&accumulated);
}

static void render_damage_highlights(struct wlf_scene *scene,
		struct wlf_render_target_info *target, const struct timespec *now,
		int width, int height) {
	struct highlight_region *highlight;
	wlf_linked_list_for_each(highlight,
			&scene->damage_highlight_regions, link) {
		struct timespec elapsed;
		timespec_sub(&elapsed, now, &highlight->when);
		double alpha = 1.0 - (double)timespec_to_msec(&elapsed) /
			HIGHLIGHT_DAMAGE_FADEOUT_TIME;
		if (alpha < 0.0) {
			alpha = 0.0;
		} else if (alpha > 1.0) {
			alpha = 1.0;
		}

		struct wlf_render_rect_options options = {
			.box = { .width = width, .height = height },
			.color = { .r = 1.0, .a = alpha * 0.5 },
			.clip = &highlight->region,
			.blend_mode = WLF_RENDER_BLEND_MODE_PREMULTIPLIED,
		};
		wlf_render_pass_add_rect(scene->rect_pass, target, &options);
	}
}

static struct wlf_vector_pass *create_vector_pass(
		struct wlf_renderer *renderer) {
	if (wlf_renderer_is_gles(renderer)) {
		return wlf_gles_vector_pass_create();
	}
	if (wlf_renderer_is_pixman(renderer)) {
		return wlf_pixman_vector_pass_create();
	}
	return NULL;
}

static bool create_passes(struct wlf_scene *scene) {
	struct wlf_renderer *renderer = scene->window->state.renderer;
	if (wlf_renderer_is_gles(renderer)) {
		scene->rect_pass = wlf_gles_rect_pass_create();
		scene->texture_pass = wlf_gles_texture_pass_create();
	} else if (wlf_renderer_is_pixman(renderer)) {
		scene->rect_pass = wlf_pixman_rect_pass_create();
		scene->texture_pass = wlf_pixman_texture_pass_create();
	} else {
		wlf_log(WLF_ERROR, "Scene rendering is unsupported by this renderer");
		return false;
	}

	scene->rect_shape_pass = wlf_rect_shape_pass_create(
		create_vector_pass(renderer));
	scene->circle_pass = wlf_circle_pass_create(create_vector_pass(renderer));
	scene->ellipse_pass = wlf_ellipse_pass_create(create_vector_pass(renderer));
	scene->line_pass = wlf_line_pass_create(create_vector_pass(renderer));
	scene->poly_pass = wlf_poly_pass_create(create_vector_pass(renderer));
	scene->path_pass = wlf_path_pass_create(create_vector_pass(renderer));

	return scene->rect_pass != NULL && scene->texture_pass != NULL &&
		scene->rect_shape_pass != NULL && scene->circle_pass != NULL &&
		scene->ellipse_pass != NULL && scene->line_pass != NULL &&
		scene->poly_pass != NULL && scene->path_pass != NULL;
}

static void destroy_passes(struct wlf_scene *scene) {
	wlf_render_path_pass_destroy(scene->path_pass);
	wlf_render_poly_pass_destroy(scene->poly_pass);
	wlf_render_line_pass_destroy(scene->line_pass);
	wlf_render_ellipse_pass_destroy(scene->ellipse_pass);
	wlf_render_circle_pass_destroy(scene->circle_pass);
	wlf_render_rect_shape_pass_destroy(scene->rect_shape_pass);
	wlf_render_texture_pass_destroy(scene->texture_pass);
	wlf_render_rect_pass_destroy(scene->rect_pass);
}

static struct wlf_render_target_info *begin_render(
		struct wlf_scene *scene) {
	struct wlf_window *window = scene->window;
	if (wlf_renderer_is_gles(window->state.renderer) &&
			wlf_swapchain_is_egl(window->state.swapchain)) {
		struct wlf_egl_swapchain *swapchain =
			wlf_egl_swapchain_from_swapchain(window->state.swapchain);
		struct wlf_gles_render_target_info *target =
			wlf_gles_begin_egl_render_pass(swapchain);
		return target != NULL ? &target->base : NULL;
	}

	if (wlf_renderer_is_pixman(window->state.renderer) &&
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
		if (buffer == NULL) {
			return NULL;
		}
		struct wlf_pixman_render_target_info *target =
			wlf_pixman_begin_pixman_render_pass(buffer);
		return target != NULL ? &target->base : NULL;
	}

	wlf_log(WLF_ERROR, "Renderer and swapchain are incompatible");
	return NULL;
}

static void clear_node_visibility(struct wlf_scene_node *node) {
	pixman_region32_clear(&node->state.visible);
	if (!wlf_scene_node_is_tree(node)) {
		return;
	}

	struct wlf_scene_tree *tree = wlf_scene_tree_from_node(node);
	struct wlf_scene_node *child;
	wlf_linked_list_for_each(child, &tree->children, link) {
		clear_node_visibility(child);
	}
}

static void calculate_node_visibility(struct wlf_scene_node *node,
		pixman_region32_t *remaining) {
	if (!node->state.enabled) {
		clear_node_visibility(node);
		return;
	}

	pixman_region32_clear(&node->state.visible);
	if (wlf_scene_node_is_tree(node)) {
		struct wlf_scene_tree *tree = wlf_scene_tree_from_node(node);
		struct wlf_scene_node *child;
		wlf_linked_list_for_each_reverse(child, &tree->children, link) {
			calculate_node_visibility(child, remaining);
			pixman_region32_union(&node->state.visible,
				&node->state.visible, &child->state.visible);
		}
		return;
	}

	double x, y;
	if (!wlf_scene_node_coords(node, &x, &y) ||
			wlf_scene_node_invisible(node)) {
		return;
	}

	pixman_region32_t bounds;
	pixman_region32_init(&bounds);
	wlf_scene_node_bounds(node, x, y, &bounds);
	pixman_region32_intersect(&node->state.visible, &bounds, remaining);

	pixman_region32_t opaque;
	pixman_region32_init(&opaque);
	wlf_scene_node_get_opaque_region(node, x, y, &opaque);
	pixman_region32_intersect(&opaque, &opaque, &node->state.visible);
	pixman_region32_subtract(remaining, remaining, &opaque);
	pixman_region32_fini(&opaque);
	pixman_region32_fini(&bounds);
}

void wlf_scene_recalculate_visibility(struct wlf_scene *scene) {
	if (scene == NULL) {
		return;
	}
	int width = scene->window->state.geometry.width;
	int height = scene->window->state.geometry.height;
	if (width <= 0 || height <= 0) {
		clear_node_visibility(&scene->tree->base);
		return;
	}

	pixman_region32_t remaining;
	pixman_region32_init_rect(&remaining, 0, 0, width, height);
	calculate_node_visibility(&scene->tree->base, &remaining);
	pixman_region32_fini(&remaining);
}

static void render_node(struct wlf_scene *scene, struct wlf_scene_node *node,
		struct wlf_render_target_info *target,
		const pixman_region32_t *damage) {
	if (!node->state.enabled) {
		return;
	}

	if (wlf_scene_node_is_tree(node)) {
		struct wlf_scene_tree *tree = wlf_scene_tree_from_node(node);
		struct wlf_scene_node *child;
		wlf_linked_list_for_each(child, &tree->children, link) {
			render_node(scene, child, target, damage);
		}
		return;
	}

	pixman_region32_t clip;
	pixman_region32_init(&clip);
	pixman_region32_intersect(&clip, damage, &node->state.visible);
	if (pixman_region32_empty(&clip)) {
		pixman_region32_fini(&clip);
		return;
	}

	if (wlf_scene_node_is_rect(node)) {
		wlf_rect_node_render(wlf_rect_node_from_node(node), scene->rect_pass,
			target, &clip);
	} else if (wlf_scene_node_is_texture(node)) {
		wlf_texture_node_render(wlf_texture_node_from_node(node),
			scene->texture_pass, target, &clip);
	} else if (wlf_scene_node_is_rect_shape(node)) {
		wlf_rect_shape_node_render(wlf_rect_shape_node_from_node(node),
			scene->rect_shape_pass, target, &clip);
	} else if (wlf_scene_node_is_circle(node)) {
		wlf_circle_node_render(wlf_circle_node_from_node(node),
			scene->circle_pass, target, &clip);
	} else if (wlf_scene_node_is_ellipse(node)) {
		wlf_ellipse_node_render(wlf_ellipse_node_from_node(node),
			scene->ellipse_pass, target, &clip);
	} else if (wlf_scene_node_is_line(node)) {
		wlf_line_node_render(wlf_line_node_from_node(node), scene->line_pass,
			target, &clip);
	} else if (wlf_scene_node_is_poly(node)) {
		wlf_poly_node_render(wlf_poly_node_from_node(node), scene->poly_pass,
			target, &clip);
	} else if (wlf_scene_node_is_path(node)) {
		wlf_path_node_render(wlf_path_node_from_node(node), scene->path_pass,
			target, &clip);
	}
	pixman_region32_fini(&clip);
}

static void handle_window_expose(struct wlf_listener *listener, void *data) {
	(void)data;
	struct wlf_scene *scene =
		wlf_container_of(listener, scene, window_expose);
	if (wlf_scene_needs_frame(scene) && !wlf_scene_commit(scene)) {
		return;
	}

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlf_scene_send_frame_done(scene, &now);
}

static void handle_window_resize(struct wlf_listener *listener, void *data) {
	(void)data;
	struct wlf_scene *scene =
		wlf_container_of(listener, scene, window_resize);
	wlf_scene_damage_whole(scene);
}

struct wlf_scene *wlf_scene_create(struct wlf_window *window) {
	if (window == NULL || window->scene != NULL || window->tree != NULL ||
			window->state.renderer == NULL || window->state.swapchain == NULL) {
		return NULL;
	}

	struct wlf_scene *scene = calloc(1, sizeof(*scene));
	if (scene == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_scene");
		return NULL;
	}
	scene->window = window;
	scene->tree = wlf_root_scene_tree_create();
	if (scene->tree == NULL) {
		free(scene);
		return NULL;
	}
	scene->tree->base.window = window;
	window->tree = scene->tree;
	if (!create_passes(scene)) {
		wlf_scene_node_destroy(&scene->tree->base);
		window->tree = NULL;
		destroy_passes(scene);
		free(scene);
		return NULL;
	}

	pixman_region32_init_rect(&scene->damage, 0, 0,
		window->state.geometry.width, window->state.geometry.height);
	pixman_region32_init_rect(&scene->previous_damage, 0, 0,
		window->state.geometry.width, window->state.geometry.height);
	wlf_linked_list_init(&scene->damage_highlight_regions);
	const char *debug_damage_options[] = {
		"none",
		"rerender",
		"highlight",
		NULL,
	};
	scene->debug_damage_option = (enum wlf_scene_debug_damage_option)
		wlf_env_parse_switch("WLF_SCENE_DEBUG_DAMAGE", debug_damage_options);
	wlf_signal_init(&scene->events.frame_done);
	wlf_signal_init(&scene->events.destroy);

	window->scene = scene;

	scene->window_expose.notify = handle_window_expose;
	scene->window_resize.notify = handle_window_resize;
	wlf_signal_add(&window->events.expose, &scene->window_expose);
	wlf_signal_add(&window->events.resize, &scene->window_resize);
	return scene;
}

void wlf_scene_destroy(struct wlf_scene *scene) {
	if (scene == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&scene->events.destroy, scene);
	wlf_linked_list_remove(&scene->window_expose.link);
	wlf_linked_list_remove(&scene->window_resize.link);
	if (scene->window != NULL && scene->window->scene == scene) {
		scene->window->scene = NULL;
	}
	wlf_scene_node_destroy(&scene->tree->base);
	if (scene->window != NULL && scene->window->tree == scene->tree) {
		scene->window->tree = NULL;
	}
	destroy_passes(scene);
	clear_highlight_regions(scene);
	pixman_region32_fini(&scene->damage);
	pixman_region32_fini(&scene->previous_damage);
	free(scene);
}

void wlf_scene_damage(struct wlf_scene *scene,
		const pixman_region32_t *damage) {
	if (scene == NULL || damage == NULL || pixman_region32_empty(damage)) {
		return;
	}
	int width = scene->window->state.geometry.width;
	int height = scene->window->state.geometry.height;
	if (width <= 0 || height <= 0) {
		return;
	}

	bool schedule = pixman_region32_empty(&scene->damage);
	pixman_region32_t clipped;
	pixman_region32_init(&clipped);
	pixman_region32_intersect_rect(&clipped, damage, 0, 0,
		width, height);
	pixman_region32_union(&scene->damage, &scene->damage, &clipped);
	bool has_damage = !pixman_region32_empty(&clipped);
	pixman_region32_fini(&clipped);

	if (schedule && has_damage) {
		wlf_window_schedule_frame(scene->window);
	}
}

void wlf_scene_damage_whole(struct wlf_scene *scene) {
	if (scene == NULL) {
		return;
	}
	int width = scene->window->state.geometry.width;
	int height = scene->window->state.geometry.height;
	if (width <= 0 || height <= 0) {
		return;
	}
	pixman_region32_t damage;
	pixman_region32_init_rect(&damage, 0, 0,
		width, height);
	wlf_scene_damage(scene, &damage);
	pixman_region32_fini(&damage);
}

void wlf_scene_set_debug_damage(struct wlf_scene *scene,
		enum wlf_scene_debug_damage_option option) {
	if (scene == NULL || option < WLF_SCENE_DEBUG_DAMAGE_NONE ||
			option > WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT ||
			scene->debug_damage_option == option) {
		return;
	}

	clear_highlight_regions(scene);
	scene->debug_damage_option = option;
	wlf_scene_damage_whole(scene);
}

bool wlf_scene_needs_frame(const struct wlf_scene *scene) {
	return scene != NULL && (!pixman_region32_empty(
		(pixman_region32_t *)&scene->damage) ||
		(scene->debug_damage_option == WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT &&
		!wlf_linked_list_empty(&scene->damage_highlight_regions)));
}

bool wlf_scene_commit(struct wlf_scene *scene) {
	if (scene == NULL || !wlf_scene_needs_frame(scene)) {
		return scene != NULL;
	}

	struct wlf_window *window = scene->window;
	int width = window->state.geometry.width;
	int height = window->state.geometry.height;
	if (width <= 0 || height <= 0) {
		return false;
	}
	if (window->state.swapchain->width != width ||
			window->state.swapchain->height != height) {
		if (!wlf_swapchain_resize(window->state.swapchain, width, height)) {
			return false;
		}
		pixman_region32_union_rect(&scene->damage, &scene->damage,
			0, 0, width, height);
		pixman_region32_clear(&scene->previous_damage);
		pixman_region32_union_rect(&scene->previous_damage,
			&scene->previous_damage, 0, 0, width, height);
	}

	pixman_region32_t damage;
	pixman_region32_t render_damage;
	pixman_region32_init(&damage);
	pixman_region32_init(&render_damage);
	pixman_region32_copy(&damage, &scene->damage);
	struct timespec highlight_now = {0};
	if (scene->debug_damage_option == WLF_SCENE_DEBUG_DAMAGE_RERENDER) {
		pixman_region32_clear(&damage);
		pixman_region32_union_rect(&damage, &damage, 0, 0, width, height);
	} else if (scene->debug_damage_option == WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT) {
		prepare_highlight_damage(scene, &damage, &highlight_now);
	}
	pixman_region32_union(&render_damage, &damage, &scene->previous_damage);
	/* EGL does not expose a stable buffer identity here. Without EGL buffer-age
	 * tracking, repaint the whole back buffer while still presenting only the
	 * actual surface damage. */
	if (wlf_renderer_is_gles(window->state.renderer)) {
		pixman_region32_clear(&render_damage);
		pixman_region32_union_rect(&render_damage, &render_damage,
			0, 0, width, height);
	}
	struct wlf_render_target_info *target = begin_render(scene);
	if (target == NULL) {
		pixman_region32_fini(&render_damage);
		pixman_region32_fini(&damage);
		return false;
	}

	struct wlf_render_rect_options background = {
		.box = { .width = width, .height = height },
		.color = window->state.background_color,
		.clip = &render_damage,
		.blend_mode = WLF_RENDER_BLEND_MODE_NONE,
	};
	wlf_render_pass_add_rect(scene->rect_pass, target, &background);
	render_node(scene, &scene->tree->base, target, &render_damage);
	if (scene->debug_damage_option == WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT) {
		render_damage_highlights(scene, target, &highlight_now, width, height);
	}
	wlf_render_target_info_destroy(target);
	wlf_swapchain_present(window->state.swapchain, &damage);
	pixman_region32_copy(&scene->previous_damage, &damage);
	pixman_region32_clear(&scene->damage);
	pixman_region32_fini(&render_damage);
	pixman_region32_fini(&damage);
	if (scene->debug_damage_option == WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT &&
			!wlf_linked_list_empty(&scene->damage_highlight_regions)) {
		wlf_window_schedule_frame(window);
	}
	return true;
}

void wlf_scene_send_frame_done(struct wlf_scene *scene,
		const struct timespec *when) {
	if (scene != NULL && when != NULL) {
		wlf_signal_emit_mutable(&scene->events.frame_done, (void *)when);
	}
}
