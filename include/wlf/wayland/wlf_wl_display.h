/**
 * @file        wlf_wl_display.h
 * @brief       Wayland display and interface management for wlframe.
 * @details     This file provides structures and functions for managing Wayland display,
 *              registry, and global interfaces, including creation, destruction, registry
 *              initialization, interface lookup, and version checking. It also provides
 *              signals for global add/remove and destroy events.
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

struct wl_display;
struct wl_registry;

/**
 * @brief Structure representing a Wayland global interface.
 */
struct wlf_wl_interface {
	struct wlf_linked_list link;   /**< Linked list node for interface list */
	uint32_t name;                 /**< Global name (Wayland registry id) */
	char *interface;               /**< Interface name string */
	uint32_t version;              /**< Interface version */
};

/**
 * @brief Structure representing a Wayland display and its registry/interfaces.
 */
struct wlf_wl_display {
	struct wl_display *base;         /**< Wayland display pointer */
	struct wl_registry *registry;       /**< Wayland registry pointer */

	struct wlf_linked_list interfaces;  /**< List of global interfaces */

	struct {
		struct wlf_signal destroy;      /**< Signal emitted when display is destroyed */
		struct wlf_signal global_add;   /**< Signal emitted when a global is added */
		struct wlf_signal global_remove;/**< Signal emitted when a global is removed */
	} events;
};

/**
 * @brief Create a new wlf_wl_display object.
 * @return Pointer to the newly allocated wlf_wl_display.
 */
struct wlf_wl_display *wlf_wl_display_create(void);

/**
 * @brief Initialize the registry for a Wayland display.
 * @param display Pointer to the wlf_wl_display object.
 * @return true on success, false otherwise.
 */
bool wlf_wl_display_init_registry(struct wlf_wl_display *display);

/**
 * @brief Destroy a wlf_wl_display object and free its resources.
 * @param display Pointer to the wlf_wl_display object.
 */
void wlf_wl_display_destroy(struct wlf_wl_display *display);

/**
 * @brief Get a registry interface by interface name.
 * @param display Pointer to the wlf_wl_display object.
 * @param interface Name of the interface to search for.
 * @return Pointer to the found wlf_wl_interface, or NULL if not found.
 */
struct wlf_wl_interface *wlf_wl_display_get_registry_from_interface(
	const struct wlf_wl_display *display, const char *interface);

/**
 * @brief Create a new wlf_wl_interface object.
 * @param display Pointer to the wlf_wl_display object.
 * @param interface Name of the interface.
 * @param version Interface version.
 * @param name Global name (Wayland registry id).
 * @return Pointer to the newly created wlf_wl_interface.
 */
struct wlf_wl_interface *wlf_wl_interface_create(struct wlf_wl_display *display,
	const char *interface, uint32_t version, uint32_t name);

/**
 * @brief Destroy a wlf_wl_interface (registry) object.
 * @param registry Pointer to the wlf_wl_interface object.
 */
void wlf_wl_registry_destroy(struct wlf_wl_interface *registry);

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
