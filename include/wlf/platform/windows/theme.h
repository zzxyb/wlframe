/**
 * @file        theme.h
 * @brief       Windows theme backend for wlframe.
 * @details     This file declares the Windows-specific theme constructor used
 *              by the generic theme module to integrate Windows app appearance
 *              detection, accent color lookup, and registry change monitoring.
 * @author      YaoBing Xiao
 * @date        2026-07-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-07-05, initial version\n
 */

#ifndef WLF_THEME_WINDOWS_THEME_H
#define WLF_THEME_WINDOWS_THEME_H

#include "wlf/platform/wlf_theme.h"

#include <stdbool.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/**
 * @brief Windows theme specific data.
 *
 * Extends the generic theme object with a semantic palette resolved from
 * Windows personalization settings and a background monitor for registry
 * changes.
 */
struct wlf_windows_theme {
	struct wlf_theme base;  /**< Base theme structure. */
	/** Resolved palette colors. */
	struct wlf_color palette[WLF_THEME_COLOR_COUNT];
	HANDLE stop_event;  /**< Event used to stop the settings monitor thread. */
	/** Thread waiting for Windows theme setting changes. */
	HANDLE monitor_thread;
};

/**
 * @brief Creates a Windows-backed theme object.
 *
 * The returned theme reads the current Windows app light/dark mode from the
 * personalization registry settings, resolves the current Windows accent color
 * as the highlight color, and monitors both values for changes.
 *
 * @return A newly allocated Windows theme, or NULL on failure.
 */
struct wlf_windows_theme *wlf_windows_theme_create(void);

/**
 * @brief Reloads the Windows theme from current system settings.
 *
 * Re-resolves appearance and system colors, then emits `theme_changed` when
 * the resolved light/dark appearance changes and `highlight_changed` when the
 * highlight color changes.
 *
 * @param theme Windows theme to refresh. NULL is allowed.
 */
void wlf_windows_theme_reload(struct wlf_windows_theme *theme);

/**
 * @brief Checks whether a theme is backed by the Windows theme implementation.
 *
 * @param theme Pointer to the generic theme object.
 * @return true if the theme uses the Windows backend, false otherwise.
 */
bool wlf_theme_is_windows(const struct wlf_theme *theme);

/**
 * @brief Casts a generic theme to a Windows theme.
 *
 * @param theme Pointer to the generic theme.
 * @return Pointer to the Windows theme.
 */
struct wlf_windows_theme *wlf_windows_theme_from_theme(struct wlf_theme *theme);

#endif // WLF_THEME_WINDOWS_THEME_H
