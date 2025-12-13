/**
 * @file        wlf_wl_interface.h
 * @brief       Wayland interface management for wlframe.
 * @details     This file provides structures and functions for
 *              interface lookup, and version checking.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef WAYLAND_WLF_WL_DISPLAY_H
#define WAYLAND_WLF_WL_DISPLAY_H

#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"

#include <stdint.h>
#include <stdbool.h>

struct wlf_backend_wayland;

/**
 * @brief Structure representing a Wayland global interface.
 */
struct wlf_wl_interface {
	struct wlf_linked_list link;   /**< Linked list node for interface list */
	struct {
		struct wlf_signal destroy;      /**< Signal emitted when interface is destroyed */
	} events;

	uint32_t name;                 /**< Global name (Wayland registry id) */
	char *interface;               /**< Interface name string */
	uint32_t version;              /**< Interface version */
};

/**
 * @brief Get a registry interface by interface name.
 * @param backend Pointer to the wlf_backend_wayland object.
 * @param interface Name of the interface to search for.
 * @return Pointer to the found wlf_wl_interface, or NULL if not found.
 */
struct wlf_wl_interface *wlf_wl_backend_find_interface(
	const struct wlf_backend_wayland *backend, const char *interface);

/**
 * @brief Create a new wlf_wl_interface object.
 * @param interface Name of the interface.
 * @param version Interface version.
 * @param name Global name (Wayland registry id).
 * @return Pointer to the newly created wlf_wl_interface.
 */
struct wlf_wl_interface *wlf_wl_interface_create(const char *interface,
	uint32_t version, uint32_t name);

/**
 * @brief Destroy a wlf_wl_interface (registry) object.
 * @param interface Pointer to the wlf_wl_interface object.
 */
void wlf_wl_interface_destroy(struct wlf_wl_interface *interface);

/**
 * @brief Check if the client interface version is higher than the remote version.
 * @param interface Name of the interface.
 * @param client_version Client's version.
 * @param remote_version Remote's version.
 * @return true if client_version > remote_version, false otherwise.
 */
bool client_interface_version_is_higher(const char *interface,
	uint32_t client_version, uint32_t remote_version);

#endif // WAYLAND_WLF_WL_DISPLAY_H
