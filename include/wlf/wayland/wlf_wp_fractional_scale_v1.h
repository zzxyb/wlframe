/**
 * @file        wlf_wp_fractional_scale_v1.h
 * @brief       Wayland wp_fractional_scale_manager_v1 protocol wrapper for wlframe.
 * @details     Implements the staging fractional-scale-v1 protocol, which lets
 *              a compositor suggest fractional buffer scales for a wl_surface.
 *
 *              The protocol reports scale as a numerator with a fixed
 *              denominator of 120. For example, 180 means 1.5x scale.
 *
 *              Usage:
 *                1. Bind wlf_wp_fractional_scale_manager_v1 from the registry.
 *                2. For each surface, call
 *                   wlf_wp_fractional_scale_manager_v1_get_fractional_scale().
 *                3. Listen to the per-surface preferred_scale signal.
 *
 * @author      YaoBing Xiao
 * @date        2026-06-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-20, initial version\n
 */

#ifndef WAYLAND_WLF_WP_FRACTIONAL_SCALE_V1_H
#define WAYLAND_WLF_WP_FRACTIONAL_SCALE_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_surface;
struct wp_fractional_scale_manager_v1;
struct wp_fractional_scale_v1;

#define WLF_WP_FRACTIONAL_SCALE_V1_DENOMINATOR 120

/**
 * @brief Wrapper around a bound wp_fractional_scale_manager_v1 global.
 *
 * Obtain via wlf_wp_fractional_scale_manager_v1_create(). Per-surface
 * fractional-scale objects created from this manager remain independent and
 * must be destroyed separately.
 */
struct wlf_wp_fractional_scale_manager_v1 {
	struct wp_fractional_scale_manager_v1 *base; /**< Underlying protocol object */
	uint32_t version;                            /**< Bound protocol version */

	struct {
		struct wlf_signal destroy; /**< Emitted just before object is freed */
	} events;
};

/**
 * @brief Per-surface fractional scale object.
 *
 * Created by wlf_wp_fractional_scale_manager_v1_get_fractional_scale().
 * The compositor sends preferred_scale events through this object.
 */
struct wlf_wp_fractional_scale_v1 {
	struct wp_fractional_scale_v1 *base; /**< Underlying protocol object */
	uint32_t version;                    /**< Bound protocol version */
	double preferred_scale_double;       /**< Last preferred scale as a double */

	struct {
		/**
		 * Emitted when the compositor sends preferred_scale.
		 *
		 * Signal data: struct wlf_wp_fractional_scale_v1 *
		 */
		struct wlf_signal preferred_scale;

		struct wlf_signal destroy; /**< Emitted just before object is freed */
	} events;
};

/**
 * @brief Bind to the wp_fractional_scale_manager_v1 global from the registry.
 *
 * @param wl_registry Wayland registry to bind from.
 * @param name Global name from the registry event.
 * @param version Advertised version; clamped to the client maximum.
 * @return Newly allocated manager, or NULL on failure.
 */
struct wlf_wp_fractional_scale_manager_v1 *
wlf_wp_fractional_scale_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the fractional scale manager.
 *
 * @param manager Manager to destroy; silently ignored when NULL.
 */
void wlf_wp_fractional_scale_manager_v1_destroy(
	struct wlf_wp_fractional_scale_manager_v1 *manager);

/**
 * @brief Create a per-surface fractional scale object.
 *
 * The compositor raises a fractional_scale_exists protocol error if the surface
 * already has a wp_fractional_scale_v1 object.
 *
 * @param manager Bound fractional scale manager.
 * @param surface wl_surface to associate with.
 * @return Newly allocated fractional scale object, or NULL on failure.
 */
struct wlf_wp_fractional_scale_v1 *
wlf_wp_fractional_scale_manager_v1_get_fractional_scale(
	struct wlf_wp_fractional_scale_manager_v1 *manager,
	struct wl_surface *surface);

/**
 * @brief Convert a preferred scale numerator to a double.
 *
 * @param preferred_scale Numerator sent by the compositor.
 * @return Scale as a double using denominator 120.
 */
double wlf_wp_fractional_scale_v1_to_double(uint32_t preferred_scale);

/**
 * @brief Destroy the per-surface fractional scale object.
 *
 * @param fractional_scale Object to destroy; silently ignored when NULL.
 */
void wlf_wp_fractional_scale_v1_destroy(
	struct wlf_wp_fractional_scale_v1 *fractional_scale);

#endif /* WAYLAND_WLF_WP_FRACTIONAL_SCALE_V1_H */
