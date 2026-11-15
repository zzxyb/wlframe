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
#include <stdint.h>

struct wlf_rect_node;
struct wlf_event_node;
struct wlf_scene_node;
struct wlf_scene_tree;
struct wlf_svg_node;
struct wlf_text_node;
struct wlf_window;

/**
 * @brief Built-in CSD window controls.
 */
enum wlf_titlebar_button_type {
	WLF_TITLEBAR_BUTTON_MINIMIZE, /**< Window control: minimize. */
	WLF_TITLEBAR_BUTTON_MAXIMIZE, /**< Window control: maximize. */
	WLF_TITLEBAR_BUTTON_CLOSE, /**< Window control: close. */
};

/**
 * @brief Default titlebar dimensions and number of resize input targets.
 */
enum {
	WLF_TITLEBAR_HEIGHT = 28, /**< Titlebar height in logical pixels. */
	WLF_TITLEBAR_RESIZE_HANDLE_COUNT = 8, /**< Number of window edge and corner input targets. */
};

/**
 * @brief Input target and listener for one window resize edge or corner.
 */
struct wlf_titlebar_resize_handle {
	struct wlf_titlebar *titlebar; /**< Titlebar owning this control. */
	uint32_t edge; /**< Window resize edge handled by this input target. */
	struct wlf_event_node *event_node; /**< Scene node receiving control input. */
	struct wlf_listener pointer_button; /**< Listener for pointer button events. */
};

/**
 * @brief One titlebar control, represented by a dedicated scene subtree.
 */
struct wlf_titlebar_button {
	struct wlf_titlebar *titlebar; /**< Titlebar owning this control. */
	enum wlf_titlebar_button_type type; /**< Built-in window control represented by this button. */
	struct wlf_scene_tree *tree; /**< Root of the owned scene subtree. */
	struct wlf_rect_node *background; /**< Background rectangle scene node. */
	struct wlf_svg_node *icon; /**< SVG icon scene node. */
	struct wlf_event_node *event_node; /**< Scene node receiving control input. */
	struct {
		struct wlf_listener pointer_enter; /**< Listener for pointer enter events. */
		struct wlf_listener pointer_leave; /**< Listener for pointer leave events. */
		struct wlf_listener pointer_button; /**< Listener for pointer button events. */
		struct wlf_listener touch_down; /**< Listener for touch down events. */
		struct wlf_listener touch_up; /**< Listener for touch up events. */
		struct wlf_listener touch_cancel; /**< Listener for touch cancel events. */
	} listeners; /**< Listeners attached to related objects. */
	bool visible; /**< Whether the control participates in layout and input. */
	bool hovered; /**< Whether the pointer is over the control. */
	bool pressed; /**< Whether the pointer button is pressed on the control. */
	bool touch_pressed; /**< Whether a touch press is active on the control. */
	int32_t touch_id; /**< Identifier of the touch point pressing the control. */
	bool custom_icon; /**< Whether the application supplied the control icon. */
};

/**
 * @brief A wsm-inspired client-side titlebar scene subtree.
 */
struct wlf_titlebar {
	struct wlf_window *window; /**< Window associated with this object. */
	struct wlf_scene_tree *tree; /**< Root of the owned scene subtree. */
	struct wlf_rect_node *background; /**< Background rectangle scene node. */
	struct wlf_rect_node *separator; /**< Separator rectangle below the titlebar. */
	/** Public container for application-supplied titlebar scene nodes. */
	struct wlf_scene_tree *content;
	struct wlf_event_node *move_event_node; /**< Input target for dragging the window. */
	struct wlf_svg_node *window_icon; /**< Window icon scene node. */
	struct wlf_text_node *title_text; /**< Window title text node. */
	struct wlf_titlebar_button minimize_button; /**< Built-in minimize control. */
	struct wlf_titlebar_button maximize_button; /**< Built-in maximize control. */
	struct wlf_titlebar_button close_button; /**< Built-in close control. */
	/** Input targets for window edges and corners. */
	struct wlf_titlebar_resize_handle resize_handles[
		WLF_TITLEBAR_RESIZE_HANDLE_COUNT];
	struct {
		struct wlf_listener resize; /**< Listener for resize events. */
		struct wlf_listener focus_in; /**< Listener for focus in events. */
		struct wlf_listener focus_out; /**< Listener for focus out events. */
		struct wlf_listener theme_changed; /**< Listener for theme changed events. */
		struct wlf_listener pointer_button; /**< Listener for pointer button events. */
	} listeners; /**< Listeners attached to related objects. */
	bool theme_listener_attached; /**< Whether the host theme change listener is attached. */
};

/**
 * @brief Creates a titlebar under @p parent and attaches it to @p window events.
 * The returned object owns its complete scene subtree.
 *
 * @param parent Parent scene node.
 * @param window Window to inspect or associate with the object.
 * @return New titlebar owning its scene subtree, or NULL on failure.
 */
struct wlf_titlebar *wlf_titlebar_create(struct wlf_scene_node *parent,
	struct wlf_window *window);

/**
 * @brief Destroys a titlebar, its listeners and its scene subtree.
 *
 * @param titlebar Titlebar to inspect or modify.
 */
void wlf_titlebar_destroy(struct wlf_titlebar *titlebar);

/**
 * @brief Updates titlebar geometry to match the current window width.
 *
 * @param titlebar Titlebar to inspect or modify.
 */
void wlf_titlebar_arrange(struct wlf_titlebar *titlebar);

/**
 * @brief Updates the displayed title. NULL is treated as an empty title.
 *
 * @param titlebar Titlebar to inspect or modify.
 * @param title New title text, or NULL for an empty title.
 */
void wlf_titlebar_set_title(struct wlf_titlebar *titlebar,
	const char *title);

/**
 * @brief Returns the titlebar content container. Nodes created below this tree are
 * owned by the titlebar and may be positioned freely by the caller.
 *
 * @param titlebar Titlebar to inspect or modify.
 * @return Borrowed content container, or NULL for a NULL titlebar.
 */
struct wlf_scene_tree *wlf_titlebar_get_content_tree(
	struct wlf_titlebar *titlebar);

/**
 * @brief Returns a built-in control so callers may customize its scene subtree.
 *
 * @param titlebar Titlebar to inspect or modify.
 * @param type Built-in window control to select.
 * @return Borrowed control, or NULL for a NULL titlebar or invalid control type.
 */
struct wlf_titlebar_button *wlf_titlebar_get_button(
	struct wlf_titlebar *titlebar, enum wlf_titlebar_button_type type);

/**
 * @brief Replaces the upper-left window icon from an SVG document.
 *
 * @param titlebar Titlebar to inspect or modify.
 * @param svg_source SVG document describing the new icon.
 * @return true if the icon was replaced, false if SVG parsing or node creation fails.
 */
bool wlf_titlebar_set_icon(struct wlf_titlebar *titlebar,
	const char *svg_source);

/**
 * @brief Shows or hides one of the built-in window controls.
 *
 * @param titlebar Titlebar to inspect or modify.
 * @param type Built-in window control to select.
 * @param visible Whether the control is visible.
 */
void wlf_titlebar_set_button_visible(struct wlf_titlebar *titlebar,
	enum wlf_titlebar_button_type type, bool visible);

/**
 * @brief Replaces a built-in window control icon from an SVG document.
 *
 * @param titlebar Titlebar to inspect or modify.
 * @param type Built-in window control to select.
 * @param svg_source SVG document describing the new icon.
 * @return true if the icon was replaced, false for an invalid control or icon creation failure.
 */
bool wlf_titlebar_set_button_icon(struct wlf_titlebar *titlebar,
	enum wlf_titlebar_button_type type, const char *svg_source);

/**
 * @brief Applies focused or unfocused wsm-inspired titlebar colors.
 *
 * @param titlebar Titlebar to inspect or modify.
 * @param active Whether the window has keyboard focus.
 */
void wlf_titlebar_set_active(struct wlf_titlebar *titlebar, bool active);

#endif // WINDOW_WLF_TITLEBAR_H
