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
 * AppKit notification observer state. The palette tracks the current macOS
 * selection highlight color independently from the light/dark appearance.
 */
struct wlf_macos_theme {
	struct wlf_theme base;  /**< Base theme structure. */
	bool observer_registered;  /**< Whether the AppKit observer is registered. */
	void *observer;  /**< Opaque AppKit observer object. */
};

/**
 * @brief Creates a macOS-backed theme object.
 *
 * The returned theme uses AppKit to detect the current effective appearance,
 * resolves the current system highlight color, and listens for
 * appearance and system color changes so `theme_changed` and
 * `highlight_changed` can be emitted.
 *
 * @return A newly allocated macOS theme, or NULL on failure.
 */
struct wlf_macos_theme *wlf_macos_theme_create(void);

/**
 * @brief Reloads the macOS theme from current system settings.
 *
 * Re-resolves appearance and system colors, then emits `theme_changed` when
 * the resolved light/dark appearance changes and `highlight_changed` when the
 * highlight color changes.
 *
 * @param theme macOS theme to refresh. NULL is allowed.
 */
void wlf_macos_theme_reload(struct wlf_macos_theme *theme);

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
