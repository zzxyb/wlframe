/**
 * @file        wlf_titlebar.h
 * @brief       Client-side window titlebar built from scene nodes.
 * @details     This file defines the titlebar controls, resize handles, and
 *              scene integration used for client-side window decorations.
 * @author      YaoBing Xiao
 * @date        2026-08-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-09, initial version\n
 */

#ifndef WINDOW_WLF_TITLEBAR_H
#define WINDOW_WLF_TITLEBAR_H

#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_rect_node;
struct wlf_event_node;
struct wlf_scene_node;
struct wlf_scene_tree;
struct wlf_svg_node;
struct wlf_text_node;
struct wlf_window;

/** Built-in CSD window controls. */
enum wlf_titlebar_button_type {
	WLF_TITLEBAR_BUTTON_MINIMIZE,
	WLF_TITLEBAR_BUTTON_MAXIMIZE,
	WLF_TITLEBAR_BUTTON_CLOSE,
};

enum {
	WLF_TITLEBAR_HEIGHT = 28,
};

/** One titlebar control, represented by a dedicated scene subtree. */
struct wlf_titlebar_button {
	struct wlf_titlebar *titlebar;
	enum wlf_titlebar_button_type type;
	struct wlf_scene_tree *tree;
	struct wlf_rect_node *background;
	struct wlf_svg_node *icon;
	struct wlf_event_node *event_node;
	struct {
		struct wlf_listener pointer_enter;
		struct wlf_listener pointer_leave;
		struct wlf_listener pointer_button;
	} listeners;
	bool visible;
	bool hovered;
	bool custom_icon;
};

/** A wsm-inspired client-side titlebar scene subtree. */
struct wlf_titlebar {
	struct wlf_window *window;
	struct wlf_scene_tree *tree;
	struct wlf_rect_node *background;
	struct wlf_rect_node *separator;
	/** Public container for application-supplied titlebar scene nodes. */
	struct wlf_scene_tree *content;
	struct wlf_event_node *move_event_node;
	struct wlf_svg_node *window_icon;
	struct wlf_text_node *title_text;
	struct wlf_titlebar_button minimize_button;
	struct wlf_titlebar_button maximize_button;
	struct wlf_titlebar_button close_button;
	struct {
		struct wlf_listener resize;
		struct wlf_listener focus_in;
		struct wlf_listener focus_out;
		struct wlf_listener theme_changed;
		struct wlf_listener pointer_button;
	} listeners;
	bool theme_listener_attached;
};

/**
 * Creates a titlebar under @p parent and attaches it to @p window events.
 * The returned object owns its complete scene subtree.
 */
struct wlf_titlebar *wlf_titlebar_create(struct wlf_scene_node *parent,
	struct wlf_window *window);

/** Destroys a titlebar, its listeners and its scene subtree. */
void wlf_titlebar_destroy(struct wlf_titlebar *titlebar);

/** Updates titlebar geometry to match the current window width. */
void wlf_titlebar_arrange(struct wlf_titlebar *titlebar);

/** Updates the displayed title. NULL is treated as an empty title. */
void wlf_titlebar_set_title(struct wlf_titlebar *titlebar,
	const char *title);

/**
 * Returns the titlebar content container. Nodes created below this tree are
 * owned by the titlebar and may be positioned freely by the caller.
 */
struct wlf_scene_tree *wlf_titlebar_get_content_tree(
	struct wlf_titlebar *titlebar);

/** Returns a built-in control so callers may customize its scene subtree. */
struct wlf_titlebar_button *wlf_titlebar_get_button(
	struct wlf_titlebar *titlebar, enum wlf_titlebar_button_type type);

/** Replaces the upper-left window icon from an SVG document. */
bool wlf_titlebar_set_icon(struct wlf_titlebar *titlebar,
	const char *svg_source);

/** Shows or hides one of the built-in window controls. */
void wlf_titlebar_set_button_visible(struct wlf_titlebar *titlebar,
	enum wlf_titlebar_button_type type, bool visible);

/** Replaces a built-in window control icon from an SVG document. */
bool wlf_titlebar_set_button_icon(struct wlf_titlebar *titlebar,
	enum wlf_titlebar_button_type type, const char *svg_source);

/** Applies focused or unfocused wsm-inspired titlebar colors. */
void wlf_titlebar_set_active(struct wlf_titlebar *titlebar, bool active);

#endif // WINDOW_WLF_TITLEBAR_H
