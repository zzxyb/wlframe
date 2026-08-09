/**
 * @file        wlf_wl_data_offer.h
 * @brief       Wayland wl_data_offer wrapper for wlframe.
 * @details     Wraps a wl_data_offer (created by the compositor for selection
 *              or drag-and-drop), exposes offered MIME types, and provides
 *              accept/receive/finish/set_actions helpers.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_DATA_OFFER_H
#define WAYLAND_WLF_WL_DATA_OFFER_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_data_offer;

/**
 * @brief Payload for the offer event (a new MIME type is advertised).
 */
struct wlf_wl_data_offer_mime_event {
	struct wlf_wl_data_offer *offer;
	const char *mime_type;
};

/**
 * @brief Wayland data offer wrapper.
 */
struct wlf_wl_data_offer {
	struct wl_data_offer *wl_data_offer; /**< Underlying Wayland object. */

	uint32_t source_actions; /**< Bitmask of actions offered by the source. */
	uint32_t dnd_action;     /**< Action selected by the compositor. */

	struct {
		struct wlf_signal destroy;        /**< Emitted before destruction. */
		/** Emitted for each MIME type the source can provide. Payload: wlf_wl_data_offer_mime_event. */
		struct wlf_signal offer;
		/** Emitted when source actions are announced (DnD v3+). Payload: wlf_wl_data_offer. */
		struct wlf_signal source_actions;
		/** Emitted when the compositor selects an action (DnD v3+). Payload: wlf_wl_data_offer. */
		struct wlf_signal action;
	} events;
};

/**
 * @brief Wraps an existing wl_data_offer.
 *
 * Called internally by wlf_wl_data_device when a data_offer event is received.
 */
struct wlf_wl_data_offer *wlf_wl_data_offer_wrap(
	struct wl_data_offer *wl_data_offer);

/**
 * @brief Destroys a wlf_wl_data_offer. Passing NULL is a no-op.
 */
void wlf_wl_data_offer_destroy(struct wlf_wl_data_offer *offer);

/**
 * @brief Accepts the given MIME type from this offer.
 *
 * @param offer     Data offer.
 * @param serial    Serial from the enter event.
 * @param mime_type MIME type to accept, or NULL to reject.
 */
void wlf_wl_data_offer_accept(struct wlf_wl_data_offer *offer,
	uint32_t serial, const char *mime_type);

/**
 * @brief Requests that data is transferred for the given MIME type.
 *
 * @param offer     Data offer.
 * @param mime_type MIME type to receive.
 * @param fd        File descriptor to write data into.
 */
void wlf_wl_data_offer_receive(struct wlf_wl_data_offer *offer,
	const char *mime_type, int fd);

/**
 * @brief Finalises a drag-and-drop operation (v3+).
 */
void wlf_wl_data_offer_finish(struct wlf_wl_data_offer *offer);

/**
 * @brief Sets supported DnD actions on this offer (v3+).
 *
 * @param offer            Data offer.
 * @param dnd_actions      Bitmask of actions the destination supports.
 * @param preferred_action Preferred action bitmask.
 */
void wlf_wl_data_offer_set_actions(struct wlf_wl_data_offer *offer,
	uint32_t dnd_actions, uint32_t preferred_action);

#endif /* WAYLAND_WLF_WL_DATA_OFFER_H */
