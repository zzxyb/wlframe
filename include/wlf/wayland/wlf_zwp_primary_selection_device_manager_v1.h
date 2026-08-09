/**
 * @file        wlf_zwp_primary_selection_device_manager_v1.h
 * @brief       Wayland primary-selection-unstable-v1 wrapper for wlframe.
 * @details     Provides the "primary selection" clipboard — a separate
 *              selection buffer traditionally populated by mouse-button
 *              selection (X11-style).
 *
 *              Interfaces wrapped:
 *              - wlf_zwp_primary_selection_device_manager_v1 — global.
 *              - wlf_zwp_primary_selection_device_v1         — per-seat
 *                selection device.
 *              - wlf_zwp_primary_selection_offer_v1          — incoming data
 *                offer (MIME types announced by source).
 *              - wlf_zwp_primary_selection_source_v1         — outgoing data
 *                source.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_ZWP_PRIMARY_SELECTION_DEVICE_MANAGER_V1_H
#define WAYLAND_WLF_ZWP_PRIMARY_SELECTION_DEVICE_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stddef.h>
#include <stdint.h>

struct wl_registry;
struct wl_seat;
struct zwp_primary_selection_device_manager_v1;
struct zwp_primary_selection_device_v1;
struct zwp_primary_selection_offer_v1;
struct zwp_primary_selection_source_v1;

/* Forward declarations */
struct wlf_zwp_primary_selection_offer_v1;
struct wlf_zwp_primary_selection_source_v1;

/* -------------------------------------------------------------------------
 * Data offer
 * ---------------------------------------------------------------------- */

/**
 * @brief Represents an incoming primary-selection offer from another client.
 *
 * The compositor delivers MIME types via the @c offer event, which are
 * accumulated in @c mime_types.  Use @c _receive() to request the data.
 */
struct wlf_zwp_primary_selection_offer_v1 {
	struct zwp_primary_selection_offer_v1 *base;

	char **mime_types;    /**< NULL-terminated array of offered MIME types  */
	size_t n_mime_types;

	struct {
		struct wlf_signal destroy;
	} events;
};

/* -------------------------------------------------------------------------
 * Data source
 * ---------------------------------------------------------------------- */

/**
 * @brief Represents an outgoing primary-selection source.
 *
 * The client calls @c _offer() for each MIME type it can provide, then
 * listens for the @c send signal (which carries the requested MIME type and
 * a write fd) to produce the data.
 */
struct wlf_zwp_primary_selection_source_v1 {
	struct zwp_primary_selection_source_v1 *base;

	struct {
		/** Data: self — caller should write data to current.fd */
		struct wlf_signal send;
		/** Data: self — source is no longer used              */
		struct wlf_signal cancelled;
		struct wlf_signal destroy;
	} events;

	/** Populated before emitting @c send */
	const char *send_mime_type; /**< MIME type requested (owned by event)   */
	int send_fd;                /**< Write fd for the requested data         */
};

/* -------------------------------------------------------------------------
 * Device
 * ---------------------------------------------------------------------- */

/**
 * @brief Per-seat primary-selection device.
 *
 * The compositor emits @c data_offer when a new selection is available,
 * followed by @c selection which references that offer.  When the selection
 * is cleared, @c selection fires with a NULL offer.
 */
struct wlf_zwp_primary_selection_device_v1 {
	struct zwp_primary_selection_device_v1 *base;

	struct {
		/** Data: wlf_zwp_primary_selection_offer_v1 * (new offer) */
		struct wlf_signal data_offer;
		/** Data: wlf_zwp_primary_selection_offer_v1 * (or NULL)   */
		struct wlf_signal selection;
		struct wlf_signal destroy;
	} events;
};

/* -------------------------------------------------------------------------
 * Manager
 * ---------------------------------------------------------------------- */

/**
 * @brief Wrapper around the zwp_primary_selection_device_manager_v1 global.
 */
struct wlf_zwp_primary_selection_device_manager_v1 {
	struct zwp_primary_selection_device_manager_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the zwp_primary_selection_device_manager_v1 global.
 */
struct wlf_zwp_primary_selection_device_manager_v1 *
wlf_zwp_primary_selection_device_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager.
 */
void wlf_zwp_primary_selection_device_manager_v1_destroy(
	struct wlf_zwp_primary_selection_device_manager_v1 *manager);

/**
 * @brief Create a primary-selection source.
 */
struct wlf_zwp_primary_selection_source_v1 *
wlf_zwp_primary_selection_device_manager_v1_create_source(
	struct wlf_zwp_primary_selection_device_manager_v1 *manager);

/**
 * @brief Get the primary-selection device for @p seat.
 */
struct wlf_zwp_primary_selection_device_v1 *
wlf_zwp_primary_selection_device_manager_v1_get_device(
	struct wlf_zwp_primary_selection_device_manager_v1 *manager,
	struct wl_seat *seat);

/* -------------------------------------------------------------------------
 * Device API
 * ---------------------------------------------------------------------- */

/**
 * @brief Set the primary selection to @p source.
 *
 * @param device  The seat's selection device.
 * @param source  The source offering the data (NULL to clear selection).
 * @param serial  Serial from a pointer/key press event.
 */
void wlf_zwp_primary_selection_device_v1_set_selection(
	struct wlf_zwp_primary_selection_device_v1 *device,
	struct wlf_zwp_primary_selection_source_v1 *source, uint32_t serial);

/**
 * @brief Destroy a primary-selection device.
 */
void wlf_zwp_primary_selection_device_v1_destroy(
	struct wlf_zwp_primary_selection_device_v1 *device);

/* -------------------------------------------------------------------------
 * Offer API
 * ---------------------------------------------------------------------- */

/**
 * @brief Receive the data from an offer.
 *
 * @param offer      The offer to receive from.
 * @param mime_type  MIME type to request.
 * @param fd         Write end of a pipe; the compositor writes data to it.
 */
void wlf_zwp_primary_selection_offer_v1_receive(
	struct wlf_zwp_primary_selection_offer_v1 *offer,
	const char *mime_type, int fd);

/**
 * @brief Destroy a data offer.
 */
void wlf_zwp_primary_selection_offer_v1_destroy(
	struct wlf_zwp_primary_selection_offer_v1 *offer);

/* -------------------------------------------------------------------------
 * Source API
 * ---------------------------------------------------------------------- */

/**
 * @brief Announce a supported MIME type on the source.
 *
 * Must be called before setting the selection.
 */
void wlf_zwp_primary_selection_source_v1_offer(
	struct wlf_zwp_primary_selection_source_v1 *source,
	const char *mime_type);

/**
 * @brief Destroy a primary-selection source.
 */
void wlf_zwp_primary_selection_source_v1_destroy(
	struct wlf_zwp_primary_selection_source_v1 *source);

#endif /* WAYLAND_WLF_ZWP_PRIMARY_SELECTION_DEVICE_MANAGER_V1_H */
