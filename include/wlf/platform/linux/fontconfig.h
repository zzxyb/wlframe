/**
 * @file        fontconfig.h
 * @brief       Linux font configuration backend for wlframe.
 * @details     This file declares the Linux-specific font configuration
 *              constructor used by the generic fontconfig module to resolve
 *              fontconfig-backed family mappings.
 * @author      YaoBing Xiao
 * @date        2026-07-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-07-07, initial version\n
 */

#ifndef WLF_FONTCONFIG_LINUX_FONTCONFIG_H
#define WLF_FONTCONFIG_LINUX_FONTCONFIG_H

#include "wlf/platform/wlf_fontconfig.h"

#include <stdbool.h>

typedef struct _FcConfig FcConfig;

/**
 * @brief Linux font configuration specific data.
 *
 * Extends the generic font configuration object with the fontconfig handle used
 * to resolve generic family aliases and fallback order.
 */
struct wlf_linux_fontconfig {
	struct wlf_fontconfig base;  /**< Base font configuration structure. */
	FcConfig *fc_config;  /**< Fontconfig configuration owned by this backend. */
};

/**
 * @brief Creates a Linux-backed font configuration object.
 *
 * The returned configuration is initialized from fontconfig generic aliases and
 * built-in fallback families.
 *
 * @return A newly allocated Linux font configuration, or NULL on failure.
 */
struct wlf_linux_fontconfig *wlf_linux_fontconfig_create(void);

/**
 * @brief Checks whether a font configuration uses the Linux backend.
 *
 * @param config Pointer to the generic font configuration object.
 * @return true if the configuration uses the Linux backend, false otherwise.
 */
bool wlf_fontconfig_is_linux(const struct wlf_fontconfig *config);

/**
 * @brief Casts a generic font configuration to a Linux font configuration.
 *
 * @param config Pointer to the generic font configuration.
 * @return Pointer to the Linux font configuration.
 */
struct wlf_linux_fontconfig *wlf_linux_fontconfig_from_fontconfig(
	struct wlf_fontconfig *config);

#endif // WLF_FONTCONFIG_LINUX_FONTCONFIG_H
