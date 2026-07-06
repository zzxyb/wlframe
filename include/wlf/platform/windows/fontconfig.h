/**
 * @file        fontconfig.h
 * @brief       Windows font configuration backend for wlframe.
 * @details     This file declares the Windows-specific font configuration
 *              constructor used by the generic fontconfig module to provide
 *              Windows default family mappings.
 * @author      YaoBing Xiao
 * @date        2026-07-06
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-07-06, initial version\n
 */

#ifndef WLF_FONTCONFIG_WINDOWS_FONTCONFIG_H
#define WLF_FONTCONFIG_WINDOWS_FONTCONFIG_H

#include "wlf/platform/wlf_fontconfig.h"

#include <stdbool.h>

/**
 * @brief Windows font configuration specific data.
 *
 * Extends the generic font configuration object with a dedicated backend type
 * so callers can detect and downcast Windows-backed configurations.
 */
struct wlf_windows_fontconfig {
	struct wlf_fontconfig base;  /**< Base font configuration structure. */
};

/**
 * @brief Creates a Windows-backed font configuration object.
 *
 * The returned configuration is initialized with Windows-oriented defaults and
 * generic-family environment overrides.
 *
 * @return A newly allocated Windows font configuration, or NULL on failure.
 */
struct wlf_windows_fontconfig *wlf_windows_fontconfig_create(void);

/**
 * @brief Checks whether a font configuration uses the Windows backend.
 *
 * @param config Pointer to the generic font configuration object.
 * @return true if the configuration uses the Windows backend, false otherwise.
 */
bool wlf_fontconfig_is_windows(const struct wlf_fontconfig *config);

/**
 * @brief Casts a generic font configuration to a Windows font configuration.
 *
 * @param config Pointer to the generic font configuration.
 * @return Pointer to the Windows font configuration.
 */
struct wlf_windows_fontconfig *wlf_windows_fontconfig_from_fontconfig(
	struct wlf_fontconfig *config);

#endif // WLF_FONTCONFIG_WINDOWS_FONTCONFIG_H
