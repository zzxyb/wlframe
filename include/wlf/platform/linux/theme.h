/**
 * @file        theme.h
 * @brief       Linux theme backend for wlframe.
 * @details     This file declares the Linux-specific theme constructor used by
 *              the generic theme module on Linux desktops.
 * @author      YaoBing Xiao
 * @date        2026-07-04
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-07-04, initial version\n
 */

#ifndef WLF_THEME_Linux_THEME_H
#define WLF_THEME_Linux_THEME_H

#include "wlf/platform/wlf_theme.h"

#include <stdbool.h>

typedef struct _GDBusProxy GDBusProxy;
typedef struct _GMainContext GMainContext;
typedef struct _GMainLoop GMainLoop;
typedef struct _GThread GThread;
typedef unsigned long gulong;

/**
 * @brief Linux theme specific data.
 *
 * Extends the generic theme object with a semantic palette resolved from Linux
 * style contexts and settings change handlers.
 */
struct wlf_linux_theme {
	struct wlf_theme base;  /**< Base theme structure. */
	struct wlf_color palette[WLF_THEME_COLOR_COUNT];  /**< Resolved palette colors. */
	bool use_portal;  /**< Whether the xdg-desktop-portal settings backend is active. */
	GMainContext *monitor_context;  /**< GLib context used by the settings monitor thread. */
	GMainLoop *monitor_loop;  /**< GLib loop used by the settings monitor thread. */
	GThread *monitor_thread;  /**< Thread dispatching Linux settings changes. */
	GDBusProxy *portal_settings;  /**< xdg-desktop-portal settings proxy, if active. */
	gulong change_handler_id;  /**< Signal handler for the active settings backend. */
};

/**
 * @brief Creates a Linux-backed theme object.
 *
 * Uses xdg-desktop-portal settings when available and monitors its DBus
 * changes, otherwise falls back to a one-shot GNOME interface GSettings read.
 *
 * @return A newly allocated Linux theme, or NULL on allocation failure.
 */
struct wlf_linux_theme *wlf_linux_theme_create(void);

/**
 * @brief Reloads the Linux theme from current Linux settings.
 *
 * Re-resolves appearance and style colors, then emits `theme_changed` when any
 * palette role changes and `highlight_changed` when the highlight roles change.
 *
 * @param theme Linux theme to refresh. NULL is allowed.
 */
void wlf_linux_theme_reload(struct wlf_linux_theme *theme);

/**
 * @brief Checks whether a theme is backed by the Linux theme implementation.
 *
 * @param theme Pointer to the generic theme object.
 * @return true if the theme uses the Linux backend, false otherwise.
 */
bool wlf_theme_is_linux(const struct wlf_theme *theme);

/**
 * @brief Casts a generic theme to a Linux theme.
 *
 * @param theme Pointer to the generic theme.
 * @return Pointer to the Linux theme, or NULL if the theme is not Linux-backed.
 */
struct wlf_linux_theme *wlf_linux_theme_from_theme(struct wlf_theme *theme);

#endif // WLF_THEME_Linux_THEME_H
