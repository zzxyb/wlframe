/**
 * @file        wlf_zxdg_decoration_manager_v1.h
 * @brief       Wayland zxdg_decoration_manager_v1 protocol wrapper for wlframe.
 * @details     Implements the xdg-decoration-unstable-v1 protocol, which allows
 *              a compositor to offer server-side window decorations (title bar,
 *              borders, resize handles) for xdg_toplevel surfaces.
 *
 *              Usage:
 *                1. Bind wlf_zxdg_decoration_manager_v1 from the registry.
 *                2. For each xdg_toplevel, call
 *                   wlf_zxdg_decoration_manager_v1_get_toplevel_decoration()
 *                   to obtain a per-toplevel wlf_zxdg_toplevel_decoration_v1.
 *                3. Optionally call wlf_zxdg_toplevel_decoration_v1_set_mode()
 *                   to express a preference (client-side or server-side).
 *                   Call wlf_zxdg_toplevel_decoration_v1_unset_mode() to
 *                   leave the decision to the compositor.
 *                4. Listen to the configure signal to react to the compositor's
 *                   decision, then ack_configure on the xdg_surface.
 *                5. Destroy the per-toplevel object before the xdg_toplevel.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_ZXDG_DECORATION_MANAGER_V1_H
#define WAYLAND_WLF_ZXDG_DECORATION_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct xdg_toplevel;
struct zxdg_decoration_manager_v1;
struct zxdg_toplevel_decoration_v1;

/**
 * @brief Decoration mode values mirroring zxdg_toplevel_decoration_v1.mode.
 */
enum wlf_decoration_mode {
	WLF_DECORATION_MODE_CLIENT_SIDE = 1, /**< Client draws its own decorations */
	WLF_DECORATION_MODE_SERVER_SIDE = 2, /**< Compositor draws decorations      */
};

/**
 * @brief Wrapper around a bound zxdg_decoration_manager_v1 global.
 *
 * Obtain via wlf_zxdg_decoration_manager_v1_create().  Emits a destroy
 * signal before teardown.
 */
struct wlf_zxdg_decoration_manager_v1 {
	struct zxdg_decoration_manager_v1 *base; /**< Underlying protocol object */
	uint32_t version;

	struct {
		struct wlf_signal destroy; /**< Emitted just before object is freed */
	} events;
};

/**
 * @brief Per-toplevel decoration negotiation object.
 *
 * Created by wlf_zxdg_decoration_manager_v1_get_toplevel_decoration().
 * The caller owns this object and must destroy it before the associated
 * xdg_toplevel is destroyed.
 *
 * The mode field is updated each time the configure signal fires and reflects
 * the compositor's current decision.
 */
struct wlf_zxdg_toplevel_decoration_v1 {
	struct zxdg_toplevel_decoration_v1 *base; /**< Underlying protocol object */
	uint32_t version;
	/**
	 * Compositor-chosen decoration mode.
	 * Set to 0 (invalid) until the first configure event arrives.
	 */
	enum wlf_decoration_mode mode;

	struct {
		/**
		 * Emitted when the compositor sends a configure event.
		 * The listener receives a pointer to the
		 * wlf_zxdg_toplevel_decoration_v1; read the mode field for the
		 * negotiated mode.  The client must ack_configure on the
		 * xdg_surface.
		 */
		struct wlf_signal configure;

		struct wlf_signal destroy; /**< Emitted just before object is freed */
	} events;
};

/**
 * @brief Bind to the zxdg_decoration_manager_v1 global from the registry.
 *
 * @param wl_registry  Wayland registry to bind from.
 * @param name         Global name (ID) from the registry event.
 * @param version      Advertised version; clamped to the client maximum.
 * @return Newly allocated manager, or NULL on failure.
 */
struct wlf_zxdg_decoration_manager_v1 *wlf_zxdg_decoration_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 *
 * Emits the destroy signal, sends the protocol destroy request and frees the
 * wrapper.  Per-toplevel decoration objects are unaffected and must be
 * destroyed separately before their xdg_toplevels.
 *
 * @param manager  Manager to destroy; silently ignored when NULL.
 */
void wlf_zxdg_decoration_manager_v1_destroy(
	struct wlf_zxdg_decoration_manager_v1 *manager);

/**
 * @brief Create a per-toplevel decoration object.
 *
 * Associates @p toplevel with a new zxdg_toplevel_decoration_v1 via the
 * manager.  The compositor raises an already_constructed protocol error if a
 * decoration object already exists for this toplevel.
 *
 * @param manager   Bound manager.
 * @param toplevel  xdg_toplevel to associate with.
 * @return Newly allocated per-toplevel object, or NULL on failure.
 */
struct wlf_zxdg_toplevel_decoration_v1 *
wlf_zxdg_decoration_manager_v1_get_toplevel_decoration(
	struct wlf_zxdg_decoration_manager_v1 *manager,
	struct xdg_toplevel *toplevel);

/**
 * @brief Express a preferred decoration mode.
 *
 * Informs the compositor of the client's preference.  The compositor may
 * ignore it and enforce a different mode via the configure event.
 * The change is not double-buffered; it takes effect immediately at the
 * protocol level, but the compositor may batch it with the next configure.
 *
 * @param decoration  Per-toplevel decoration object.
 * @param mode        Preferred mode (client-side or server-side).
 */
void wlf_zxdg_toplevel_decoration_v1_set_mode(
	struct wlf_zxdg_toplevel_decoration_v1 *decoration,
	enum wlf_decoration_mode mode);

/**
 * @brief Unset any preferred decoration mode, leaving the choice to the
 *        compositor.
 *
 * @param decoration  Per-toplevel decoration object.
 */
void wlf_zxdg_toplevel_decoration_v1_unset_mode(
	struct wlf_zxdg_toplevel_decoration_v1 *decoration);

/**
 * @brief Destroy the per-toplevel decoration object.
 *
 * Emits the destroy signal, sends the protocol destroy request (reverting to
 * client-side decoration at the next commit) and frees the wrapper.
 *
 * @param decoration  Object to destroy; silently ignored when NULL.
 */
void wlf_zxdg_toplevel_decoration_v1_destroy(
	struct wlf_zxdg_toplevel_decoration_v1 *decoration);

#endif /* WAYLAND_WLF_ZXDG_DECORATION_MANAGER_V1_H */
