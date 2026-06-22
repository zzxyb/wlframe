/**
 * @file        wlf_xdg_wm_dialog_v1.h
 * @brief       Wayland xdg_wm_dialog_v1 protocol wrapper for wlframe.
 * @details     Implements the staging xdg-dialog-v1 protocol, which allows
 *              clients to hint to the compositor that a toplevel is a modal
 *              or non-modal dialog.
 *
 *              Usage:
 *                1. Bind wlf_xdg_wm_dialog_v1 from the registry.
 *                2. For each dialog toplevel, call
 *                   wlf_xdg_wm_dialog_v1_get_xdg_dialog() to obtain a
 *                   per-toplevel wlf_xdg_dialog_v1.
 *                3. Call wlf_xdg_dialog_v1_set_modal() or
 *                   wlf_xdg_dialog_v1_unset_modal() to update modal state.
 *                4. Destroy the dialog object before or together with the
 *                   toplevel.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_XDG_WM_DIALOG_V1_H
#define WAYLAND_WLF_XDG_WM_DIALOG_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>
#include <stdbool.h>

struct wl_registry;
struct xdg_toplevel;
struct xdg_wm_dialog_v1;
struct xdg_dialog_v1;
struct wlf_xdg_toplevel;

/**
 * @brief Wrapper around a bound xdg_wm_dialog_v1 global.
 *
 * The manager is bound from a registry global and is used to create per
 * xdg_toplevel dialog helper objects.
 */
struct wlf_xdg_wm_dialog_v1 {
	struct xdg_wm_dialog_v1 *base; /**< Underlying protocol object. */
	uint32_t version;              /**< Negotiated bind version. */

	struct {
		/** Emitted before the manager is destroyed and freed. */
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Per-toplevel dialog object.
 *
 * Created by wlf_xdg_wm_dialog_v1_get_xdg_dialog().  The caller owns
 * and must destroy this object.
 */
struct wlf_xdg_dialog_v1 {
	struct xdg_dialog_v1 *base; /**< Underlying protocol object. */
	uint32_t version;          /**< Protocol object version. */
	/**
	 * Last modal hint sent by this wrapper.
	 *
	 * This is local client state only.  The xdg-dialog-v1 protocol does not
	 * send events confirming compositor policy for modal dialogs.
	 */
	bool is_modal;

	struct {
		/** Emitted before the dialog object is destroyed and freed. */
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the xdg_wm_dialog_v1 global from the registry.
 *
 * The advertised version is clamped to the maximum version supported by the
 * generated client protocol headers.
 *
 * @param wl_registry Wayland registry that announced the global.
 * @param name Global name from the registry event.
 * @param version Advertised global version.
 * @return A new manager wrapper, or NULL on allocation/bind failure.
 */
struct wlf_xdg_wm_dialog_v1 *wlf_xdg_wm_dialog_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the dialog manager and free its resources.
 *
 * Destroying the manager does not affect xdg_dialog_v1 objects created from
 * it.  Per-toplevel dialog objects must still be destroyed separately.
 *
 * @param manager Manager to destroy; ignored when NULL.
 */
void wlf_xdg_wm_dialog_v1_destroy(struct wlf_xdg_wm_dialog_v1 *manager);

/**
 * @brief Create a per-toplevel dialog object.
 *
 * @param manager   Bound manager.
 * @param toplevel  The xdg_toplevel to associate with.
 * @return A new wlf_xdg_dialog_v1, or NULL on failure.
 *
 * @note The compositor raises a protocol error if another xdg_dialog_v1
 * already exists for @p toplevel.
 */
struct wlf_xdg_dialog_v1 *wlf_xdg_wm_dialog_v1_get_xdg_dialog(
	struct wlf_xdg_wm_dialog_v1 *manager,
	struct xdg_toplevel *toplevel);

/**
 * @brief Create a per-toplevel dialog object from a wlf_xdg_toplevel wrapper.
 *
 * This is a convenience helper for wlframe xdg-window code.  It is equivalent
 * to calling wlf_xdg_wm_dialog_v1_get_xdg_dialog(manager, toplevel->base).
 *
 * @param manager Bound manager.
 * @param toplevel wlframe xdg_toplevel wrapper to associate with.
 * @return A new wlf_xdg_dialog_v1, or NULL on failure.
 */
struct wlf_xdg_dialog_v1 *wlf_xdg_wm_dialog_v1_get_dialog_for_toplevel(
	struct wlf_xdg_wm_dialog_v1 *manager,
	struct wlf_xdg_toplevel *toplevel);

/**
 * @brief Mark the toplevel as a modal dialog.
 *
 * The compositor may use this hint for placement, focus, or presentation
 * policy.  The client remains responsible for filtering or blocking parent
 * window interaction according to its own modal behavior.
 *
 * @param dialog Dialog wrapper to update.
 */
void wlf_xdg_dialog_v1_set_modal(struct wlf_xdg_dialog_v1 *dialog);

/**
 * @brief Remove the modal hint from the toplevel.
 *
 * @param dialog Dialog wrapper to update.
 */
void wlf_xdg_dialog_v1_unset_modal(struct wlf_xdg_dialog_v1 *dialog);

/**
 * @brief Destroy the per-toplevel dialog object.
 *
 * If destroyed before the associated xdg_toplevel, the compositor should
 * unapply dialog-specific effects.  If the xdg_toplevel was already destroyed,
 * the dialog protocol object is inert and can still be destroyed safely.
 *
 * @param dialog Dialog object to destroy; ignored when NULL.
 */
void wlf_xdg_dialog_v1_destroy(struct wlf_xdg_dialog_v1 *dialog);

#endif /* WAYLAND_WLF_XDG_WM_DIALOG_V1_H */
