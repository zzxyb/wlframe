/**
 * @file        wlf_wl_data_device.h
 * @brief       Wayland wl_data_device wrapper for wlframe.
 * @details     Wraps a wl_data_device obtained from wl_data_device_manager,
 *              exposing selection and drag-and-drop events.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_DATA_DEVICE_H
#define WAYLAND_WLF_WL_DATA_DEVICE_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_data_device;
struct wl_surface;
struct wlf_wl_data_device_manager;
struct wlf_wl_data_offer;
struct wlf_wl_data_source;
struct wlf_wl_seat;

/**
 * @brief Payload for the enter event (DnD drag entered this surface).
 */
struct wlf_wl_data_device_enter_event {
	struct wlf_wl_data_device *device;
	uint32_t serial;
	struct wl_surface *surface;
	double x, y;                        /**< Surface-local position. */
	struct wlf_wl_data_offer *offer;    /**< Data offer, or NULL. */
};

/**
 * @brief Payload for the motion event.
 */
struct wlf_wl_data_device_motion_event {
	struct wlf_wl_data_device *device;
	uint32_t time_msec;
	double x, y; /**< Surface-local position. */
};

/**
 * @brief Payload for the selection event.
 */
struct wlf_wl_data_device_selection_event {
	struct wlf_wl_data_device *device;
	struct wlf_wl_data_offer *offer; /**< New selection offer, or NULL. */
};

/**
 * @brief Wayland data device wrapper.
 */
struct wlf_wl_data_device {
	struct wl_data_device *wl_data_device; /**< Underlying Wayland object. */

	struct wlf_wl_data_offer *drag_offer;  /**< Current DnD offer, or NULL. */
	struct wlf_wl_data_offer *selection_offer; /**< Current selection, or NULL. */

	struct {
		struct wlf_signal destroy;    /**< Emitted before destruction. */
		/** Emitted when an offer object is created. Payload: wlf_wl_data_offer. */
		struct wlf_signal data_offer;
		/** Emitted when a DnD drag enters the client surface. Payload: wlf_wl_data_device_enter_event. */
		struct wlf_signal enter;
		/** Emitted when the drag leaves the client surface. Payload: wlf_wl_data_device. */
		struct wlf_signal leave;
		/** Emitted when the drag moves. Payload: wlf_wl_data_device_motion_event. */
		struct wlf_signal motion;
		/** Emitted when the user drops. Payload: wlf_wl_data_device. */
		struct wlf_signal drop;
		/** Emitted when the selection changes. Payload: wlf_wl_data_device_selection_event. */
		struct wlf_signal selection;
	} events;
};

/**
 * @brief Creates a data device for the given seat from the manager.
 *
 * @param manager Data device manager.
 * @param seat    Seat to associate the device with.
 * @return Newly allocated wlf_wl_data_device, or NULL on failure.
 */
struct wlf_wl_data_device *wlf_wl_data_device_create(
	struct wlf_wl_data_device_manager *manager,
	struct wlf_wl_seat *seat);

/**
 * @brief Destroys a wlf_wl_data_device. Passing NULL is a no-op.
 */
void wlf_wl_data_device_destroy(struct wlf_wl_data_device *device);

/**
 * @brief Starts a drag-and-drop operation.
 *
 * @param device  Data device.
 * @param source  Data source providing drag data, or NULL.
 * @param origin  Surface where the drag originates.
 * @param icon    Drag icon surface, or NULL.
 * @param serial  Serial from the implicit grab event.
 */
void wlf_wl_data_device_start_drag(struct wlf_wl_data_device *device,
	struct wlf_wl_data_source *source,
	struct wl_surface *origin,
	struct wl_surface *icon,
	uint32_t serial);

/**
 * @brief Sets the clipboard selection.
 *
 * @param device  Data device.
 * @param source  Data source, or NULL to clear the selection.
 * @param serial  Serial from the event that triggered the selection.
 */
void wlf_wl_data_device_set_selection(struct wlf_wl_data_device *device,
	struct wlf_wl_data_source *source, uint32_t serial);

#endif /* WAYLAND_WLF_WL_DATA_DEVICE_H */
