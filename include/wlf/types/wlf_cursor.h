/**
 * @file        wlf_cursor.h
 * @brief       Backend-independent cursor shape control.
 * @details     This file defines cursor shape identifiers and the generic
 *              cursor interface shared by platform backends.
 * @author      YaoBing Xiao
 * @date        2026-08-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-09, initial version\n
 */

#ifndef TYPES_WLF_CURSOR_H
#define TYPES_WLF_CURSOR_H

#include <stdbool.h>
#include <stdint.h>

struct wlf_cursor;

/**
 * @brief Cursor shape identifiers shared with cursor-shape-v1.
 */
enum wlf_cursor_shape {
	WLF_CURSOR_SHAPE_DEFAULT = 1, /**< Cursor shape: default. */
	WLF_CURSOR_SHAPE_CONTEXT_MENU = 2, /**< Cursor shape: context menu. */
	WLF_CURSOR_SHAPE_HELP = 3, /**< Cursor shape: help. */
	WLF_CURSOR_SHAPE_POINTER = 4, /**< Cursor shape: pointer. */
	WLF_CURSOR_SHAPE_PROGRESS = 5, /**< Cursor shape: progress. */
	WLF_CURSOR_SHAPE_WAIT = 6, /**< Cursor shape: wait. */
	WLF_CURSOR_SHAPE_CELL = 7, /**< Cursor shape: cell. */
	WLF_CURSOR_SHAPE_CROSSHAIR = 8, /**< Cursor shape: crosshair. */
	WLF_CURSOR_SHAPE_TEXT = 9, /**< Cursor shape: text. */
	WLF_CURSOR_SHAPE_VERTICAL_TEXT = 10, /**< Cursor shape: vertical text. */
	WLF_CURSOR_SHAPE_ALIAS = 11, /**< Cursor shape: alias. */
	WLF_CURSOR_SHAPE_COPY = 12, /**< Cursor shape: copy. */
	WLF_CURSOR_SHAPE_MOVE = 13, /**< Cursor shape: move. */
	WLF_CURSOR_SHAPE_NO_DROP = 14, /**< Cursor shape: no drop. */
	WLF_CURSOR_SHAPE_NOT_ALLOWED = 15, /**< Cursor shape: not allowed. */
	WLF_CURSOR_SHAPE_GRAB = 16, /**< Cursor shape: grab. */
	WLF_CURSOR_SHAPE_GRABBING = 17, /**< Cursor shape: grabbing. */
	WLF_CURSOR_SHAPE_E_RESIZE = 18, /**< Cursor shape: e resize. */
	WLF_CURSOR_SHAPE_N_RESIZE = 19, /**< Cursor shape: n resize. */
	WLF_CURSOR_SHAPE_NE_RESIZE = 20, /**< Cursor shape: ne resize. */
	WLF_CURSOR_SHAPE_NW_RESIZE = 21, /**< Cursor shape: nw resize. */
	WLF_CURSOR_SHAPE_S_RESIZE = 22, /**< Cursor shape: s resize. */
	WLF_CURSOR_SHAPE_SE_RESIZE = 23, /**< Cursor shape: se resize. */
	WLF_CURSOR_SHAPE_SW_RESIZE = 24, /**< Cursor shape: sw resize. */
	WLF_CURSOR_SHAPE_W_RESIZE = 25, /**< Cursor shape: w resize. */
	WLF_CURSOR_SHAPE_EW_RESIZE = 26, /**< Cursor shape: ew resize. */
	WLF_CURSOR_SHAPE_NS_RESIZE = 27, /**< Cursor shape: ns resize. */
	WLF_CURSOR_SHAPE_NESW_RESIZE = 28, /**< Cursor shape: nesw resize. */
	WLF_CURSOR_SHAPE_NWSE_RESIZE = 29, /**< Cursor shape: nwse resize. */
	WLF_CURSOR_SHAPE_COL_RESIZE = 30, /**< Cursor shape: col resize. */
	WLF_CURSOR_SHAPE_ROW_RESIZE = 31, /**< Cursor shape: row resize. */
	WLF_CURSOR_SHAPE_ALL_SCROLL = 32, /**< Cursor shape: all scroll. */
	WLF_CURSOR_SHAPE_ZOOM_IN = 33, /**< Cursor shape: zoom in. */
	WLF_CURSOR_SHAPE_ZOOM_OUT = 34, /**< Cursor shape: zoom out. */
	WLF_CURSOR_SHAPE_DND_ASK = 35, /**< Cursor shape: dnd ask. */
	WLF_CURSOR_SHAPE_ALL_RESIZE = 36, /**< Cursor shape: all resize. */
};

/**
 * @brief Platform callbacks for cursor shape control.
 */
struct wlf_cursor_impl {
	void (*destroy)(struct wlf_cursor *cursor); /**< Releases implementation-specific resources. */
	/**
	 * @brief Applies a platform cursor shape.
	 *
	 * @param cursor Cursor controller to modify.
	 * @param serial Pointer enter event serial.
	 * @param shape Requested cursor shape.
	 * @return true if the shape was applied, false on failure.
	 */
	bool (*set_shape)(struct wlf_cursor *cursor, uint32_t serial,
		enum wlf_cursor_shape shape);
};

/**
 * @brief Generic cursor controller and its current shape.
 */
struct wlf_cursor {
	const struct wlf_cursor_impl *impl; /**< Implementation virtual method table. */
	enum wlf_cursor_shape shape; /**< Currently selected cursor shape. */
};

/**
 * @brief Initializes a cursor controller with platform callbacks.
 *
 * @param cursor Cursor controller to initialize, modify or destroy.
 * @param impl Implementation callbacks used by the object.
 */
void wlf_cursor_init(struct wlf_cursor *cursor,
	const struct wlf_cursor_impl *impl);
/**
 * @brief Destroys a cursor controller and releases its resources.
 *
 * @param cursor Cursor controller to initialize, modify or destroy.
 */
void wlf_cursor_destroy(struct wlf_cursor *cursor);
/**
 * @brief Applies a cursor shape using a valid input event serial.
 *
 * @param cursor Cursor controller to initialize, modify or destroy.
 * @param serial Nonzero serial from a pointer enter event.
 * @param shape Cursor shape to apply.
 * @return true if the shape was applied, false for invalid input or a backend failure.
 */
bool wlf_cursor_set_shape(struct wlf_cursor *cursor, uint32_t serial,
	enum wlf_cursor_shape shape);

#endif
