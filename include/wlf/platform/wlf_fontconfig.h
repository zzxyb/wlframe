/**
 * @file        wlf_fontconfig.h
 * @brief       Cross-platform font defaults for wlframe.
 * @details     This module provides platform aware generic font family mapping
 *              and fallback chains suitable for UI text rendering.
 * @author      YaoBing Xiao
 * @date        2026-06-25
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-25, initial version\n
 */

#ifndef FONTCONFIG_WLF_FONTCONFIG_H
#define FONTCONFIG_WLF_FONTCONFIG_H

#include <stddef.h>

/**
 * @brief Maximum number of font families stored for each semantic role.
 */
#define WLF_FONTCONFIG_MAX_FALLBACKS 8
/**
 * @brief Font family name capacity in bytes, including the terminating NUL.
 */
#define WLF_FONTCONFIG_FAMILY_NAME_MAX 64

/**
 * @brief Generic font roles commonly used by wlframe.
 *
 * Each role represents a semantic font usage such as UI text, serif content,
 * or monospace code text.
 */
enum wlf_font_role {
	WLF_FONT_ROLE_UI = 0, /**< Semantic font role: ui. */
	WLF_FONT_ROLE_SANS_SERIF, /**< Semantic font role: sans serif. */
	WLF_FONT_ROLE_SERIF, /**< Semantic font role: serif. */
	WLF_FONT_ROLE_MONOSPACE, /**< Semantic font role: monospace. */
	WLF_FONT_ROLE_ROUNDED, /**< Semantic font role: rounded. */
	WLF_FONT_ROLE_EMOJI, /**< Semantic font role: emoji. */
	WLF_FONT_ROLE_TITLE, /**< Semantic font role: title. */
	WLF_FONT_ROLE_COUNT, /**< Number of semantic font roles; not a usable role. */
};

struct wlf_fontconfig;

/**
 * @brief Font configuration backend implementation interface.
 *
 * Mirrors the theme module's backend model so platform-specific font defaults
 * can be selected through a shared public object API.
 */
struct wlf_fontconfig_impl {
	const char *name;  /**< Backend name, for example "macos". */
	void (*destroy)(struct wlf_fontconfig *config);  /**< Destroy backend-specific resources. */
};

/**
 * @brief Platform font family mapping and fallback chains.
 *
 * Stores the ordered fallback family list for each semantic font role.
 */
struct wlf_fontconfig {
	const struct wlf_fontconfig_impl *impl; /**< Implementation virtual method table. */
	double ui_scale; /**< Scale applied to platform font defaults. */
	char families[WLF_FONT_ROLE_COUNT][WLF_FONTCONFIG_MAX_FALLBACKS][WLF_FONTCONFIG_FAMILY_NAME_MAX]; /**< Ordered font family fallbacks for each semantic role. */
	size_t counts[WLF_FONT_ROLE_COUNT]; /**< Number of valid font families for each role. */
};

/**
 * @brief Return a stable lowercase string for a font role.
 *
 * @param role Font role.
 * @return Constant role name string.
 */
const char *wlf_font_role_name(enum wlf_font_role role);

/**
 * @brief Initialize a font configuration with a backend implementation.
 *
 * This helper wires the backend metadata into the public object. Platform
 * constructors should call this before filling role mappings.
 *
 * @param config Font configuration to initialize.
 * @param impl Backend implementation to attach.
 */
void wlf_fontconfig_init(struct wlf_fontconfig *config,
	const struct wlf_fontconfig_impl *impl);

/**
 * @brief Create a font configuration for the current host platform.
 *
 * Selects the best available platform backend and returns a newly allocated
 * configuration object.
 *
 * @return A newly created font configuration, or NULL on allocation failure.
 */
struct wlf_fontconfig *wlf_fontconfig_autocreate(void);

/**
 * @brief Destroy a font configuration object.
 *
 * @param config Configuration to destroy. NULL is allowed.
 */
void wlf_fontconfig_destroy(struct wlf_fontconfig *config);

/**
 * @brief Get the primary family for a role.
 *
 * @param config Font configuration.
 * @param role Font role.
 * @return Primary family name, or NULL if unavailable.
 */
const char *wlf_fontconfig_primary_family(const struct wlf_fontconfig *config,
	enum wlf_font_role role);

/**
 * @brief Get the fallback family list for a role.
 *
 * @param config Font configuration.
 * @param role Font role.
 * @param out_families Output array receiving family pointers.
 * @param max_families Capacity of output array.
 * @return Number of families written.
 */
size_t wlf_fontconfig_get_families(const struct wlf_fontconfig *config,
	enum wlf_font_role role,
	const char **out_families,
	size_t max_families);

/**
 * @brief Resolve a generic family name into a primary family.
 *
 * @param config Font configuration.
 * @param generic_family Generic family such as "sans-serif".
 * @return Matching primary family, or NULL if no match exists.
 */
const char *wlf_fontconfig_resolve_generic(const struct wlf_fontconfig *config,
	const char *generic_family);

#endif // FONTCONFIG_WLF_FONTCONFIG_H
