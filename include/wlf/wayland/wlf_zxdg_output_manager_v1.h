/**
 * @file        wlf_zxdg_output_manager_v1.h
 * @brief       zxdg-output-manager-v1 client wrappers.
 * @details     Provides output geometry and metadata reported by the
 *              compositor through the zxdg-output protocol.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WLF_ZXDG_OUTPUT_MANAGER_V1_H
#define WLF_ZXDG_OUTPUT_MANAGER_V1_H

#include <stdint.h>

#include "wlf/utils/wlf_signal.h"

struct wl_output;
struct wl_registry;
struct zxdg_output_manager_v1;
struct zxdg_output_v1;

/**
 * @brief zxdg-output wrapper and its latest compositor-provided metadata.
 *
 * The fields are updated when the protocol's done event is received.
 */
struct wlf_zxdg_output_v1 {
	struct zxdg_output_v1 *base; /**< Protocol object. */

	int32_t x; /**< Logical x coordinate. */
	int32_t y; /**< Logical y coordinate. */
	int32_t width; /**< Logical output width. */
	int32_t height; /**< Logical output height. */
	char *name; /**< Output name, or NULL when unavailable. */
	char *description; /**< Output description, or NULL when unavailable. */

	struct {
		struct wlf_signal done;    /**< Payload: wlf_zxdg_output_v1. */
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief zxdg-output manager wrapper.
 *
 * The manager creates one output wrapper for each output being inspected.
 */
struct wlf_zxdg_output_manager_v1 {
	struct zxdg_output_manager_v1 *base; /**< Protocol object. */

	struct {
		struct wlf_signal destroy; /**< Emitted before the wrapper is destroyed. */
	} events;
};

/**
 * @brief Creates a zxdg-output manager wrapper.
 * @param registry Wayland registry used to bind the manager.
 * @param name Global name advertised by the registry.
 * @param version Protocol version to bind.
 * @return Newly allocated manager, or NULL on failure.
 */
struct wlf_zxdg_output_manager_v1 *wlf_zxdg_output_manager_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version);
/**
 * @brief Destroys a zxdg-output manager.
 * @param manager Manager to destroy.
 */
void wlf_zxdg_output_manager_v1_destroy(
	struct wlf_zxdg_output_manager_v1 *manager);

/**
 * @brief Obtains metadata for a Wayland output.
 * @param manager zxdg-output manager.
 * @param output Output whose metadata should be tracked.
 * @return Newly allocated output wrapper, or NULL on failure.
 */
struct wlf_zxdg_output_v1 *wlf_zxdg_output_manager_v1_get_xdg_output(
	struct wlf_zxdg_output_manager_v1 *manager, struct wl_output *output);
/**
 * @brief Destroys a zxdg-output wrapper.
 * @param output Output wrapper to destroy.
 */
void wlf_zxdg_output_v1_destroy(struct wlf_zxdg_output_v1 *output);

#endif /* WLF_ZXDG_OUTPUT_MANAGER_V1_H */
