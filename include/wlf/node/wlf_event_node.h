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

/** Stable payload for event-node pointer enter/leave signals. */
struct wlf_event_pointer_focus_event {
	struct wlf_window *window;
	struct wlf_pointer *pointer;
	double x, y;
};

/** A scene-local input target whose shape is controlled by input_region. */
struct wlf_event_node {
	struct wlf_scene_node base;
	pixman_region32_t input_region;
	bool pointer_inside; /**< Whether the pointer currently targets this node. */
	enum wlf_cursor_shape cursor_shape; /**< Shape used while pointer targets this node. */
	struct {
		struct wlf_signal pointer_enter;
		struct wlf_signal pointer_leave;
		struct wlf_signal pointer_motion;
		struct wlf_signal pointer_button;
		struct wlf_signal pointer_axis;
		struct wlf_signal pointer_frame;
		struct wlf_signal keyboard_enter;
		struct wlf_signal keyboard_leave;
		struct wlf_signal keyboard_keymap;
		struct wlf_signal keyboard_key;
		struct wlf_signal keyboard_modifiers;
		struct wlf_signal keyboard_repeat_info;
		/** Generic tablet payload; protocol-specific event pointer is preserved. */
		struct wlf_signal tablet;
		struct wlf_signal touch_down;
		struct wlf_signal touch_up;
		struct wlf_signal touch_motion;
		struct wlf_signal touch_cancel;
		struct wlf_signal touch_frame;
		struct wlf_signal touch_shape;
		struct wlf_signal touch_orientation;
	} events;
};

/** Creates an event target with a rectangular initial input region. */
struct wlf_event_node *wlf_event_node_create(struct wlf_scene_node *parent,
	int x, int y, uint32_t width, uint32_t height);

/** Replaces the node-local input region. NULL clears it. */
void wlf_event_node_set_input_region(struct wlf_event_node *node,
	const pixman_region32_t *region);

/** Sets the cursor shape shown while this node is the pointer target. */
void wlf_event_node_set_cursor_shape(struct wlf_event_node *node,
	enum wlf_cursor_shape shape);

/** Updates pointer focus state and emits pointer_enter once per transition. */
void wlf_event_node_notify_pointer_enter(struct wlf_event_node *node,
	const struct wlf_event_pointer_focus_event *event);

/** Updates pointer focus state and emits pointer_leave once per transition. */
void wlf_event_node_notify_pointer_leave(struct wlf_event_node *node,
	const struct wlf_event_pointer_focus_event *event);

bool wlf_scene_node_is_event(const struct wlf_scene_node *node);
struct wlf_event_node *wlf_event_node_from_node(struct wlf_scene_node *node);

/** Returns the topmost event node containing a scene-global point. */
struct wlf_event_node *wlf_event_node_at(struct wlf_scene_node *root,
	double x, double y);

#endif // SCENE_WLF_EVENT_NODE_H
