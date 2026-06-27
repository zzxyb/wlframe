/**
 * @file        wlf_theme.h
 * @brief       Cross-platform theme palette utilities for wlframe.
 * @details     The theme module provides platform aware default palettes,
 *              appearance detection, and role-based color access for wlframe.
 * @author      YaoBing Xiao
 * @date        2026-06-25
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-25, initial version\n
 */

#ifndef THEME_WLF_THEME_H
#define THEME_WLF_THEME_H

#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

/**
 * @brief Preferred appearance mode.
 *
 * Controls whether the theme palette is generated for a light or dark UI.
 */
enum wlf_theme_appearance {
	WLF_THEME_APPEARANCE_LIGHT = 0,
	WLF_THEME_APPEARANCE_DARK,
};

/**
 * @brief Semantic color roles for wlframe themes.
 *
 * These roles provide semantic access to colors so rendering code can request
 * "window", "text", or "highlight" colors without depending on fixed values.
 */
enum wlf_theme_color_role {
	WLF_THEME_COLOR_WINDOW = 0,
	WLF_THEME_COLOR_WINDOW_TEXT,
	WLF_THEME_COLOR_BASE,
	WLF_THEME_COLOR_ALTERNATE_BASE,
	WLF_THEME_COLOR_TEXT,
	WLF_THEME_COLOR_BUTTON,
	WLF_THEME_COLOR_BUTTON_TEXT,
	WLF_THEME_COLOR_BORDER,
	WLF_THEME_COLOR_SEPARATOR,
	WLF_THEME_COLOR_PLACEHOLDER_TEXT,
	WLF_THEME_COLOR_ACCENT,
	WLF_THEME_COLOR_HIGHLIGHT,
	WLF_THEME_COLOR_HIGHLIGHTED_TEXT,
	WLF_THEME_COLOR_LINK,
	WLF_THEME_COLOR_VISITED_LINK,
	WLF_THEME_COLOR_MARK,
	WLF_THEME_COLOR_SUCCESS,
	WLF_THEME_COLOR_WARNING,
	WLF_THEME_COLOR_ERROR,
	WLF_THEME_COLOR_COUNT,
};

struct wlf_theme;

/**
 * @brief Theme backend implementation interface.
 *
 * Each platform theme backend provides a human readable name, a destroy
 * callback, and a semantic color lookup function.
 */
struct wlf_theme_impl {
	const char *name;  /**< Backend name, for example "macos". */
	void (*destroy)(struct wlf_theme *theme);  /**< Destroy backend-specific theme resources. */
	struct wlf_color (*theme_palette_color)(struct wlf_theme *theme,
		enum wlf_theme_color_role role);  /**< Resolve a semantic palette color. */
};

/**
 * @brief Theme object with platform and appearance metadata.
 *
 * A theme exposes the current appearance state and emits lifecycle and
 * palette-change notifications that renderers or widgets can observe.
 *
 * @code
 * struct wlf_theme *theme = wlf_theme_autocreate();
 * struct wlf_color accent =
 * 	wlf_theme_palette_color(theme, WLF_THEME_COLOR_ACCENT);
 * @endcode
 */
struct wlf_theme {
	const struct wlf_theme_impl *impl;  /**< Platform theme backend in use. */

	enum wlf_theme_appearance appearance;  /**< Current resolved light/dark appearance. */

	struct {
		struct wlf_signal destroy;        /**< Emitted before the theme is destroyed. */
		struct wlf_signal theme_changed;  /**< Emitted when system appearance or palette changes. */
	} events;
};

/**
 * @brief Initializes a theme with a platform implementation.
 *
 * This helper sets the backend pointer and initializes the public theme
 * signals. Platform-specific constructors should call this before filling
 * backend state.
 *
 * @param theme Theme object to initialize.
 * @param impl Backend implementation to attach.
 */
void wlf_theme_init(struct wlf_theme *theme,
	const struct wlf_theme_impl *impl);

/**
 * @brief Creates a theme for the current host platform.
 *
 * Selects the best available platform theme backend and returns a newly
 * allocated theme object.
 *
 * @return A newly created theme, or NULL if no platform backend is available.
 */
struct wlf_theme *wlf_theme_autocreate(void);

/**
 * @brief Destroys a theme object.
 *
 * Emits the destroy signal and then releases backend-specific resources.
 *
 * @param theme Theme to destroy. NULL is allowed.
 */
void wlf_theme_destroy(struct wlf_theme *theme);

/**
 * @brief Resolves a color for a semantic theme role.
 *
 * @param theme Theme to query.
 * @param role Semantic color role to resolve.
 * @return The color for the requested role, or a backend-defined fallback.
 */
struct wlf_color wlf_theme_palette_color(struct wlf_theme *theme,
	enum wlf_theme_color_role role);

#endif // THEME_WLF_THEME_H
