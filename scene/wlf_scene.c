#include "wlf/scene/wlf_scene.h"

#include "wlf/pass/wlf_circle_pass.h"
#include "wlf/pass/wlf_ellipse_pass.h"
#include "wlf/pass/wlf_line_pass.h"
#include "wlf/pass/wlf_path_pass.h"
#include "wlf/pass/wlf_poly_pass.h"
#include "wlf/pass/wlf_rect_shape_pass.h"
#include "wlf/pass/gles/rect_pass.h"
#include "wlf/pass/gles/texture_pass.h"
#include "wlf/pass/gles/vector_pass.h"
#include "wlf/pass/pixman/rect_pass.h"
#include "wlf/pass/pixman/texture_pass.h"
#include "wlf/pass/pixman/vector_pass.h"
#include "wlf/config.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/renderer/gles/renderer.h"
#include "wlf/renderer/pixman/renderer.h"
#endif
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_time.h"
#include "wlf/swapchain/wlf_swapchain.h"
#include "wlf/window/wlf_window.h"
#include "wlf/window/wlf_titlebar.h"

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
#if WLF_HAS_LINUX_PLATFORM
	if (wlf_renderer_is_gles(renderer)) {
		return wlf_gles_vector_pass_create();
	}
	if (wlf_renderer_is_pixman(renderer)) {
		return wlf_pixman_vector_pass_create();
	}
#endif
	return NULL;
}

static bool create_passes(struct wlf_scene *scene) {
	struct wlf_renderer *renderer = scene->window->state.renderer;
#if WLF_HAS_LINUX_PLATFORM
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
#endif
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
		struct wlf_scene *scene, const pixman_region32_t *damage) {
	struct wlf_window *window = scene->window;
	struct wlf_buffer_pass_options options = {
		.damage = damage,
	};
	return wlf_renderer_begin_buffer_pass(window->state.renderer,
		wlf_swapchain_get_back_buffer(window->state.swapchain), &options);
}

static void clear_node_visibility(struct wlf_scene_node *node) {
	pixman_region32_clear(&node->state.visible);
	struct wlf_linked_list *children = wlf_scene_node_get_children(node);
	if (children == NULL) {
		return;
	}

	struct wlf_scene_node *child;
	wlf_linked_list_for_each(child, children, link) {
		clear_node_visibility(child);
	}
}

static void calculate_node_visibility(struct wlf_scene_node *node,
		pixman_region32_t *remaining, bool calculate_visibility) {
	if (!node->state.enabled) {
		clear_node_visibility(node);
		return;
	}

	pixman_region32_clear(&node->state.visible);
	struct wlf_linked_list *children = wlf_scene_node_get_children(node);
	if (children != NULL) {
		struct wlf_scene_node *child;
		wlf_linked_list_for_each_reverse(child, children, link) {
			calculate_node_visibility(child, remaining,
				calculate_visibility);
			pixman_region32_union(&node->state.visible,
				&node->state.visible, &child->state.visible);
		}
		return;
	}

	int x, y;
	if (!wlf_scene_node_coords(node, &x, &y) ||
			wlf_scene_node_invisible(node)) {
		return;
	}

	pixman_region32_t bounds;
	pixman_region32_init(&bounds);
	wlf_scene_node_bounds(node, x, y, &bounds);
	pixman_region32_intersect(&node->state.visible, &bounds, remaining);

	if (calculate_visibility) {
		pixman_region32_t opaque;
		pixman_region32_init(&opaque);
		wlf_scene_node_opaque_region(node, x, y, &opaque);
		pixman_region32_intersect(&opaque, &opaque, &node->state.visible);
		pixman_region32_subtract(remaining, remaining, &opaque);
		pixman_region32_fini(&opaque);
	}
	pixman_region32_fini(&bounds);
}

void wlf_scene_recalculate_visibility(struct wlf_scene *scene) {
	if (scene == NULL) {
		return;
	}
	int width = scene->window->state.geometry.width;
	int height = scene->window->state.geometry.height;
	if (width <= 0 || height <= 0) {
		clear_node_visibility(&scene->root->base);
		return;
	}

	pixman_region32_t remaining;
	pixman_region32_init_rect(&remaining, 0, 0, width, height);
	calculate_node_visibility(&scene->root->base, &remaining,
		scene->calculate_visibility);
	pixman_region32_fini(&remaining);
}

static bool scene_build_render_list(struct wlf_scene *scene,
		int width, int height) {
	struct wlf_render_list_constructor_data list_con = {
		.box = {
			.width = width,
			.height = height,
		},
		.render_list = &scene->render_list,
		.calculate_visibility = scene->calculate_visibility,
		.highlight_transparent_region =
			scene->highlight_transparent_region,
	};

	scene->render_list.size = 0;
	wlf_scene_node_nodes_in_box(&scene->root->base, &list_con.box,
		wlf_scene_node_construct_render_list_iterator, &list_con);
	return !list_con.failed;
}

static void scene_render_background(struct wlf_scene *scene,
		const struct wlf_render_data *render_data,
		struct wlf_render_list_entry *entries, size_t entries_len,
		int width, int height) {
	pixman_region32_t background;
	pixman_region32_init(&background);
	pixman_region32_copy(&background,
		(pixman_region32_t *)&render_data->damage);

	if (scene->calculate_visibility) {
		for (size_t i = entries_len; i > 0; i--) {
			struct wlf_render_list_entry *entry = &entries[i - 1];
			pixman_region32_t opaque;
			pixman_region32_init(&opaque);
			wlf_scene_node_opaque_region(entry->node,
				entry->x, entry->y, &opaque);
			pixman_region32_intersect(&opaque, &opaque,
				&entry->node->state.visible);
			pixman_region32_subtract(&background, &background, &opaque);
			pixman_region32_fini(&opaque);
		}
	}

	wlf_render_pass_add_rect(scene->rect_pass, render_data->target,
		&(struct wlf_render_rect_options){
			.box = { .width = width, .height = height },
			.color = scene->window->state.background_color,
			.clip = &background,
			.blend_mode = WLF_RENDER_BLEND_MODE_NONE,
		});
	pixman_region32_fini(&background);
}

static void handle_window_expose(struct wlf_listener *listener, void *data) {
	(void)data;
	struct wlf_scene *scene =
		wlf_container_of(listener, scene, window_expose);
	scene->frame_scheduled = false;
	if (wlf_scene_needs_frame(scene) && !wlf_scene_commit(scene)) {
		if (!scene->frame_scheduled) {
			scene->frame_scheduled = true;
			wlf_window_schedule_frame(scene->window);
		}
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
	scene->root = wlf_root_scene_tree_create();
	if (scene->root == NULL) {
		free(scene);
		return NULL;
	}
	scene->root->base.window = window;
	scene->root->base.scene = scene;
	scene->tree = wlf_scene_tree_create(&scene->root->base);
	if (scene->tree == NULL) {
		wlf_scene_node_destroy(&scene->root->base);
		free(scene);
		return NULL;
	}
	window->tree = scene->tree;
	if (!create_passes(scene)) {
		wlf_scene_node_destroy(&scene->root->base);
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
	wlf_array_init(&scene->render_list);
	scene->calculate_visibility =
		!wlf_env_parse_bool("WLF_SCENE_DISABLE_VISIBILITY");
	scene->highlight_transparent_region =
		wlf_env_parse_bool("WLF_SCENE_HIGHLIGHT_TRANSPARENT_REGION");
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

	scene->window_expose.notify = handle_window_expose;
	scene->window_resize.notify = handle_window_resize;
	wlf_signal_add(&window->events.expose, &scene->window_expose);
	wlf_signal_add(&window->events.resize, &scene->window_resize);
	window->scene = scene;
	if (!window->state.server_side_decorated &&
			!(window->state.state & WLF_WINDOW_FULLSCREEN) &&
			!wlf_scene_set_client_side_decorated(scene, true)) {
		wlf_scene_destroy(scene);
		return NULL;
	}
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
	wlf_titlebar_destroy(scene->titlebar);
	scene->titlebar = NULL;
	wlf_scene_node_destroy(&scene->root->base);
	if (scene->window != NULL && scene->window->tree == scene->tree) {
		scene->window->tree = NULL;
	}
	destroy_passes(scene);
	clear_highlight_regions(scene);
	wlf_array_release(&scene->render_list);
	pixman_region32_fini(&scene->damage);
	pixman_region32_fini(&scene->previous_damage);
	free(scene);
}

bool wlf_scene_set_client_side_decorated(struct wlf_scene *scene,
		bool enabled) {
	if (enabled == (scene->titlebar != NULL)) {
		return true;
	}
	if (enabled) {
		scene->titlebar = wlf_titlebar_create(&scene->root->base,
			scene->window);
		if (scene->titlebar == NULL) {
			return false;
		}
		wlf_scene_node_set_position(&scene->tree->base,
			0, WLF_TITLEBAR_HEIGHT);
	} else {
		wlf_titlebar_destroy(scene->titlebar);
		scene->titlebar = NULL;
		wlf_scene_node_set_position(&scene->tree->base, 0, 0);
	}
	wlf_scene_damage_whole(scene);
	return true;
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

	pixman_region32_t clipped;
	pixman_region32_init(&clipped);
	pixman_region32_intersect_rect(&clipped, damage, 0, 0,
		width, height);
	pixman_region32_union(&scene->damage, &scene->damage, &clipped);
	bool has_damage = !pixman_region32_empty(&clipped);
	pixman_region32_fini(&clipped);

	if (has_damage && !scene->frame_scheduled) {
		scene->frame_scheduled = true;
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

struct scene_state {
	pixman_region32_t damage;
};

static bool scene_build_state(struct wlf_scene *scene,
		struct scene_state *state) {
	struct wlf_window *window = scene->window;
	int width = window->state.geometry.width;
	int height = window->state.geometry.height;
	if (width <= 0 || height <= 0) {
		return false;
	}
	int buffer_width = (int)wlf_window_scale_length(window, (uint32_t)width);
	int buffer_height = (int)wlf_window_scale_length(window, (uint32_t)height);
	if (window->state.swapchain->width != buffer_width ||
			window->state.swapchain->height != buffer_height) {
		if (!wlf_swapchain_resize(window->state.swapchain,
				buffer_width, buffer_height)) {
			return false;
		}
		pixman_region32_union_rect(&scene->damage, &scene->damage,
			0, 0, width, height);
		pixman_region32_clear(&scene->previous_damage);
		pixman_region32_union_rect(&scene->previous_damage,
			&scene->previous_damage, 0, 0, width, height);
	}

	pixman_region32_t render_damage;
	pixman_region32_init(&render_damage);
	pixman_region32_copy(&state->damage, &scene->damage);
	struct timespec highlight_now = {0};
	if (scene->debug_damage_option == WLF_SCENE_DEBUG_DAMAGE_RERENDER) {
		pixman_region32_clear(&state->damage);
		pixman_region32_union_rect(&state->damage, &state->damage,
			0, 0, width, height);
	} else if (scene->debug_damage_option == WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT) {
		prepare_highlight_damage(scene, &state->damage, &highlight_now);
	}
	pixman_region32_union(&render_damage, &state->damage,
		&scene->previous_damage);
#if WLF_HAS_LINUX_PLATFORM
	/* EGL does not expose a stable buffer identity here. Without EGL buffer-age
	 * tracking, repaint the whole back buffer while still presenting only the
	 * actual surface damage. */
	if (wlf_renderer_is_gles(window->state.renderer)) {
		pixman_region32_clear(&render_damage);
		pixman_region32_union_rect(&render_damage, &render_damage,
			0, 0, width, height);
	}
#endif
	if (!scene_build_render_list(scene, width, height)) {
		pixman_region32_fini(&render_damage);
		return false;
	}
	struct wlf_render_target_info *target = begin_render(scene, &render_damage);
	if (target == NULL) {
		pixman_region32_fini(&render_damage);
		return false;
	}

	struct wlf_render_data render_data = {
		.scene = scene,
		.target = target,
		.logical = {
			.width = width,
			.height = height,
		},
	};
	pixman_region32_init(&render_data.damage);
	pixman_region32_copy(&render_data.damage, &render_damage);
	struct wlf_render_list_entry *entries = scene->render_list.data;
	size_t entries_len = scene->render_list.size / sizeof(*entries);
	scene_render_background(scene, &render_data, entries, entries_len,
		width, height);
	for (size_t i = entries_len; i > 0; i--) {
		wlf_scene_node_render(&entries[i - 1], &render_data);
	}
	pixman_region32_fini(&render_data.damage);
	if (scene->debug_damage_option == WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT) {
		render_damage_highlights(scene, target, &highlight_now, width, height);
	}
	wlf_render_target_info_destroy(target);
	pixman_region32_fini(&render_damage);
	return true;
}

bool wlf_scene_commit(struct wlf_scene *scene) {
	if (scene == NULL || !wlf_scene_needs_frame(scene)) {
		return scene != NULL;
	}

	struct scene_state state;
	pixman_region32_init(&state.damage);
	if (!scene_build_state(scene, &state)) {
		pixman_region32_fini(&state.damage);
		return false;
	}

	pixman_region32_t buffer_damage;
	pixman_region32_init(&buffer_damage);
	struct wlf_render_target_info scale_info = {
		.scale = scene->window->state.scale,
	};
	wlf_render_target_info_scale_region(&scale_info,
		&state.damage, &buffer_damage);
	wlf_swapchain_present(scene->window->state.swapchain, &buffer_damage);
	pixman_region32_fini(&buffer_damage);
	pixman_region32_copy(&scene->previous_damage, &state.damage);
	pixman_region32_clear(&scene->damage);
	pixman_region32_fini(&state.damage);
	if (scene->debug_damage_option == WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT &&
			!wlf_linked_list_empty(&scene->damage_highlight_regions)) {
		if (!scene->frame_scheduled) {
			scene->frame_scheduled = true;
			wlf_window_schedule_frame(scene->window);
		}
	}
	return true;
}

void wlf_scene_send_frame_done(struct wlf_scene *scene,
		const struct timespec *when) {
	if (scene != NULL && when != NULL) {
		wlf_signal_emit_mutable(&scene->events.frame_done, (void *)when);
	}
}
