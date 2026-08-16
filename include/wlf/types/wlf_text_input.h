/**
 * @file wlf_text_input.h
 * @brief Backend-independent committed and composing text events.
 */

#ifndef WLF_TYPES_WLF_TEXT_INPUT_H
#define WLF_TYPES_WLF_TEXT_INPUT_H

#include <stddef.h>

struct wlf_window;

/** UTF-8 text committed by a keyboard layout or input method. */
struct wlf_text_input_commit_event {
	struct wlf_window *window;
	const char *text; /**< Callback-lifetime UTF-8 string. */
};

/** UTF-8 composition text which has not yet been committed. */
struct wlf_text_input_preedit_event {
	struct wlf_window *window;
	const char *text; /**< Callback-lifetime UTF-8 string. */
	size_t cursor_begin; /**< UTF-8 byte offset of the selection start. */
	size_t cursor_end; /**< UTF-8 byte offset of the selection end. */
};

#endif /* WLF_TYPES_WLF_TEXT_INPUT_H */
