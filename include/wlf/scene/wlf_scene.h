#ifndef SCENE_WLF_SCENE_H
#define SCENE_WLF_SCENE_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_array.h"

#include <stdbool.h>
#include <pixman.h>
#include <time.h>

struct wlf_window;
struct wlf_scene_tree;
struct wlf_rect_pass;
struct wlf_texture_pass;
struct wlf_rect_shape_pass;
struct wlf_circle_pass;
struct wlf_ellipse_pass;
struct wlf_line_pass;
struct wlf_poly_pass;
struct wlf_path_pass;
struct wlf_titlebar;

/** Scene damage visualization mode, compatible with wlroots semantics. */
enum wlf_scene_debug_damage_option {
	WLF_SCENE_DEBUG_DAMAGE_NONE,
	WLF_SCENE_DEBUG_DAMAGE_RERENDER,
	WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT,
};

/** A window-local scene graph with accumulated buffer damage. */
struct wlf_scene {
	struct wlf_window *window;
	struct wlf_scene_tree *root; /**< Internal root containing content and decoration layers. */
	struct wlf_scene_tree *tree; /**< Application content tree. */
	struct wlf_titlebar *titlebar; /**< Client-side titlebar, or NULL when using SSD. */
	pixman_region32_t damage;
	/** Damage from the previous swapchain buffer, used to repair double buffering. */
	pixman_region32_t previous_damage;
	enum wlf_scene_debug_damage_option debug_damage_option;
	/** Whether opaque nodes cull scene content below them. */
	bool calculate_visibility;
	/** Whether translucent portions are highlighted for debugging. */
	bool highlight_transparent_region;
	struct wlf_linked_list damage_highlight_regions;
	/** Reused array of struct wlf_render_list_entry. */
	struct wlf_array render_list;

	struct wlf_rect_pass *rect_pass;
	struct wlf_texture_pass *texture_pass;
	struct wlf_rect_shape_pass *rect_shape_pass;
	struct wlf_circle_pass *circle_pass;
	struct wlf_ellipse_pass *ellipse_pass;
	struct wlf_line_pass *line_pass;
	struct wlf_poly_pass *poly_pass;
	struct wlf_path_pass *path_pass;

	struct wlf_listener window_expose;
	struct wlf_listener window_resize;
	struct {
		/** Payload is a const struct timespec pointer. */
		struct wlf_signal frame_done;
		struct wlf_signal destroy;
	} events;
};

/** Creates and attaches a scene to an initialized window renderer. */
struct wlf_scene *wlf_scene_create(struct wlf_window *window);

/** Destroys the scene, its root tree, and its rendering passes. */
void wlf_scene_destroy(struct wlf_scene *scene);

/** Adds window-local damage and schedules one frame when necessary. */
void wlf_scene_damage(struct wlf_scene *scene,
	const pixman_region32_t *damage);

/** Damages the complete current window. */
void wlf_scene_damage_whole(struct wlf_scene *scene);

/** Recalculates node visibility from top to bottom using opaque regions. */
void wlf_scene_recalculate_visibility(struct wlf_scene *scene);

/** Changes damage debugging and repaints the scene. */
void wlf_scene_set_debug_damage(struct wlf_scene *scene,
	enum wlf_scene_debug_damage_option option);

/** Returns whether a scene commit has work to do. */
bool wlf_scene_needs_frame(const struct wlf_scene *scene);

/** Renders accumulated damage and presents the window swapchain. */
bool wlf_scene_commit(struct wlf_scene *scene);

/** Enables or disables the scene-owned client-side titlebar. */
bool wlf_scene_set_client_side_decorated(struct wlf_scene *scene,
	bool enabled);

/** Notifies scene consumers that the frame timestamp has been reached. */
void wlf_scene_send_frame_done(struct wlf_scene *scene,
	const struct timespec *when);

#endif // SCENE_WLF_SCENE_H
