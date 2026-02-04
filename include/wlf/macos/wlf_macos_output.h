/**
 * @file        wlf_macos_output.h
 * @brief       macOS backend implementation for wlf_output.
 * @details     This file defines the macOS-specific output structure and creation
 *              helpers. Each @ref wlf_macos_output represents one NSScreen and is
 *              owned by the macOS output manager.
 * @author      YaoBing Xiao
 * @date        2026-03-17
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-17, initial version\n
 */

#ifndef MACOS_WLF_MACOS_OUTPUT_H
#define MACOS_WLF_MACOS_OUTPUT_H

#include "wlf/types/wlf_output.h"

#include <stdbool.h>

struct wlf_macos_output_manager;

/**
 * @brief macOS-specific output object.
 *
 * Wraps a single NSScreen as a @ref wlf_output. The NSScreen is referenced
 * through an opaque void pointer so the header can be used from plain-C TUs.
 */
struct wlf_macos_output {
	struct wlf_output base;              /**< Base output (must be first). */
	void *ns_screen;                     /**< NSScreen handle (opaque). */
	struct wlf_macos_output_manager *manager; /**< Owning output manager. */
};

/**
 * @brief Creates a macOS output manager and enumerates connected screens.
 *
 * Enumerates all NSScreen instances available at call time and registers
 * a listener for screen-configuration change notifications.
 *
 * @return Pointer to the generic @ref wlf_output_manager, or NULL on failure.
 */
struct wlf_output_manager *wlf_macos_output_manager_create(void);

/**
 * @brief Checks whether an output is backed by the macOS backend.
 *
 * @param output Generic output pointer.
 * @return true if the output was created by the macOS backend.
 */
bool wlf_output_is_macos(const struct wlf_output *output);

/**
 * @brief Downcasts a generic output to a macOS output.
 *
 * @param output Generic output pointer.
 * @return Pointer to the underlying wlf_macos_output, or NULL if mismatch.
 */
struct wlf_macos_output *wlf_macos_output_from_output(struct wlf_output *output);

#endif // MACOS_WLF_MACOS_OUTPUT_H
