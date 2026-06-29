/**
 * @file        wlf_wl_data_device_manager.h
 * @brief       Wayland wl_data_device_manager global wrapper for wlframe.
 * @details     Wraps wl_data_device_manager bound from the Wayland registry,
 *              providing data source and data device creation.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_DATA_DEVICE_MANAGER_H
#define WAYLAND_WLF_WL_DATA_DEVICE_MANAGER_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_data_device_manager;
struct wl_registry;
struct wlf_wl_data_device;
struct wlf_wl_data_source;
struct wlf_wl_seat;

/**
 * @brief Wayland data device manager global.
 */
struct wlf_wl_data_device_manager {
	struct wl_data_device_manager *wl_data_device_manager;

	struct {
		struct wlf_signal destroy; /**< Emitted before destruction. */
	} events;
};

/**
 * @brief Binds wl_data_device_manager from the Wayland registry.
 *
 * @param wl_registry Wayland registry.
 * @param name        Global name.
 * @param version     Advertised version.
 * @return Newly allocated wlf_wl_data_device_manager, or NULL on failure.
 */
struct wlf_wl_data_device_manager *wlf_wl_data_device_manager_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroys a wlf_wl_data_device_manager. Passing NULL is a no-op.
 */
void wlf_wl_data_device_manager_destroy(
	struct wlf_wl_data_device_manager *manager);

/**
 * @brief Creates a new data source.
 *
 * @param manager Data device manager.
 * @return Newly allocated wlf_wl_data_source, or NULL on failure.
 */
struct wlf_wl_data_source *wlf_wl_data_device_manager_create_data_source(
	struct wlf_wl_data_device_manager *manager);

/**
 * @brief Creates a data device for the given seat.
 *
 * @param manager Data device manager.
 * @param seat    Seat to associate the device with.
 * @return Newly allocated wlf_wl_data_device, or NULL on failure.
 */
struct wlf_wl_data_device *wlf_wl_data_device_manager_get_data_device(
	struct wlf_wl_data_device_manager *manager,
	struct wlf_wl_seat *seat);

#endif /* WAYLAND_WLF_WL_DATA_DEVICE_MANAGER_H */
