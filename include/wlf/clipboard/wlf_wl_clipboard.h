/**
 * @file        wlf_wl_clipboard.h
 * @brief       Wayland clipboard implementation for wlframe.
 * @details     This file provides the Wayland-specific implementation of the clipboard
 *              interface using the wl_data_device_manager protocol. It supports both
 *              clipboard (Ctrl+C/V) and primary selection (middle-click) modes.
 *
 *              The implementation handles:
 *                  - Data offers from other Wayland clients
 *                  - Data sources for providing clipboard data to other clients
 *                  - MIME type negotiation
 *                  - Multiple clipboard modes (clipboard and selection)
 *
 * @author      YaoBing Xiao
 * @date        2026-02-10
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-10, initial version\n
 */

#ifndef CLIPBOARD_WLF_WL_CLIPBOARD_H
#define CLIPBOARD_WLF_WL_CLIPBOARD_H

#include "wlf/clipboard/wlf_clipboard.h"
#include "wlf/wayland/wlf_wl_display.h"
#include "wlf/utils/wlf_linked_list.h"

#include <wayland-client.h>

struct wl_seat;
struct wl_data_device_manager;
struct wl_data_device;
struct wl_data_source;
struct wl_data_offer;

/**
 * @brief Structure representing a data offer from another Wayland client.
 */
struct wlf_wl_data_offer {
	struct wl_data_offer *offer;      /**< Wayland data offer object */
	struct wlf_linked_list mime_types; /**< List of available MIME types */

	struct wlf_linked_list link;      /**< Linked list node */
};

/**
 * @brief Structure representing a MIME type entry.
 */
struct wlf_wl_mime_type {
	char *mime_type;                  /**< MIME type string */
	struct wlf_linked_list link;      /**< Linked list node */
};

/**
 * @brief Structure representing clipboard data for a specific mode.
 */
struct wlf_wl_clipboard_data {
	struct wl_data_source *source;    /**< Wayland data source for this data */
	struct wlf_linked_list entries;   /**< List of clipboard data entries */
};

/**
 * @brief Wayland clipboard implementation structure.
 */
struct wlf_wl_clipboard {
	struct wlf_clipboard base;        /**< Base clipboard structure */

	struct wlf_wl_display *display;   /**< Wayland display */
	struct wl_seat *seat;             /**< Wayland seat for input */
	struct wl_data_device_manager *data_device_manager; /**< Data device manager */
	struct wl_data_device *data_device; /**< Data device for clipboard operations */

	// Clipboard and selection data
	struct wlf_wl_clipboard_data clipboard_data;  /**< Clipboard mode data */
	struct wlf_wl_clipboard_data selection_data;  /**< Selection mode data */

	// Received data offers
	struct wl_data_offer *clipboard_offer;  /**< Current clipboard offer */
	struct wl_data_offer *selection_offer;  /**< Current selection offer */

	struct wlf_linked_list clipboard_mime_types;  /**< Available MIME types in clipboard */
	struct wlf_linked_list selection_mime_types;  /**< Available MIME types in selection */
};

/**
 * @brief Create a Wayland clipboard.
 *
 * @param display Wayland display to use.
 * @param seat Wayland seat for input (can be NULL, will use first available).
 * @return Pointer to the newly created Wayland clipboard, or NULL on failure.
 */
struct wlf_wl_clipboard *wlf_wl_clipboard_create(
	struct wlf_wl_display *display, struct wl_seat *seat);

/**
 * @brief Destroy a Wayland clipboard.
 *
 * @param clipboard Wayland clipboard to destroy.
 */
void wlf_wl_clipboard_destroy(struct wlf_wl_clipboard *clipboard);

/**
 * @brief Get the base clipboard structure from a Wayland clipboard.
 *
 * @param wl_clipboard Wayland clipboard.
 * @return Pointer to the base clipboard structure.
 */
static inline struct wlf_clipboard *wlf_wl_clipboard_get_base(
		struct wlf_wl_clipboard *wl_clipboard) {
	return &wl_clipboard->base;
}

#endif // CLIPBOARD_WLF_WL_CLIPBOARD_H
