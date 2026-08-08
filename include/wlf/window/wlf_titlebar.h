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
struct wlf_scene_node;
struct wlf_scene_tree;
struct wlf_svg_node;
struct wlf_text_node;
struct wlf_window;

enum {
	WLF_TITLEBAR_HEIGHT = 28,
};

/** One titlebar control, represented by a dedicated scene subtree. */
struct wlf_titlebar_button {
	struct wlf_scene_tree *tree;
	struct wlf_rect_node *background;
	struct wlf_svg_node *icon;
};

/** A wsm-inspired client-side titlebar scene subtree. */
struct wlf_titlebar {
	struct wlf_window *window;
	struct wlf_scene_tree *tree;
	struct wlf_rect_node *background;
	struct wlf_rect_node *separator;
	struct wlf_text_node *title_text;
	struct wlf_titlebar_button minimize_button;
	struct wlf_titlebar_button maximize_button;
	struct wlf_titlebar_button close_button;
	struct {
		struct wlf_listener resize;
		struct wlf_listener focus_in;
		struct wlf_listener focus_out;
	} listeners;
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

/** Applies focused or unfocused wsm-inspired titlebar colors. */
void wlf_titlebar_set_active(struct wlf_titlebar *titlebar, bool active);

#endif // WINDOW_WLF_TITLEBAR_H
