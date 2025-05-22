/**
 * @file        wlf_keyboard.h
 * @brief       Keyboard input device type definition for wlframe.
 * @details     This file provides the structure definitions and functions for keyboard input devices,
 *              including key events, modifier tracking, keymap management, repeat info, and event signals
 *              for keyboard actions. It supports modifier state, key repeat, and device-specific data.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef TYPES_WLF_KEYBOARD_H
#define TYPES_WLF_KEYBOARD_H

#include "wlf/utils/wlf_signal.h"
#include <stdbool.h>
#include <stdint.h>
#include <xkbcommon/xkbcommon.h>

#define WLF_LED_COUNT 3
#define WLF_KEYBOARD_KEYS_CAP 32
#define WLF_MODIFIER_COUNT 8

struct wlf_keyboard;

/**
 * @brief Enumeration of keyboard modifier states.
 */
enum wlf_keyboard_modifier {
	WLF_MODIFIER_SHIFT = 1 << 0, /**< Shift modifier */
	WLF_MODIFIER_CAPS  = 1 << 1, /**< Caps Lock modifier */
	WLF_MODIFIER_CTRL  = 1 << 2, /**< Control modifier */
	WLF_MODIFIER_ALT   = 1 << 3, /**< Alt modifier */
	WLF_MODIFIER_MOD2  = 1 << 4, /**< Modifier 2 */
	WLF_MODIFIER_MOD3  = 1 << 5, /**< Modifier 3 */
	WLF_MODIFIER_LOGO  = 1 << 6, /**< Logo key modifier */
	WLF_MODIFIER_MOD5  = 1 << 7, /**< Modifier 5 */
};

/**
 * @brief Structure representing the state of keyboard modifiers.
 */
struct wlf_keyboard_modifiers {
    xkb_mod_mask_t depressed; /**< Mask of currently depressed modifiers */
    xkb_mod_mask_t latched;   /**< Mask of latched modifiers */
    xkb_mod_mask_t locked;    /**< Mask of locked modifiers */
};

/**
 * @brief Enumeration of keyboard key states.
 */
enum wlf_keyboard_key_state {
	WLF_KEYBOARD_KEY_STATE_RELEASED = 0, /**< Key is not pressed */
	WLF_KEYBOARD_KEY_STATE_PRESSED  = 1, /**< Key is pressed */
};

/**
 * @brief Keyboard implementation interface.
 * @details This structure defines function keyboards for keyboard device implementations,
 *          allowing backend-specific or device-specific behavior to be provided.
 */
struct wlf_keyboard_impl {
	const char *(*name)(const struct wlf_keyboard *keyboard); /**< Function to get the name of the keyboard */
};

/**
 * @brief Structure representing a keyboard input device.
 */
struct wlf_keyboard {
	const struct wlf_keyboard_impl *impl; /**< Pointer to the keyboard implementation interface */

	char *keymap_string;                        /**< Keymap string for the keyboard */
	size_t keymap_size;                         /**< Size of the keymap */
	int keymap_fd;                              /**< File descriptor for the keymap */
	struct xkb_keymap *keymap;                  /**< Pointer to the XKB keymap */
	struct xkb_state *xkb_state;                /**< Pointer to the XKB state */
	xkb_led_index_t led_indexes[WLF_LED_COUNT]; /**< Array of LED indexes */
	xkb_mod_index_t mod_indexes[WLF_MODIFIER_COUNT]; /**< Array of modifier indexes */

	uint32_t keycodes[WLF_KEYBOARD_KEYS_CAP];   /**< Array of keycodes */
	size_t num_keycodes;                        /**< Number of keycodes currently in use */
	struct wlf_keyboard_modifiers modifiers;    /**< Current state of keyboard modifiers */

	struct {
		int32_t rate;   /**< Key repeat rate in repeats per second */
		int32_t delay;  /**< Key repeat delay in milliseconds */
	} repeat_info;      /**< Information about key repeat settings */

	struct {
		struct wlf_signal key;         /**< Signal emitted for key press/release events */
		struct wlf_signal modifiers;   /**< Signal emitted when modifier state changes */
		struct wlf_signal keymap;      /**< Signal emitted when the keymap changes */
		struct wlf_signal repeat_info; /**< Signal emitted when repeat info changes */

		struct wlf_signal destroy;     /**< Signal emitted when seat is destroyed */
	} events;           /**< Events related to keyboard actions */

	void *data;         /**< Pointer to device-specific data */
};

/**
 * @brief Structure representing a keyboard key event.
 */
struct wlf_keyboard_key_event {
	uint32_t time_msec;                  /**< Time of the event in milliseconds */
	uint32_t keycode;                    /**< Keycode of the key event */
	bool update_state;                   /**< Indicates if the state should be updated */
	enum wlf_keyboard_key_state state;   /**< State of the key (pressed/released) */
};

/**
 * @brief Create a new keyboard object.
 * @return Pointer to the newly created keyboard.
 */
struct wlf_keyboard *wlf_keyboard_create(void);

/**
 * @brief Destroy a keyboard object and free its resources.
 * @param kb Pointer to the keyboard object.
 */
void wlf_keyboard_destroy(struct wlf_keyboard *kb);

/**
 * @brief Set the keymap for a keyboard.
 * @param kb The keyboard to set the keymap for.
 * @param keymap The new keymap to set.
 * @return True if the keymap was set successfully, false otherwise.
 */
bool wlf_keyboard_set_keymap(struct wlf_keyboard *kb, struct xkb_keymap *keymap);

/**
 * @brief Check if two keymaps match.
 * @param km1 The first keymap.
 * @param km2 The second keymap.
 * @return True if the keymaps match, false otherwise.
 */
bool wlf_keyboard_keymaps_match(struct xkb_keymap *km1, struct xkb_keymap *km2);

/**
 * @brief Interpret pointer button key symbols.
 * @param keysym The key symbol to interpret.
 * @return The corresponding button code or 0.
 */
uint32_t wlf_keyboard_keysym_to_pointer_button(xkb_keysym_t keysym);

/**
 * @brief Interpret pointer motion key symbols.
 * @param keysym The key symbol to interpret.
 * @param dx Pointer to store the horizontal delta.
 * @param dy Pointer to store the vertical delta.
 */
void wlf_keyboard_keysym_to_pointer_motion(xkb_keysym_t keysym, int *dx, int *dy);

/**
 * @brief Set the keyboard repeat info.
 * @param kb The keyboard to set the repeat info for.
 * @param rate_hz The repeat rate in key repeats per second.
 * @param delay_ms The delay in milliseconds before repeat starts.
 */
void wlf_keyboard_set_repeat_info(struct wlf_keyboard *kb, int32_t rate_hz, int32_t delay_ms);

/**
 * @brief Get the set of currently depressed or latched modifiers.
 * @param keyboard The keyboard to query.
 * @return A bitmask of currently active modifiers.
 */
uint32_t wlf_keyboard_get_modifiers(struct wlf_keyboard *keyboard);

#endif // TYPES_WLF_KEYBOARD_H
