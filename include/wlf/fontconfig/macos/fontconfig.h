/**
 * @file        fontconfig.h
 * @brief       macOS font configuration backend for wlframe.
 * @details     This file declares the macOS-specific font configuration
 *              constructor used by the generic fontconfig module to provide
 *              Apple platform default family mappings.
 * @author      YaoBing Xiao
 * @date        2026-06-27
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-27, initial version\n
 */

#ifndef WLF_FONTCONFIG_MACOS_FONTCONFIG_H
#define WLF_FONTCONFIG_MACOS_FONTCONFIG_H

#include "wlf/fontconfig/wlf_fontconfig.h"

#include <stdbool.h>

/**
 * @brief macOS font configuration specific data.
 *
 * Extends the generic font configuration object with a dedicated backend type
 * so callers can detect and downcast macOS-backed configurations.
 */
struct wlf_macos_fontconfig {
	struct wlf_fontconfig base;  /**< Base font configuration structure. */
};

/**
 * @brief Creates a macOS-backed font configuration object.
 *
 * The returned configuration is initialized with Apple-oriented defaults and
 * generic-family environment overrides.
 *
 * @return A newly allocated macOS font configuration, or NULL on failure.
 */
struct wlf_macos_fontconfig *wlf_macos_fontconfig_create(void);

/**
 * @brief Checks whether a font configuration uses the macOS backend.
 *
 * @param config Pointer to the generic font configuration object.
 * @return true if the configuration uses the macOS backend, false otherwise.
 */
bool wlf_fontconfig_is_macos(const struct wlf_fontconfig *config);

/**
 * @brief Casts a generic font configuration to a macOS font configuration.
 *
 * @param config Pointer to the generic font configuration.
 * @return Pointer to the macOS font configuration.
 */
struct wlf_macos_fontconfig *wlf_macos_fontconfig_from_fontconfig(
	struct wlf_fontconfig *config);

#endif // WLF_FONTCONFIG_MACOS_FONTCONFIG_H
