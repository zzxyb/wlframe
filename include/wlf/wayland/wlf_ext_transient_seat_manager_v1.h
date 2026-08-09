/**
 * @file        wlf_ext_transient_seat_manager_v1.h
 * @brief       ext-transient-seat-v1 client wrappers.
 * @details     Provides creation and lifecycle helpers for transient seats
 *              granted by a Wayland compositor.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WLF_EXT_TRANSIENT_SEAT_MANAGER_V1_H
#define WLF_EXT_TRANSIENT_SEAT_MANAGER_V1_H

#include <stdint.h>

#include "wlf/utils/wlf_signal.h"

struct wl_registry;
struct ext_transient_seat_manager_v1;
struct ext_transient_seat_v1;

/**
 * @brief Transient seat wrapper.
 *
 * The ready and denied signals report the compositor's response to the seat
 * request.
 */
struct wlf_ext_transient_seat_v1 {
	struct ext_transient_seat_v1 *base; /**< Protocol object. */

	uint32_t global_name; /**< Registry name of the originating seat global. */

	struct {
		struct wlf_signal ready;   /**< Payload: wlf_ext_transient_seat_v1. */
		struct wlf_signal denied;  /**< Payload: wlf_ext_transient_seat_v1. */
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Transient seat manager wrapper.
 *
 * The manager creates transient seat requests for the current client.
 */
struct wlf_ext_transient_seat_manager_v1 {
	struct ext_transient_seat_manager_v1 *base; /**< Protocol object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Creates a transient seat manager wrapper.
 * @param registry Wayland registry used to bind the manager.
 * @param name Global name advertised by the registry.
 * @param version Protocol version to bind.
 * @return Newly allocated manager, or NULL on failure.
 */
struct wlf_ext_transient_seat_manager_v1 *wlf_ext_transient_seat_manager_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version);

/**
 * @brief Destroys a transient seat manager.
 * @param manager Manager to destroy.
 */
void wlf_ext_transient_seat_manager_v1_destroy(
	struct wlf_ext_transient_seat_manager_v1 *manager);

/**
 * @brief Requests a transient seat from the manager.
 * @param manager Manager receiving the request.
 * @return Newly allocated seat request, or NULL on failure.
 */
struct wlf_ext_transient_seat_v1 *wlf_ext_transient_seat_manager_v1_create_seat(
	struct wlf_ext_transient_seat_manager_v1 *manager);

/**
 * @brief Destroys a transient seat wrapper.
 * @param seat Seat request to destroy.
 */
void wlf_ext_transient_seat_v1_destroy(struct wlf_ext_transient_seat_v1 *seat);

#endif /* WLF_EXT_TRANSIENT_SEAT_MANAGER_V1_H */
