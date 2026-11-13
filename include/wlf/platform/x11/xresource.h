/**
 * @file        xresource.h
 * @brief       X11 resource database and typed Xft/Xcursor settings.
 * @details     Loads RESOURCE_MANAGER with Xresources/Xdefaults fallbacks using
 *              xcb-util-xrm matching rules. Settings are owned by the manager
 *              and replaced atomically on reload. Use on the X11 event thread.
 * @author      YaoBing Xiao
 * @date        2026-11-13
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-11-13, initial version\n
 */
#ifndef PLATFORM_X11_XRESOURCE_H
#define PLATFORM_X11_XRESOURCE_H

#include "wlf/utils/wlf_signal.h"
#include <stdbool.h>
#include <stdint.h>
#include <xcb/xcb.h>

struct xcb_xrm_database_t;

/** Xft hinting strength, matching the conventional resource values. */
enum wlf_xft_hintstyle {
	WLF_XFT_HINT_NONE,
	WLF_XFT_HINT_SLIGHT,
	WLF_XFT_HINT_MEDIUM,
	WLF_XFT_HINT_FULL,
};

/** Xft subpixel component ordering. */
enum wlf_xft_rgba {
	WLF_XFT_RGBA_NONE,
	WLF_XFT_RGBA_RGB,
	WLF_XFT_RGBA_BGR,
	WLF_XFT_RGBA_VRGB,
	WLF_XFT_RGBA_VBGR,
};

/** Xft LCD filter names, independent of the font rendering library. */
enum wlf_xft_lcdfilter {
	WLF_XFT_LCD_NONE,
	WLF_XFT_LCD_DEFAULT,
	WLF_XFT_LCD_LIGHT,
	WLF_XFT_LCD_LEGACY,
};

/** Fields explicitly supplied by valid X resources. */
enum wlf_xft_field {
	WLF_XFT_DPI = 1 << 0,
	WLF_XFT_ANTIALIAS = 1 << 1,
	WLF_XFT_HINTING = 1 << 2,
	WLF_XFT_HINTSTYLE = 1 << 3,
	WLF_XFT_RGBA = 1 << 4,
	WLF_XFT_LCDFILTER = 1 << 5,
};

/**
 * @brief Parsed Xft settings.
 * @note Missing or invalid fields use the documented defaults below. Consumers
 * should use present to leave unspecified fontconfig preferences untouched.
 */
struct wlf_xft_settings {
	uint32_t present; /**< Bitmask of valid, explicitly supplied fields. */
	double dpi; /**< Xft.dpi, default 96; accepted range (0, 1000]. */
	bool antialias; /**< Xft.antialias, default true. */
	bool hinting; /**< Xft.hinting, default true. */
	enum wlf_xft_hintstyle hintstyle; /**< Xft.hintstyle, default hintfull. */
	enum wlf_xft_rgba rgba; /**< Xft.rgba, default none. */
	enum wlf_xft_lcdfilter lcdfilter; /**< Xft.lcdfilter, default lcddefault. */
};

/**
 * @brief Effective Xcursor settings, including environment overrides.
 * @note XCURSOR_THEME and a valid XCURSOR_SIZE override X resources. No global
 * environment variables or server properties are modified by the manager.
 */
struct wlf_xcursor_settings {
	char *theme; /**< Owned theme name, or NULL for the default theme. */
	int size; /**< Pixel size in [1, 1024]; default 24 scaled by Xft.dpi/96. */
	bool theme_core; /**< Xcursor.theme_core, default true; theme core shapes. */
};

/**
 * @brief X resource manager borrowing its XCB connection.
 * @note Destroy before disconnecting. Returned settings and database are
 * invalidated by a successful reload. Remove listeners before destruction.
 */
struct wlf_xresource_manager {
	xcb_connection_t *xcb; /**< Borrowed live XCB connection. */
	xcb_window_t root; /**< Screen-zero root holding RESOURCE_MANAGER. */
	struct xcb_xrm_database_t *database; /**< Owned xcb-util-xrm database. */
	struct wlf_xft_settings xft; /**< Current font settings. */
	struct wlf_xcursor_settings xcursor; /**< Current cursor settings. */
	struct {
		struct wlf_signal changed; /**< Successful reload; data is this manager. */
	} events; /**< Manager notifications. */
};

/**
 * @brief Loads resources and subscribes to root property notifications.
 * @param xcb Live connection to borrow.
 * @return New manager, or NULL on connection, allocation, or X11 failure.
 * @note Loads RESOURCE_MANAGER, otherwise ~/.Xresources then ~/.Xdefaults;
 * XENVIRONMENT or ~/.Xdefaults-hostname is merged with override enabled.
 * Existing event selections on this connection are preserved.
 */
struct wlf_xresource_manager *wlf_xresource_manager_create(xcb_connection_t *xcb);

/**
 * @brief Releases the database, strings, and manager.
 * @param manager Manager to destroy; NULL is allowed.
 * @note Root property event selection is retained because other consumers on
 * the same connection may still need it. The connection remains borrowed.
 */
void wlf_xresource_manager_destroy(struct wlf_xresource_manager *manager);

/**
 * @brief Reloads resources and notifies listeners after committing settings.
 * @param manager Manager to update.
 * @return true on success, false on failure with previous settings retained.
 * @note Missing resources are valid and restore defaults. File/environment
 * changes need an explicit reload; RESOURCE_MANAGER changes can be dispatched.
 */
bool wlf_xresource_manager_reload(struct wlf_xresource_manager *manager);

/**
 * @brief Handles a RESOURCE_MANAGER property change or deletion.
 * @param manager Manager to update.
 * @param event Borrowed event from the manager connection.
 * @return true if the event belongs to RESOURCE_MANAGER, false otherwise.
 * @note Both PropertyNewValue and PropertyDelete cause a reload.
 */
bool wlf_xresource_manager_handle_event(struct wlf_xresource_manager *manager,
	const xcb_generic_event_t *event);

/**
 * @brief Queries a resource using Xrm instance/class and wildcard precedence.
 * @param manager Manager containing the database.
 * @param name Fully qualified instance name, for example "Xft.dpi".
 * @param class_name Optional class name with the same component count.
 * @param value Output string; initialized to NULL and freed by the caller.
 * @return true when found, false for an invalid query or missing resource.
 */
bool wlf_xresource_manager_get_string(struct wlf_xresource_manager *manager,
	const char *name, const char *class_name, char **value);

#endif // PLATFORM_X11_XRESOURCE_H
