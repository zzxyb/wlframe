/**
 * @file        wlf_clipboard.h
 * @brief       Clipboard abstraction for wlframe.
 * @details     This file provides a unified interface for clipboard operations,
 *              including setting/getting data in various MIME types (text, image, etc.),
 *              clipboard modes (clipboard/primary selection), and change notifications.
 *              The design is inspired by Qt's QClipboard.
 *
 *              Typical usage:
 *                  - Create clipboard: wlf_clipboard_create()
 *                  - Set text: wlf_clipboard_set_text()
 *                  - Get text: wlf_clipboard_text()
 *                  - Listen to changes: connect to events.changed signal
 *
 * @author      YaoBing Xiao
 * @date        2026-02-10
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-10, initial version\n
 */

#ifndef CLIPBOARD_WLF_CLIPBOARD_H
#define CLIPBOARD_WLF_CLIPBOARD_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct wlf_clipboard;

/**
 * @brief Clipboard mode enumeration
 *
 * Different clipboard modes for different selection types.
 * In X11/Wayland, there are typically two clipboards:
 * - Clipboard: Explicit copy/paste (Ctrl+C/Ctrl+V)
 * - Selection: Implicit selection (middle-click paste)
 */
enum wlf_clipboard_mode {
	WLF_CLIPBOARD_MODE_CLIPBOARD = 0,  /**< Standard clipboard (Ctrl+C/Ctrl+V) */
	WLF_CLIPBOARD_MODE_SELECTION,       /**< Primary selection (middle-click) */
};

/**
 * @brief Structure representing clipboard data with MIME type
 */
struct wlf_clipboard_data {
	char *mime_type;      /**< MIME type of the data (e.g., "text/plain", "image/png") */
	void *data;           /**< Pointer to the actual data */
	size_t size;          /**< Size of the data in bytes */

	struct wlf_linked_list link;  /**< Linked list node for multiple data types */
};

/**
 * @brief Virtual methods for clipboard operations.
 *
 * This structure defines the interface that clipboard implementations must provide.
 */
struct wlf_clipboard_impl {
	/**
	 * @brief Destroy the clipboard.
	 * @param clipboard Clipboard to destroy.
	 */
	void (*destroy)(struct wlf_clipboard *clipboard);

	/**
	 * @brief Set clipboard data for a specific MIME type.
	 * @param clipboard Clipboard to modify.
	 * @param mode Clipboard mode (clipboard or selection).
	 * @param mime_type MIME type of the data.
	 * @param data Pointer to the data.
	 * @param size Size of the data in bytes.
	 * @return true on success, false on failure.
	 */
	bool (*set_data)(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, const char *mime_type,
		const void *data, size_t size);

	/**
	 * @brief Get clipboard data for a specific MIME type.
	 * @param clipboard Clipboard to query.
	 * @param mode Clipboard mode (clipboard or selection).
	 * @param mime_type MIME type of the data to retrieve.
	 * @param size Output parameter for the size of the data.
	 * @return Pointer to the data (caller must free), or NULL if not available.
	 */
	void *(*get_data)(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, const char *mime_type, size_t *size);

	/**
	 * @brief Get list of available MIME types in the clipboard.
	 * @param clipboard Clipboard to query.
	 * @param mode Clipboard mode (clipboard or selection).
	 * @param count Output parameter for the number of MIME types.
	 * @return Array of MIME type strings (caller must free), or NULL if empty.
	 */
	char **(*get_mime_types)(struct wlf_clipboard *clipboard,
		enum wlf_clipboard_mode mode, size_t *count);

	/**
	 * @brief Clear all clipboard data.
	 * @param clipboard Clipboard to clear.
	 * @param mode Clipboard mode (clipboard or selection).
	 */
	void (*clear)(struct wlf_clipboard *clipboard, enum wlf_clipboard_mode mode);
};

/**
 * @brief Main clipboard structure.
 *
 * Provides a unified interface for clipboard operations across different platforms.
 */
struct wlf_clipboard {
	const struct wlf_clipboard_impl *impl;  /**< Implementation methods */

	struct {
		struct wlf_signal destroy;         /**< Emitted when clipboard is destroyed */
		struct wlf_signal changed;         /**< Emitted when clipboard content changes */
		struct wlf_signal selection_changed; /**< Emitted when selection changes */
	} events;

	/**
	 * Implementation-specific data pointer.
	 * Each clipboard implementation can store its private data here.
	 */
	void *data;
};

/**
 * @brief Create a new clipboard.
 *
 * @param impl Implementation methods for this clipboard.
 * @return Pointer to the newly created clipboard, or NULL on failure.
 */
struct wlf_clipboard *wlf_clipboard_create(const struct wlf_clipboard_impl *impl);

/**
 * @brief Destroy a clipboard.
 *
 * @param clipboard Clipboard to destroy.
 */
void wlf_clipboard_destroy(struct wlf_clipboard *clipboard);

/**
 * @brief Set clipboard data for a specific MIME type.
 *
 * @param clipboard Clipboard to modify.
 * @param mode Clipboard mode (clipboard or selection).
 * @param mime_type MIME type of the data.
 * @param data Pointer to the data.
 * @param size Size of the data in bytes.
 * @return true on success, false on failure.
 */
bool wlf_clipboard_set_data(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode, const char *mime_type,
	const void *data, size_t size);

/**
 * @brief Get clipboard data for a specific MIME type.
 *
 * @param clipboard Clipboard to query.
 * @param mode Clipboard mode (clipboard or selection).
 * @param mime_type MIME type of the data to retrieve.
 * @param size Output parameter for the size of the data.
 * @return Pointer to the data (caller must free), or NULL if not available.
 */
void *wlf_clipboard_get_data(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode, const char *mime_type, size_t *size);

/**
 * @brief Set plain text to clipboard.
 *
 * Convenience function for setting text/plain data.
 *
 * @param clipboard Clipboard to modify.
 * @param mode Clipboard mode (clipboard or selection).
 * @param text Text string to set (null-terminated).
 * @return true on success, false on failure.
 */
bool wlf_clipboard_set_text(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode, const char *text);

/**
 * @brief Get plain text from clipboard.
 *
 * Convenience function for getting text/plain data.
 *
 * @param clipboard Clipboard to query.
 * @param mode Clipboard mode (clipboard or selection).
 * @return Text string (caller must free), or NULL if not available.
 */
char *wlf_clipboard_text(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode);

/**
 * @brief Get list of available MIME types in the clipboard.
 *
 * @param clipboard Clipboard to query.
 * @param mode Clipboard mode (clipboard or selection).
 * @param count Output parameter for the number of MIME types.
 * @return Array of MIME type strings (caller must free each string and the array),
 *         or NULL if clipboard is empty.
 */
char **wlf_clipboard_get_mime_types(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode, size_t *count);

/**
 * @brief Clear all clipboard data.
 *
 * @param clipboard Clipboard to clear.
 * @param mode Clipboard mode (clipboard or selection).
 */
void wlf_clipboard_clear(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode);

/**
 * @brief Check if clipboard has data of a specific MIME type.
 *
 * @param clipboard Clipboard to query.
 * @param mode Clipboard mode (clipboard or selection).
 * @param mime_type MIME type to check for.
 * @return true if the MIME type is available, false otherwise.
 */
bool wlf_clipboard_has_mime_type(struct wlf_clipboard *clipboard,
	enum wlf_clipboard_mode mode, const char *mime_type);

#endif // UTILS_WLF_CLIPBOARD_H
