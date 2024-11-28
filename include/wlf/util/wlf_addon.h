#ifndef WLF_ADDON_H
#define WLF_ADDON_H

#include "wlf/util/wlf_double_list.h"
#include "wlf/util/wlf_signal.h"

/**
 * @brief A structure representing a set of addons
 */
struct wlf_addon_set {
	struct wlf_double_list addons;  /**< List of addons in the addon set */
};

struct wlf_addon;

/**
 * @brief A structure representing an addon interface
 */
struct wlf_addon_interface {
	const char *name;  /**< Name of the addon interface */
	void (*destroy)(struct wlf_addon *addon);  /**< Function to destroy the addon */
};

/**
 * @brief A structure representing an addon
 */
struct wlf_addon {
	const struct wlf_addon_interface *impl;  /**< Pointer to the addon interface implementation */

	const void *owner;  /**< Owner of the addon */
	struct wlf_double_list link;  /**< Link for the addon in a list */
};

/**
 * @brief Initializes the addon set
 * @param set Pointer to the addon set to initialize
 */
void wlf_addon_set_init(struct wlf_addon_set *set);

/**
 * @brief Cleans up the addon set
 * @param set Pointer to the addon set to finish
 */
void wlf_addon_set_finish(struct wlf_addon_set *set);

/**
 * @brief Initializes an addon
 * @param addon Pointer to the addon to initialize
 * @param set Pointer to the addon set to which the addon belongs
 * @param owner Pointer to the owner of the addon
 * @param impl Pointer to the addon interface implementation
 */
void wlf_addon_init(struct wlf_addon *addon, struct wlf_addon_set *set,
	const void *owner, const struct wlf_addon_interface *impl);

/**
 * @brief Cleans up an addon
 * @param addon Pointer to the addon to finish
 */
void wlf_addon_finish(struct wlf_addon *addon);

/**
 * @brief Finds an addon in the addon set
 * @param set Pointer to the addon set to search
 * @param owner Pointer to the owner of the addon
 * @param impl Pointer to the addon interface implementation
 * @return Pointer to the found addon, or NULL if not found
 */
struct wlf_addon *wlf_addon_find(struct wlf_addon_set *set, const void *owner,
	const struct wlf_addon_interface *impl);

#endif
