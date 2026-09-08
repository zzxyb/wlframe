/**
 * @file keyboard.h
 * @brief AppKit keyboard object.
 */

#ifndef WLF_TYPES_MACOS_KEYBOARD_H
#define WLF_TYPES_MACOS_KEYBOARD_H

#include "wlf/types/wlf_keyboard.h"

#include <stdbool.h>
#include <stdint.h>

#define WLF_MACOS_KEYBOARD_KEYS_CAP 64

struct wlf_macos_keyboard {
	struct wlf_keyboard base;
	uint32_t serial;
	uint32_t modifiers; /**< NSEvent device-independent modifier flags. */
	uint32_t keys[WLF_MACOS_KEYBOARD_KEYS_CAP];
	size_t key_count;
};

struct wlf_macos_keyboard *wlf_macos_keyboard_create(void);

bool wlf_keyboard_is_macos(const struct wlf_keyboard *keyboard);

struct wlf_macos_keyboard *wlf_macos_keyboard_from_keyboard(
	struct wlf_keyboard *keyboard);

uint32_t wlf_macos_keyboard_next_serial(struct wlf_macos_keyboard *keyboard);

/** Updates the set returned in subsequent keyboard-enter events. */
void wlf_macos_keyboard_set_key_state(struct wlf_macos_keyboard *keyboard,
	uint32_t key, enum wlf_keyboard_key_state state);

#endif /* WLF_TYPES_MACOS_KEYBOARD_H */
