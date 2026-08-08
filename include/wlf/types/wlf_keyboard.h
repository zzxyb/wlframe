/**
 * @file        wlf_keyboard.h
 * @brief       Keyboard input interface and event definitions in wlframe.
 * @details     This file defines the generic keyboard object, its virtual
 *              interface, and all keyboard-related event payload structures
 *              used with wlf_signal.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef TYPES_WLF_KEYBOARD_H
#define TYPES_WLF_KEYBOARD_H

#include "wlf/utils/wlf_signal.h"

#include <stddef.h>
#include <stdint.h>

struct wlf_keyboard;
struct wlf_window;

/**
 * @brief Keymap format for keyboard events.
 */
enum wlf_keyboard_keymap_format {
	WLF_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP = 0, /**< No keymap. */
	WLF_KEYBOARD_KEYMAP_FORMAT_XKB_V1 = 1,   /**< XKB keymap. */
};

/**
 * @brief Physical key state.
 */
enum wlf_keyboard_key_state {
	WLF_KEYBOARD_KEY_STATE_RELEASED = 0, /**< Key is released. */
	WLF_KEYBOARD_KEY_STATE_PRESSED = 1,  /**< Key is pressed. */
};

/**
 * @brief Virtual methods for keyboard operations.
 */
struct wlf_keyboard_impl {
	const char *name; /**< Implementation name. */

	/**
	 * @brief Destroys backend-specific keyboard resources.
	 * @param keyboard Keyboard instance.
	 */
	void (*destroy)(struct wlf_keyboard *keyboard);
};

/**
 * @brief Base keyboard object.
 *
 * Backend-specific keyboard types should embed this struct.
 */
struct wlf_keyboard {
	const struct wlf_keyboard_impl *impl; /**< Virtual method table. */

	struct {
		struct wlf_signal destroy;   /**< Emitted before keyboard is destroyed. */
		struct wlf_signal keymap;    /**< Payload: wlf_keyboard_keymap_event. */
		struct wlf_signal enter;     /**< Payload: wlf_keyboard_enter_event. */
		struct wlf_signal leave;     /**< Payload: wlf_keyboard_leave_event. */
		struct wlf_signal key;       /**< Payload: wlf_keyboard_key_event. */
		struct wlf_signal modifiers; /**< Payload: wlf_keyboard_modifiers_event. */
		struct wlf_signal repeat_info; /**< Payload: wlf_keyboard_repeat_info_event. */
	} events;

	void *data; /**< User-defined data pointer. */
};

/**
 * @brief Keymap event payload.
 */
struct wlf_keyboard_keymap_event {
	struct wlf_keyboard *keyboard; /**< Keyboard that generated the event. */
	enum wlf_keyboard_keymap_format format; /**< Keymap format. */
	int fd;        /**< File descriptor for the keymap data. */
	uint32_t size; /**< Keymap data size in bytes. */
};

/**
 * @brief Keyboard enter event payload.
 */
struct wlf_keyboard_enter_event {
	struct wlf_keyboard *keyboard; /**< Keyboard that generated the event. */
	uint32_t serial;  /**< Serial of the enter event. */
	struct wlf_window *window; /**< Window gaining keyboard focus. */
	const uint32_t *keys;  /**< Array of currently pressed keycodes. */
	size_t keys_count;     /**< Number of keycodes in keys. */
};

/**
 * @brief Keyboard leave event payload.
 */
struct wlf_keyboard_leave_event {
	struct wlf_keyboard *keyboard; /**< Keyboard that generated the event. */
	uint32_t serial; /**< Serial of the leave event. */
	struct wlf_window *window; /**< Window losing keyboard focus. */
};

/**
 * @brief Key event payload.
 */
struct wlf_keyboard_key_event {
	struct wlf_keyboard *keyboard; /**< Keyboard that generated the event. */
	uint32_t serial;   /**< Serial of the key event. */
	uint32_t time_msec; /**< Timestamp in milliseconds. */
	uint32_t key;      /**< Hardware key code. */
	enum wlf_keyboard_key_state state; /**< Key state. */
};

/**
 * @brief Modifier state event payload.
 */
struct wlf_keyboard_modifiers_event {
	struct wlf_keyboard *keyboard; /**< Keyboard that generated the event. */
	uint32_t serial;         /**< Serial of the modifiers event. */
	uint32_t mods_depressed; /**< Currently depressed modifier keys. */
	uint32_t mods_latched;   /**< Currently latched modifier keys. */
	uint32_t mods_locked;    /**< Currently locked modifier keys. */
	uint32_t group;          /**< Active keyboard layout (group). */
};

/**
 * @brief Key repeat info event payload.
 */
struct wlf_keyboard_repeat_info_event {
	struct wlf_keyboard *keyboard; /**< Keyboard that generated the event. */
	int32_t rate;  /**< Repeat rate in characters per second; 0 disables. */
	int32_t delay; /**< Delay in milliseconds before first repeat. */
};

/**
 * @brief Initializes a keyboard object.
 *
 * @param keyboard Keyboard object to initialize.
 * @param impl     Implementation virtual methods.
 */
void wlf_keyboard_init(struct wlf_keyboard *keyboard,
	const struct wlf_keyboard_impl *impl);

/**
 * @brief Destroys a keyboard object.
 *
 * @param keyboard Keyboard object to destroy.
 */
void wlf_keyboard_destroy(struct wlf_keyboard *keyboard);

#endif /* TYPES_WLF_KEYBOARD_H */
