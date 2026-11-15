/**
 * @file        wlf_event_node.h
 * @brief       Non-rendering scene node used as an input event target.
 * @details     This file defines scene-local input targets and their signals
 *              for pointer, keyboard, tablet, and touch event dispatch.
 * @author      YaoBing Xiao
 * @date        2026-08-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-09, initial version\n
 */

#ifndef SCENE_WLF_EVENT_NODE_H
#define SCENE_WLF_EVENT_NODE_H

#include "wlf/node/wlf_scene_node.h"
#include "wlf/types/wlf_cursor.h"

struct wlf_pointer;
struct wlf_window;

/**
 * @brief Stable payload for event-node pointer enter/leave signals.
 */
struct wlf_event_pointer_focus_event {
	struct wlf_window *window; /**< Window associated with this object. */
	struct wlf_pointer *pointer; /**< Pointer that generated the event. */
	double x, /**< X coordinate in scene-global coordinates. */ y; /**< Y coordinate in scene-global coordinates. */
};

/**
 * @brief A scene-local input target whose shape is controlled by input_region.
 */
struct wlf_event_node {
	struct wlf_scene_node base; /**< Generic wlf_scene_node interface. */
	pixman_region32_t input_region; /**< Input region in node-local coordinates. */
	bool pointer_inside; /**< Whether the pointer currently targets this node. */
	enum wlf_cursor_shape cursor_shape; /**< Shape used while pointer targets this node. */
	struct {
		struct wlf_signal pointer_enter; /**< Emitted for pointer enter events. */
		struct wlf_signal pointer_leave; /**< Emitted for pointer leave events. */
		struct wlf_signal pointer_motion; /**< Emitted for pointer motion events. */
		struct wlf_signal pointer_button; /**< Emitted for pointer button events. */
		struct wlf_signal pointer_axis; /**< Emitted for pointer axis events. */
		struct wlf_signal pointer_frame; /**< Emitted for pointer frame events. */
		struct wlf_signal keyboard_enter; /**< Emitted for keyboard enter events. */
		struct wlf_signal keyboard_leave; /**< Emitted for keyboard leave events. */
		struct wlf_signal keyboard_keymap; /**< Emitted for keyboard keymap events. */
		struct wlf_signal keyboard_key; /**< Emitted for keyboard key events. */
		struct wlf_signal keyboard_modifiers; /**< Emitted for keyboard modifiers events. */
		struct wlf_signal keyboard_repeat_info; /**< Emitted for keyboard repeat info events. */
		/** Generic tablet payload; protocol-specific event pointer is preserved. */
		struct wlf_signal tablet;
		struct wlf_signal touch_down; /**< Emitted for touch down events. */
		struct wlf_signal touch_up; /**< Emitted for touch up events. */
		struct wlf_signal touch_motion; /**< Emitted for touch motion events. */
		struct wlf_signal touch_cancel; /**< Emitted for touch cancel events. */
		struct wlf_signal touch_frame; /**< Emitted for touch frame events. */
		struct wlf_signal touch_shape; /**< Emitted for touch shape events. */
		struct wlf_signal touch_orientation; /**< Emitted for touch orientation events. */
	} events;
};

/**
 * @brief Creates an event target with a rectangular initial input region.
 *
 * @param parent Parent scene node.
 * @param x X coordinate relative to the parent.
 * @param y Y coordinate relative to the parent.
 * @param width Initial width in logical pixels.
 * @param height Initial height in logical pixels.
 * @return New event target, or NULL on allocation failure.
 */
struct wlf_event_node *wlf_event_node_create(struct wlf_scene_node *parent,
	int x, int y, uint32_t width, uint32_t height);

/**
 * @brief Replaces the node-local input region. NULL clears it.
 *
 * @param node Scene node to inspect or modify.
 * @param region Input region in node-local coordinates, or NULL to clear it.
 */
void wlf_event_node_set_input_region(struct wlf_event_node *node,
	const pixman_region32_t *region);

/**
 * @brief Sets the cursor shape shown while this node is the pointer target.
 *
 * @param node Scene node to inspect or modify.
 * @param shape Cursor shape to apply.
 */
void wlf_event_node_set_cursor_shape(struct wlf_event_node *node,
	enum wlf_cursor_shape shape);

/**
 * @brief Updates pointer focus state and emits pointer_enter once per transition.
 *
 * @param node Scene node to inspect or modify.
 * @param event Pointer focus event payload passed to listeners.
 */
void wlf_event_node_notify_pointer_enter(struct wlf_event_node *node,
	struct wlf_event_pointer_focus_event *event);

/**
 * @brief Updates pointer focus state and emits pointer_leave once per transition.
 *
 * @param node Scene node to inspect or modify.
 * @param event Pointer focus event payload passed to listeners.
 */
void wlf_event_node_notify_pointer_leave(struct wlf_event_node *node,
	struct wlf_event_pointer_focus_event *event);

/**
 * @brief Checks whether a scene node is an event target.
 *
 * @param node Scene node to inspect or modify.
 * @return true for an event target, false otherwise.
 */
bool wlf_scene_node_is_event(const struct wlf_scene_node *node);
/**
 * @brief Casts a scene node to an event target.
 *
 * @param node Scene node to inspect or modify.
 * @return Enclosing event target; asserts on an incompatible node.
 */
struct wlf_event_node *wlf_event_node_from_node(struct wlf_scene_node *node);

/**
 * @brief Returns the topmost event node containing a scene-global point.
 *
 * @param root Root scene node to search.
 * @param x X coordinate in scene-global space.
 * @param y Y coordinate in scene-global space.
 * @return Topmost matching event target, or NULL if no target contains the point.
 */
struct wlf_event_node *wlf_event_node_at(struct wlf_scene_node *root,
	double x, double y);

#endif // SCENE_WLF_EVENT_NODE_H
