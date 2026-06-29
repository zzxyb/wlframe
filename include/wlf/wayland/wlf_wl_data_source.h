/**
 * @file        wlf_wl_data_source.h
 * @brief       Wayland wl_data_source wrapper for wlframe.
 * @details     Wraps a wl_data_source created by the client for selection or
 *              drag-and-drop, allowing MIME type advertisement and transfer.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_DATA_SOURCE_H
#define WAYLAND_WLF_WL_DATA_SOURCE_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_data_source;
struct wlf_wl_data_device_manager;

/**
 * @brief Payload for the target event.
 */
struct wlf_wl_data_source_target_event {
	struct wlf_wl_data_source *source;
	const char *mime_type; /**< Accepted MIME type, or NULL if rejected. */
};

/**
 * @brief Payload for the send event.
 */
struct wlf_wl_data_source_send_event {
	struct wlf_wl_data_source *source;
	const char *mime_type;
	int fd; /**< File descriptor to write data into. */
};

/**
 * @brief Payload for the action event (v3+).
 */
struct wlf_wl_data_source_action_event {
	struct wlf_wl_data_source *source;
	uint32_t dnd_action; /**< Action selected by the compositor. */
};

/**
 * @brief Wayland data source wrapper.
 */
struct wlf_wl_data_source {
	struct wl_data_source *wl_data_source; /**< Underlying Wayland object. */

	struct {
		struct wlf_signal destroy;           /**< Emitted before destruction. */
		/** Emitted when a destination accepts a MIME type. Payload: wlf_wl_data_source_target_event. */
		struct wlf_signal target;
		/** Emitted when the destination requests data transfer. Payload: wlf_wl_data_source_send_event. */
		struct wlf_signal send;
		/** Emitted when the source is no longer valid. Payload: wlf_wl_data_source. */
		struct wlf_signal cancelled;
		/** Emitted when the user drops (DnD v3+). Payload: wlf_wl_data_source. */
		struct wlf_signal dnd_drop_performed;
		/** Emitted when the DnD operation is finished (DnD v3+). Payload: wlf_wl_data_source. */
		struct wlf_signal dnd_finished;
		/** Emitted when compositor selects an action (v3+). Payload: wlf_wl_data_source_action_event. */
		struct wlf_signal action;
	} events;
};

/**
 * @brief Creates a new data source from the data device manager.
 *
 * @param manager Data device manager.
 * @return Newly allocated wlf_wl_data_source, or NULL on failure.
 */
struct wlf_wl_data_source *wlf_wl_data_source_create(
	struct wlf_wl_data_device_manager *manager);

/**
 * @brief Destroys a wlf_wl_data_source. Passing NULL is a no-op.
 */
void wlf_wl_data_source_destroy(struct wlf_wl_data_source *source);

/**
 * @brief Advertises a MIME type that this source can provide.
 */
void wlf_wl_data_source_offer(struct wlf_wl_data_source *source,
	const char *mime_type);

/**
 * @brief Sets supported DnD actions on this source (v3+).
 */
void wlf_wl_data_source_set_actions(struct wlf_wl_data_source *source,
	uint32_t dnd_actions);

#endif /* WAYLAND_WLF_WL_DATA_SOURCE_H */
