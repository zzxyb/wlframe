/**
 * @file        wlf_wp_content_type_manager_v1.h
 * @brief       Wayland wp_content_type_manager_v1 protocol wrapper for wlframe.
 * @details     Implements the staging wp_content_type_v1 protocol, which lets a
 *              client hint to the compositor what kind of content a surface
 *              displays (none, photo, video, game).  The compositor may use
 *              this to adjust hardware settings (e.g. DRM "content type"
 *              property) or rendering heuristics accordingly.
 *
 *              Usage:
 *                1. Bind wlf_wp_content_type_manager_v1 from the registry.
 *                2. For each surface, call
 *                   wlf_wp_content_type_manager_v1_get_surface_content_type()
 *                   to obtain a per-surface wlf_wp_content_type_v1.
 *                3. Call wlf_wp_content_type_v1_set_content_type() before each
 *                   wl_surface.commit to update the hint (double-buffered).
 *                4. Destroy the per-surface object before or together with the
 *                   surface.  Destroying resets the type to none.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_WP_CONTENT_TYPE_MANAGER_V1_H
#define WAYLAND_WLF_WP_CONTENT_TYPE_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_surface;
struct wp_content_type_manager_v1;
struct wp_content_type_v1;

/**
 * @brief Content type hint values mirroring wp_content_type_v1.type.
 */
enum wlf_content_type {
	WLF_CONTENT_TYPE_NONE  = 0, /**< No specific content type / unknown */
	WLF_CONTENT_TYPE_PHOTO = 1, /**< Digital still image                */
	WLF_CONTENT_TYPE_VIDEO = 2, /**< Video or animation                 */
	WLF_CONTENT_TYPE_GAME  = 3, /**< Real-time game (low-latency hint)  */
};

/**
 * @brief Wrapper around a bound wp_content_type_manager_v1 global.
 *
 * Obtain via wlf_wp_content_type_manager_v1_create().  Emits a destroy
 * signal before teardown.
 */
struct wlf_wp_content_type_manager_v1 {
	struct wp_content_type_manager_v1 *base; /**< Underlying protocol object */
	uint32_t version;

	struct {
		struct wlf_signal destroy; /**< Emitted just before object is freed */
	} events;
};

/**
 * @brief Per-surface content type hint object.
 *
 * Created by wlf_wp_content_type_manager_v1_get_surface_content_type().
 * The caller owns this object and must call wlf_wp_content_type_v1_destroy()
 * when done.  Destroying the object resets the surface's content type to
 * WLF_CONTENT_TYPE_NONE.
 */
struct wlf_wp_content_type_v1 {
	struct wp_content_type_v1 *base; /**< Underlying protocol object */
	enum wlf_content_type type;
	uint32_t version;

	struct {
		struct wlf_signal destroy; /**< Emitted just before object is freed */
	} events;
};

/**
 * @brief Bind to the wp_content_type_manager_v1 global from the registry.
 *
 * @param wl_registry  Wayland registry to bind from.
 * @param name         Global name (ID) from the registry event.
 * @param version      Advertised version; clamped to the client maximum.
 * @return Newly allocated manager, or NULL on failure.
 */
struct wlf_wp_content_type_manager_v1 *wlf_wp_content_type_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 *
 * Emits the destroy signal, sends the protocol destroy request and frees the
 * wrapper.  Per-surface content-type objects are unaffected and must be
 * destroyed separately.
 *
 * @param manager  Manager to destroy; silently ignored when NULL.
 */
void wlf_wp_content_type_manager_v1_destroy(struct wlf_wp_content_type_manager_v1 *manager);

/**
 * @brief Create a per-surface content type object.
 *
 * Associates @p surface with a new wp_content_type_v1 object via the manager.
 * The compositor raises an already_constructed protocol error if a content
 * type object already exists for this surface.
 *
 * @param manager  Bound manager.
 * @param surface  wl_surface to associate with.
 * @return Newly allocated per-surface object, or NULL on failure.
 */
struct wlf_wp_content_type_v1 *wlf_wp_content_type_manager_v1_get_surface_content_type(
	struct wlf_wp_content_type_manager_v1 *manager, struct wl_surface *surface);

/**
 * @brief Set the content type hint for the surface (double-buffered).
 *
 * Takes effect on the next wl_surface.commit.
 *
 * @param content_type  Per-surface object obtained from the manager.
 * @param type          One of the wlf_content_type enum values.
 */
void wlf_wp_content_type_v1_set_content_type(
	struct wlf_wp_content_type_v1 *content_type, enum wlf_content_type type);

/**
 * @brief Destroy the per-surface content type object.
 *
 * Emits the destroy signal, sends the protocol destroy request (resetting the
 * surface's type to none) and frees the wrapper.
 *
 * @param content_type  Object to destroy; silently ignored when NULL.
 */
void wlf_wp_content_type_v1_destroy(struct wlf_wp_content_type_v1 *content_type);

#endif /* WAYLAND_WLF_WP_CONTENT_TYPE_MANAGER_V1_H */
