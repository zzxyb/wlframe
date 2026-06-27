/**
 * @file        theme.h
 * @brief       macOS theme backend for wlframe.
 * @details     This file declares the macOS-specific theme constructor used
 *              by the generic theme module to integrate AppKit appearance
 *              detection and system theme change notifications.
 * @author      YaoBing Xiao
 * @date        2026-06-27
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-27, initial version\n
 */

#ifndef WLF_THEME_MACOS_THEME_H
#define WLF_THEME_MACOS_THEME_H

#include "wlf/platform/wlf_theme.h"

/**
 * @brief macOS theme specific data.
 *
 * Extends the generic theme object with the resolved semantic palette and
 * AppKit notification observer state.
 */
struct wlf_macos_theme {
	struct wlf_theme base;  /**< Base theme structure. */
	struct wlf_color palette[WLF_THEME_COLOR_COUNT];  /**< Resolved palette colors. */
	bool observer_registered;  /**< Whether the AppKit observer is registered. */
	void *observer;  /**< Opaque AppKit observer object. */
};

/**
 * @brief Creates a macOS-backed theme object.
 *
 * The returned theme uses AppKit to detect the current effective appearance
 * and listens for system appearance changes so `theme_changed` can be emitted.
 *
 * @return A newly allocated macOS theme, or NULL on failure.
 */
struct wlf_macos_theme *wlf_macos_theme_create(void);

/**
 * @brief Checks whether a theme is backed by the macOS theme implementation.
 *
 * @param theme Pointer to the generic theme object.
 * @return true if the theme uses the macOS backend, false otherwise.
 */
bool wlf_theme_is_macos(const struct wlf_theme *theme);

/**
 * @brief Casts a generic theme to a macOS theme.
 *
 * @param theme Pointer to the generic theme.
 * @return Pointer to the macOS theme, or NULL if the theme is not macOS-backed.
 */
struct wlf_macos_theme *wlf_macos_theme_from_theme(struct wlf_theme *theme);

#endif // WLF_THEME_MACOS_THEME_H
