/**
 * @file wlf_cursor.h
 * @brief Backend-independent cursor shape control.
 */

#ifndef TYPES_WLF_CURSOR_H
#define TYPES_WLF_CURSOR_H

#include <stdbool.h>
#include <stdint.h>

struct wlf_cursor;

/** Cursor shape identifiers shared with cursor-shape-v1. */
enum wlf_cursor_shape {
	WLF_CURSOR_SHAPE_DEFAULT = 1,
	WLF_CURSOR_SHAPE_CONTEXT_MENU = 2,
	WLF_CURSOR_SHAPE_HELP = 3,
	WLF_CURSOR_SHAPE_POINTER = 4,
	WLF_CURSOR_SHAPE_PROGRESS = 5,
	WLF_CURSOR_SHAPE_WAIT = 6,
	WLF_CURSOR_SHAPE_CELL = 7,
	WLF_CURSOR_SHAPE_CROSSHAIR = 8,
	WLF_CURSOR_SHAPE_TEXT = 9,
	WLF_CURSOR_SHAPE_VERTICAL_TEXT = 10,
	WLF_CURSOR_SHAPE_ALIAS = 11,
	WLF_CURSOR_SHAPE_COPY = 12,
	WLF_CURSOR_SHAPE_MOVE = 13,
	WLF_CURSOR_SHAPE_NO_DROP = 14,
	WLF_CURSOR_SHAPE_NOT_ALLOWED = 15,
	WLF_CURSOR_SHAPE_GRAB = 16,
	WLF_CURSOR_SHAPE_GRABBING = 17,
	WLF_CURSOR_SHAPE_E_RESIZE = 18,
	WLF_CURSOR_SHAPE_N_RESIZE = 19,
	WLF_CURSOR_SHAPE_NE_RESIZE = 20,
	WLF_CURSOR_SHAPE_NW_RESIZE = 21,
	WLF_CURSOR_SHAPE_S_RESIZE = 22,
	WLF_CURSOR_SHAPE_SE_RESIZE = 23,
	WLF_CURSOR_SHAPE_SW_RESIZE = 24,
	WLF_CURSOR_SHAPE_W_RESIZE = 25,
	WLF_CURSOR_SHAPE_EW_RESIZE = 26,
	WLF_CURSOR_SHAPE_NS_RESIZE = 27,
	WLF_CURSOR_SHAPE_NESW_RESIZE = 28,
	WLF_CURSOR_SHAPE_NWSE_RESIZE = 29,
	WLF_CURSOR_SHAPE_COL_RESIZE = 30,
	WLF_CURSOR_SHAPE_ROW_RESIZE = 31,
	WLF_CURSOR_SHAPE_ALL_SCROLL = 32,
	WLF_CURSOR_SHAPE_ZOOM_IN = 33,
	WLF_CURSOR_SHAPE_ZOOM_OUT = 34,
	WLF_CURSOR_SHAPE_DND_ASK = 35,
	WLF_CURSOR_SHAPE_ALL_RESIZE = 36,
};

struct wlf_cursor_impl {
	void (*destroy)(struct wlf_cursor *cursor);
	bool (*set_shape)(struct wlf_cursor *cursor, uint32_t serial,
		enum wlf_cursor_shape shape);
};

struct wlf_cursor {
	const struct wlf_cursor_impl *impl;
	enum wlf_cursor_shape shape;
};

void wlf_cursor_init(struct wlf_cursor *cursor,
	const struct wlf_cursor_impl *impl);
void wlf_cursor_destroy(struct wlf_cursor *cursor);
bool wlf_cursor_set_shape(struct wlf_cursor *cursor, uint32_t serial,
	enum wlf_cursor_shape shape);

#endif
