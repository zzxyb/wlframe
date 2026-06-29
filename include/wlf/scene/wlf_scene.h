/**
 * @file        wlf_scene.h
 * @brief       Window-local scene graph and damage management.
 * @details     A scene owns the render passes and scene-node tree for one
 *              window, tracks accumulated damage, and commits frames through
 *              the window's swapchain.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

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

/**
 * @brief Scene damage visualization mode, compatible with wlroots semantics.
 *
 * These modes are intended for debugging damage tracking and do not change
 * the scene graph itself.
 */
enum wlf_scene_debug_damage_option {
	WLF_SCENE_DEBUG_DAMAGE_NONE, /**< Render damage normally. */
	WLF_SCENE_DEBUG_DAMAGE_RERENDER, /**< Re-render the full window. */
	WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT, /**< Highlight the damaged region. */
};

/**
 * @brief A window-local scene graph with accumulated buffer damage.
 *
 * The scene owns its root tree and rendering passes, while the window owns
 * the scene object itself.
 */
struct wlf_scene {
	struct wlf_window *window; /**< Window rendered by this scene. */
	struct wlf_scene_tree *root; /**< Internal root containing content and decoration layers. */
	struct wlf_scene_tree *tree; /**< Application content tree. */
	struct wlf_titlebar *titlebar; /**< Client-side titlebar, or NULL when using SSD. */
	pixman_region32_t damage; /**< Damage accumulated for the next frame. */
	pixman_region32_t previous_damage; /**< Damage from the previous swapchain buffer, used to repair double buffering. */
	enum wlf_scene_debug_damage_option debug_damage_option; /**< Damage debug mode. */
	bool calculate_visibility; /**< Whether opaque nodes cull scene content below them. */
	bool highlight_transparent_region; /**< Whether translucent portions are highlighted for debugging. */
	struct wlf_linked_list damage_highlight_regions; /**< Temporary highlight regions. */
	struct wlf_array render_list; /**< Reused array of struct wlf_render_list_entry. */
	bool frame_scheduled; /**< True after requesting a frame and before the next expose callback. */

	struct wlf_rect_pass *rect_pass; /**< Solid rectangle pass. */
	struct wlf_texture_pass *texture_pass; /**< Texture pass. */
	struct wlf_rect_shape_pass *rect_shape_pass; /**< Rectangle-shape pass. */
	struct wlf_circle_pass *circle_pass; /**< Circle pass. */
	struct wlf_ellipse_pass *ellipse_pass; /**< Ellipse pass. */
	struct wlf_line_pass *line_pass; /**< Line pass. */
	struct wlf_poly_pass *poly_pass; /**< Polygon pass. */
	struct wlf_path_pass *path_pass; /**< Path pass. */

	struct wlf_listener window_expose;
	struct wlf_listener window_resize;
	struct {
		struct wlf_signal frame_done; /**< Payload is a const struct timespec pointer. */
		struct wlf_signal destroy; /**< Emitted before the scene is destroyed. */
	} events;
};

/**
 * @brief Creates and attaches a scene to an initialized window renderer.
 *
 * The scene creates the render passes required by the window's renderer.
 *
 * @param window Window that owns the scene.
 * @return New scene, or NULL on allocation or renderer failure.
 */
struct wlf_scene *wlf_scene_create(struct wlf_window *window);

/**
 * @brief Destroys the scene, its root tree, and its rendering passes.
 *
 * All scene nodes owned by the scene are destroyed before the render passes.
 *
 * @param scene Scene to destroy.
 */
void wlf_scene_destroy(struct wlf_scene *scene);

/**
 * @brief Adds window-local damage and schedules one frame when necessary.
 *
 * The region is accumulated until the next successful commit.
 *
 * @param scene Scene receiving the damage.
 * @param damage Logical region that changed.
 */
void wlf_scene_damage(struct wlf_scene *scene,
	const pixman_region32_t *damage);

/**
 * @brief Damages the complete current window.
 *
 * This is equivalent to adding the full logical window region as damage.
 *
 * @param scene Scene whose complete content should be redrawn.
 */
void wlf_scene_damage_whole(struct wlf_scene *scene);

/**
 * @brief Recalculates node visibility from top to bottom using opaque regions.
 *
 * Visibility is used to cull content hidden by opaque nodes.
 *
 * @param scene Scene whose node visibility is recalculated.
 */
void wlf_scene_recalculate_visibility(struct wlf_scene *scene);

/**
 * @brief Changes damage debugging and repaints the scene.
 *
 * Changing the option schedules a full repaint so that the new visualization
 * is visible immediately.
 *
 * @param scene Scene whose debug mode is changed.
 * @param option New damage visualization mode.
 */
void wlf_scene_set_debug_damage(struct wlf_scene *scene,
	enum wlf_scene_debug_damage_option option);

/**
 * @brief Returns whether a scene commit has work to do.
 *
 * A scene needs a frame when it has pending damage or a scheduled frame.
 *
 * @param scene Scene to inspect.
 * @return true when a frame is pending, false otherwise.
 */
bool wlf_scene_needs_frame(const struct wlf_scene *scene);

/**
 * @brief Renders accumulated damage and presents the window swapchain.
 *
 * On success, the committed damage is removed from the scene's pending region.
 *
 * @param scene Scene to commit.
 * @return true when the frame was rendered and submitted, false on failure.
 */
bool wlf_scene_commit(struct wlf_scene *scene);

/**
 * @brief Enables or disables the scene-owned client-side titlebar.
 *
 * The titlebar is created or removed as part of the scene tree update.
 *
 * @param scene Scene whose titlebar policy is changed.
 * @param enabled Whether client-side decorations should be enabled.
 * @return true when the requested policy is active, false on failure.
 */
bool wlf_scene_set_client_side_decorated(struct wlf_scene *scene,
	bool enabled);

/**
 * @brief Notifies scene consumers that the frame timestamp has been reached.
 *
 * The timestamp is emitted as the payload of the scene's frame-done signal.
 *
 * @param scene Scene sending the frame-done signal.
 * @param when Presentation timestamp payload.
 */
void wlf_scene_send_frame_done(struct wlf_scene *scene,
	const struct timespec *when);

#endif // SCENE_WLF_SCENE_H
