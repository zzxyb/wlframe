/**
 * @file        wlf_xdg_activation_v1.h
 * @brief       Wayland xdg_activation_v1 protocol wrapper for wlframe.
 * @details     Implements the staging xdg-activation-v1 protocol, which
 *              provides a token-based mechanism to request that the compositor
 *              activates (gives focus to) a particular surface.
 *
 *              Usage:
 *                1. Bind wlf_xdg_activation_v1 from the registry.
 *                2. Call wlf_xdg_activation_v1_get_activation_token() to
 *                   create a token request.
 *                3. Optionally configure the token with set_serial,
 *                   set_app_id, and set_surface.
 *                4. Call wlf_xdg_activation_token_v1_commit() and wait for
 *                   the events.done signal, which carries the token string.
 *                5. Pass the token string to the target application (e.g.
 *                   via IPC).
 *                6. The target application calls
 *                   wlf_xdg_activation_v1_activate() with the token and its
 *                   surface.
 *                7. Destroy the token object after commit.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_XDG_ACTIVATION_V1_H
#define WAYLAND_WLF_XDG_ACTIVATION_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>
#include <stdint.h>

struct wl_registry;
struct wl_seat;
struct wl_surface;
struct xdg_activation_v1;
struct xdg_activation_token_v1;

/**
 * @brief Wrapper around a bound xdg_activation_v1 global.
 */
struct wlf_xdg_activation_v1 {
	struct xdg_activation_v1 *base;
	uint32_t version;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Activation token request object.
 *
 * Created by wlf_xdg_activation_v1_get_activation_token().  The caller
 * owns and must destroy this object after the done event fires.
 *
 * The @c events.done signal is emitted when the compositor provides the
 * token string.  The signal data is a @c const @c char * containing the
 * token (valid only during signal emission; callers must copy it).
 */
struct wlf_xdg_activation_token_v1 {
	struct xdg_activation_token_v1 *base;
	uint32_t version;
	bool committed;
	bool done;

	struct {
		/** Emitted with @c const @c char *token when compositor responds. */
		struct wlf_signal done;
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the xdg_activation_v1 global from the registry.
 */
struct wlf_xdg_activation_v1 *wlf_xdg_activation_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the activation manager and free its resources.
 */
void wlf_xdg_activation_v1_destroy(struct wlf_xdg_activation_v1 *activation);

/**
 * @brief Create a new activation token request.
 *
 * @param activation  Bound activation manager.
 * @return A new wlf_xdg_activation_token_v1, or NULL on failure.
 */
struct wlf_xdg_activation_token_v1 *wlf_xdg_activation_v1_get_activation_token(
	struct wlf_xdg_activation_v1 *activation);

/**
 * @brief Activate a surface using a previously obtained token string.
 *
 * @param activation  Bound activation manager.
 * @param token       The activation token string received from done event.
 * @param surface     The surface to activate.
 */
void wlf_xdg_activation_v1_activate(struct wlf_xdg_activation_v1 *activation,
	const char *token, struct wl_surface *surface);

/**
 * @brief Provide the event serial and seat for the token request.
 *
 * Must be called before wlf_xdg_activation_token_v1_commit().
 *
 * @param token   Token request object.
 * @param serial  The serial of the input event that triggered the request.
 * @param seat    The seat where the input event occurred.
 */
void wlf_xdg_activation_token_v1_set_serial(
	struct wlf_xdg_activation_token_v1 *token, uint32_t serial,
	struct wl_seat *seat);

/**
 * @brief Set the application ID for the token request.
 *
 * @param token   Token request object.
 * @param app_id  Application ID of the requesting client.
 */
void wlf_xdg_activation_token_v1_set_app_id(
	struct wlf_xdg_activation_token_v1 *token, const char *app_id);

/**
 * @brief Set the surface associated with the token request.
 *
 * @param token    Token request object.
 * @param surface  The requesting client's current surface.
 */
void wlf_xdg_activation_token_v1_set_surface(
	struct wlf_xdg_activation_token_v1 *token, struct wl_surface *surface);

/**
 * @brief Commit the token request and wait for events.done.
 *
 * @param token  Token request object.
 */
void wlf_xdg_activation_token_v1_commit(
	struct wlf_xdg_activation_token_v1 *token);

/**
 * @brief Destroy the activation token object and free its resources.
 */
void wlf_xdg_activation_token_v1_destroy(
	struct wlf_xdg_activation_token_v1 *token);

#endif /* WAYLAND_WLF_XDG_ACTIVATION_V1_H */
