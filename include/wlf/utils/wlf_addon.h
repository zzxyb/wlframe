/**
 * @file        wlf_addon.h
 * @brief       Addon utility for attaching extensible data to wlframe objects.
 * @details     This file defines a lightweight addon mechanism.
 *              Addons can be attached to an owner object via an addon set,
 *              queried by owner/implementation pair, and destroyed with
 *              custom cleanup callbacks.
 * @author      YaoBing Xiao
 * @date        2026-03-16
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-16, initial version\n
 */

#ifndef UTILS_WLF_ADDON_H
#define UTILS_WLF_ADDON_H

#include "wlf/utils/wlf_linked_list.h"

/**
 * @brief A container that stores addons for an object family.
 */
struct wlf_addon_set {
	struct wlf_linked_list addons; /**< List of registered addons */
};

struct wlf_addon;

/**
 * @brief Addon implementation descriptor.
 *
 * Describes addon type identity and destruction behavior.
 */
struct wlf_addon_impl {
	const char *name; /**< Unique implementation name */
	void (*destroy)(struct wlf_addon *addon); /**< Destroy callback invoked during addon set finish */
};

/**
 * @brief A single addon instance attached to an owner object.
 */
struct wlf_addon {
	const struct wlf_addon_impl *impl; /**< Implementation descriptor for this addon */

	const void *owner; /**< Owner object associated with this addon */
	struct wlf_linked_list link; /**< List node for linking into wlf_addon_set */
};

/**
 * @brief Initializes an empty addon set.
 *
 * @param set Addon set to initialize.
 */
void wlf_addon_set_init(struct wlf_addon_set *set);

/**
 * @brief Finishes an addon set and destroys remaining addons.
 *
 * @param set Addon set to finish.
 */
void wlf_addon_set_finish(struct wlf_addon_set *set);

/**
 * @brief Initializes and registers an addon in a set.
 *
 * @param addon Addon instance to initialize.
 * @param set Addon set that receives the addon.
 * @param owner Owner pointer used to identify addon association.
 * @param impl Addon implementation descriptor.
 */
void wlf_addon_init(struct wlf_addon *addon, struct wlf_addon_set *set,
	const void *owner, const struct wlf_addon_impl *impl);

/**
 * @brief Finishes and detaches an addon.
 *
 * @param addon Addon instance to finish.
 */
void wlf_addon_finish(struct wlf_addon *addon);

/**
 * @brief Finds an addon by owner and implementation descriptor.
 *
 * @param set Addon set to search in.
 * @param owner Owner pointer to match.
 * @param impl Addon implementation descriptor to match.
 * @return Matching addon, or NULL if not found.
 */
struct wlf_addon *wlf_addon_find(struct wlf_addon_set *set, const void *owner,
	const struct wlf_addon_impl *impl);

#endif // UTILS_WLF_ADDON_H
