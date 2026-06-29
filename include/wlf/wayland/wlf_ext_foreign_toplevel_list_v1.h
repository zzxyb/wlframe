/**
 * @file        wlf_ext_foreign_toplevel_list_v1.h
 * @brief       Wayland ext_foreign_toplevel_list_v1 protocol wrapper for
 *              wlframe.
 * @details     Wraps the staging ext-foreign-toplevel-list-v1 protocol, which
 *              lets a client enumerate all top-level windows currently managed
 *              by the compositor, even those belonging to other clients.
 *
 *              Two interfaces are wrapped:
 *
 *              - wlf_ext_foreign_toplevel_list_v1  — the global manager.
 *                The compositor emits a @c toplevel event for each existing
 *                top-level and for every new one created afterwards.  Call
 *                @c stop to stop receiving new events without destroying the
 *                existing handles; the compositor will emit @c finished when
 *                it has stopped.
 *
 *              - wlf_ext_foreign_toplevel_handle_v1 — represents one toplevel.
 *                The compositor delivers property events (@c title, @c app_id,
 *                @c identifier) followed by a @c done event when an atomic
 *                batch of updates is complete.  On removal the @c closed event
 *                fires; the caller must still call destroy() to free the
 *                object.
 *
 *              String fields (@c title, @c app_id, @c identifier) are updated
 *              before @c events.done is emitted, and freed when the handle is
 *              destroyed.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_EXT_FOREIGN_TOPLEVEL_LIST_V1_H
#define WAYLAND_WLF_EXT_FOREIGN_TOPLEVEL_LIST_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct ext_foreign_toplevel_list_v1;
struct ext_foreign_toplevel_handle_v1;

/* Forward declare so the list signal can carry a pointer to the handle. */
struct wlf_ext_foreign_toplevel_handle_v1;

/**
 * @brief Wrapper around a bound ext_foreign_toplevel_list_v1 global.
 */
struct wlf_ext_foreign_toplevel_list_v1 {
	struct ext_foreign_toplevel_list_v1 *base;

	struct {
		/** Emitted when a new toplevel appears (after its handle is
		 *  initialised).  Data: (struct wlf_ext_foreign_toplevel_handle_v1 *) */
		struct wlf_signal toplevel;
		/** Emitted after the compositor has stopped sending events
		 *  following a stop() call.  Data: manager itself. */
		struct wlf_signal finished;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Represents one foreign top-level window.
 *
 * Created automatically by the manager's toplevel event handler and emitted
 * via wlf_ext_foreign_toplevel_list_v1::events.toplevel.  The caller is
 * responsible for listening to the handle's events and eventually calling
 * wlf_ext_foreign_toplevel_handle_v1_destroy() on it.
 *
 * @c title, @c app_id, and @c identifier are heap-allocated strings, updated
 * before each @c done signal emission.  They may be NULL if not yet received.
 */
struct wlf_ext_foreign_toplevel_handle_v1 {
	struct ext_foreign_toplevel_handle_v1 *base;

	char *title;
	char *app_id;
	char *identifier;

	struct {
		/** Emitted when the handle's properties have been fully updated. */
		struct wlf_signal done;
		/** Emitted when the toplevel has been closed.  The caller must
		 *  still call destroy() to release resources. */
		struct wlf_signal closed;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the ext_foreign_toplevel_list_v1 global from the registry.
 */
struct wlf_ext_foreign_toplevel_list_v1 *
wlf_ext_foreign_toplevel_list_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 *
 * Does not destroy individual toplevel handles; the caller must do that
 * separately.
 */
void wlf_ext_foreign_toplevel_list_v1_destroy(
	struct wlf_ext_foreign_toplevel_list_v1 *list);

/**
 * @brief Ask the compositor to stop sending new toplevel events.
 *
 * The compositor will emit the @c finished event in response.  The manager
 * must not be destroyed before @c finished is received.
 */
void wlf_ext_foreign_toplevel_list_v1_stop(
	struct wlf_ext_foreign_toplevel_list_v1 *list);

/**
 * @brief Destroy a toplevel handle and free its resources.
 */
void wlf_ext_foreign_toplevel_handle_v1_destroy(
	struct wlf_ext_foreign_toplevel_handle_v1 *handle);

#endif /* WAYLAND_WLF_EXT_FOREIGN_TOPLEVEL_LIST_V1_H */
