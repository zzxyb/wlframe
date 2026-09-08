#include "wlf/types/macos/pointer.h"

#include "wlf/types/wlf_cursor.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#import <AppKit/AppKit.h>

#include <assert.h>
#include <stdlib.h>

struct wlf_macos_cursor {
	struct wlf_cursor base;
};

static NSCursor *cursor_for_shape(enum wlf_cursor_shape shape) {
	switch (shape) {
	case WLF_CURSOR_SHAPE_POINTER:
	case WLF_CURSOR_SHAPE_CONTEXT_MENU:
		return [NSCursor pointingHandCursor];
	case WLF_CURSOR_SHAPE_CROSSHAIR:
	case WLF_CURSOR_SHAPE_CELL:
		return [NSCursor crosshairCursor];
	case WLF_CURSOR_SHAPE_TEXT:
		return [NSCursor IBeamCursor];
	case WLF_CURSOR_SHAPE_VERTICAL_TEXT:
		return [NSCursor IBeamCursorForVerticalLayout];
	case WLF_CURSOR_SHAPE_GRAB:
	case WLF_CURSOR_SHAPE_MOVE:
	case WLF_CURSOR_SHAPE_ALL_SCROLL:
		return [NSCursor openHandCursor];
	case WLF_CURSOR_SHAPE_GRABBING:
		return [NSCursor closedHandCursor];
	case WLF_CURSOR_SHAPE_E_RESIZE:
	case WLF_CURSOR_SHAPE_W_RESIZE:
	case WLF_CURSOR_SHAPE_EW_RESIZE:
	case WLF_CURSOR_SHAPE_COL_RESIZE:
		return [NSCursor resizeLeftRightCursor];
	case WLF_CURSOR_SHAPE_N_RESIZE:
	case WLF_CURSOR_SHAPE_S_RESIZE:
	case WLF_CURSOR_SHAPE_NS_RESIZE:
	case WLF_CURSOR_SHAPE_ROW_RESIZE:
		return [NSCursor resizeUpDownCursor];
	case WLF_CURSOR_SHAPE_NO_DROP:
	case WLF_CURSOR_SHAPE_NOT_ALLOWED:
		return [NSCursor operationNotAllowedCursor];
	case WLF_CURSOR_SHAPE_COPY:
		return [NSCursor dragCopyCursor];
	default:
		return [NSCursor arrowCursor];
	}
}

static void cursor_destroy(struct wlf_cursor *base) {
	struct wlf_macos_cursor *cursor =
		wlf_container_of(base, cursor, base);
	free(cursor);
}

static bool cursor_set_shape(struct wlf_cursor *base, uint32_t serial,
		enum wlf_cursor_shape shape) {
	(void)base;
	if (serial == 0) {
		return false;
	}
	[cursor_for_shape(shape) set];
	return true;
}

static const struct wlf_cursor_impl cursor_impl = {
	.destroy = cursor_destroy,
	.set_shape = cursor_set_shape,
};

static struct wlf_cursor *macos_cursor_create(void) {
	struct wlf_macos_cursor *cursor = calloc(1, sizeof(*cursor));
	if (cursor == NULL) {
		return NULL;
	}
	wlf_cursor_init(&cursor->base, &cursor_impl);
	return &cursor->base;
}

static void pointer_destroy(struct wlf_pointer *base) {
	struct wlf_macos_pointer *pointer =
		wlf_macos_pointer_from_pointer(base);
	wlf_cursor_destroy(base->cursor);
	base->cursor = NULL;
	free(pointer);
}

static const struct wlf_pointer_impl pointer_impl = {
	.name = "AppKit pointer",
	.destroy = pointer_destroy,
};

struct wlf_macos_pointer *wlf_macos_pointer_create(void) {
	struct wlf_macos_pointer *pointer = calloc(1, sizeof(*pointer));
	if (pointer == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate macOS pointer");
		return NULL;
	}
	wlf_pointer_init(&pointer->base, &pointer_impl);
	pointer->base.cursor = macos_cursor_create();
	if (pointer->base.cursor == NULL) {
		wlf_pointer_destroy(&pointer->base);
		return NULL;
	}
	return pointer;
}

bool wlf_pointer_is_macos(const struct wlf_pointer *pointer) {
	return pointer != NULL && pointer->impl == &pointer_impl;
}

struct wlf_macos_pointer *wlf_macos_pointer_from_pointer(
		struct wlf_pointer *pointer) {
	assert(wlf_pointer_is_macos(pointer));
	struct wlf_macos_pointer *macos = NULL;
	return wlf_container_of(pointer, macos, base);
}

uint32_t wlf_macos_pointer_next_serial(struct wlf_macos_pointer *pointer) {
	pointer->serial++;
	if (pointer->serial == 0) {
		pointer->serial++;
	}
	return pointer->serial;
}
