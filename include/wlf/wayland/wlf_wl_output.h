/**
 * @file        wlf_wl_output.h
 * @brief       Wayland backend implementation for wlf_output and wlf_output_manager.
 * @details     This file provides the Wayland-specific bindings for output handling
 *              in wlframe. It connects the generic wlf_output abstraction with
 *              Wayland objects such as wl_output and zxdg_output_v1.
 *
 *              It offers:
 *              - Wayland-backed wlf_output implementation
 *              - Wayland-backed wlf_output_manager implementation
 *              - Registry-based creation helpers
 *              - Backend-type checking utilities
 *
 *              This layer enables wlframe to receive geometry, scale,
 *              physical size, transform, subpixel, and name updates from
 *              the Wayland compositor.
 *
 * @author      YaoBing Xiao
 * @date        2025-12-10
 * @version     v1.0
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-12-10, initial version\n
 */

#ifndef WAYLAND_WLF_WL_OUTPUT_H
#define WAYLAND_WLF_WL_OUTPUT_H

#include "wlf/types/wlf_output.h"

#include <stdint.h>

struct wl_registry;
struct wl_output;
struct zxdg_output_manager_v1;
struct zxdg_output_v1;

/**
 * @brief Wayland-backed wlf_output implementation.
 * @details
 * Wraps:
 * - wl_output: the Wayland protocol object for output properties
 * - zxdg_output_v1: extended output information (name, description, logical size)
 *
 * The `base` field integrates this object into the generic output system.
 */
struct wlf_wl_output {
	struct wlf_output base;          /**< Generic output base structure */
	struct wl_output *output;        /**< Wayland wl_output instance */
	struct zxdg_output_v1 *xdg_output; /**< Extended xdg-output instance */
};

/**
 * @brief Wayland-backed output manager.
 * @details Manages all wl_output objects via zxdg_output_manager_v1.
 */
struct wlf_wl_output_manager {
	struct wlf_output_manager base;       /**< Generic output manager base */
	struct zxdg_output_manager_v1 *manager; /**< Wayland xdg-output manager */
};

/**
 * @brief Creates a wlf_output from a Wayland registry announcement.
 * @param wl_registry The Wayland registry.
 * @param name Global name from wl_registry.
 * @param version The version of the wl_output interface.
 * @return Pointer to the newly created wlf_output, or NULL on failure.
 */
struct wlf_output *wlf_output_create_from_wl_registry(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Checks whether a given wlf_output is backed by the Wayland backend.
 * @param output The output instance.
 * @return true if this output comes from the Wayland backend, false otherwise.
 */
bool wlf_output_is_wayland(const struct wlf_output *output);

/**
 * @brief Converts a generic wlf_output to a wlf_wl_output.
 * @param output The generic output.
 * @return Wayland-specific backend struct, or NULL if output is not Wayland-backed.
 */
struct wlf_wl_output *wlf_wl_output_from_backend(struct wlf_output *output);

/**
 * @brief Creates a wlf_output_manager from the Wayland registry.
 * @param wl_registry The Wayland registry.
 * @param name The registry name for zxdg_output_manager_v1.
 * @param version The protocol version.
 * @return Pointer to the newly created wlf_output_manager, or NULL on failure.
 */
struct wlf_output_manager *wlf_output_manager_create_from_wl_registry(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Checks whether the output manager is a Wayland-backed manager.
 * @param manager The manager instance.
 * @return true if using Wayland backend, false otherwise.
 */
bool wlf_wl_output_manager_is_wayland(const struct wlf_output_manager *manager);

/**
 * @brief Converts a generic wlf_output_manager to its Wayland backend version.
 * @param manager The generic manager.
 * @return Wayland-specific backend struct, or NULL if not Wayland-backed.
 */
struct wlf_wl_output_manager *wlf_wl_output_manager_from_backend(
	struct wlf_output_manager *manager);

#endif // WAYLAND_WLF_WL_OUTPUT_H
