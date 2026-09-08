#include "wlf/types/macos/keyboard.h"

#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>

static void keyboard_destroy(struct wlf_keyboard *base) {
	struct wlf_macos_keyboard *keyboard =
		wlf_macos_keyboard_from_keyboard(base);
	free(keyboard);
}

static const struct wlf_keyboard_impl keyboard_impl = {
	.name = "AppKit keyboard",
	.destroy = keyboard_destroy,
};

struct wlf_macos_keyboard *wlf_macos_keyboard_create(void) {
	struct wlf_macos_keyboard *keyboard = calloc(1, sizeof(*keyboard));
	if (keyboard == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate macOS keyboard");
		return NULL;
	}
	wlf_keyboard_init(&keyboard->base, &keyboard_impl);
	return keyboard;
}

bool wlf_keyboard_is_macos(const struct wlf_keyboard *keyboard) {
	return keyboard != NULL && keyboard->impl == &keyboard_impl;
}

struct wlf_macos_keyboard *wlf_macos_keyboard_from_keyboard(
		struct wlf_keyboard *keyboard) {
	assert(wlf_keyboard_is_macos(keyboard));
	struct wlf_macos_keyboard *macos = NULL;
	return wlf_container_of(keyboard, macos, base);
}

uint32_t wlf_macos_keyboard_next_serial(struct wlf_macos_keyboard *keyboard) {
	keyboard->serial++;
	if (keyboard->serial == 0) {
		keyboard->serial++;
	}
	return keyboard->serial;
}

void wlf_macos_keyboard_set_key_state(struct wlf_macos_keyboard *keyboard,
		uint32_t key, enum wlf_keyboard_key_state state) {
	size_t index = 0;
	while (index < keyboard->key_count && keyboard->keys[index] != key) {
		index++;
	}
	if (state == WLF_KEYBOARD_KEY_STATE_PRESSED) {
		if (index == keyboard->key_count &&
				keyboard->key_count < WLF_MACOS_KEYBOARD_KEYS_CAP) {
			keyboard->keys[keyboard->key_count++] = key;
		}
		return;
	}
	if (index == keyboard->key_count) {
		return;
	}
	for (size_t i = index + 1; i < keyboard->key_count; ++i) {
		keyboard->keys[i - 1] = keyboard->keys[i];
	}
	keyboard->key_count--;
}
