/**
 * @file        wlf_font_backend.h
 * @brief       Font backend plugin interface for cross-platform font system access.
 * @details     This file provides a plugin-based backend system for accessing
 *              system fonts on different platforms (macOS, Linux, etc.).
 * @author      YaoBing Xiao
 * @date        2025-06-17
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-17, initial version\n
 */

#ifndef FONT_WLF_FONT_BACKEND_H
#define FONT_WLF_FONT_BACKEND_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Font weight enumeration.
 */
enum wlf_font_weight {
	WLF_FONT_WEIGHT_THIN = 100,
	WLF_FONT_WEIGHT_EXTRA_LIGHT = 200,
	WLF_FONT_WEIGHT_LIGHT = 300,
	WLF_FONT_WEIGHT_NORMAL = 400,
	WLF_FONT_WEIGHT_MEDIUM = 500,
	WLF_FONT_WEIGHT_SEMI_BOLD = 600,
	WLF_FONT_WEIGHT_BOLD = 700,
	WLF_FONT_WEIGHT_EXTRA_BOLD = 800,
	WLF_FONT_WEIGHT_BLACK = 900,
};

/**
 * @brief Font style enumeration.
 */
enum wlf_font_style {
	WLF_FONT_STYLE_NORMAL,
	WLF_FONT_STYLE_ITALIC,
	WLF_FONT_STYLE_OBLIQUE,
};

/**
 * @brief Font width enumeration.
 */
enum wlf_font_width {
	WLF_FONT_WIDTH_ULTRA_CONDENSED = 1,
	WLF_FONT_WIDTH_EXTRA_CONDENSED = 2,
	WLF_FONT_WIDTH_CONDENSED = 3,
	WLF_FONT_WIDTH_SEMI_CONDENSED = 4,
	WLF_FONT_WIDTH_NORMAL = 5,
	WLF_FONT_WIDTH_SEMI_EXPANDED = 6,
	WLF_FONT_WIDTH_EXPANDED = 7,
	WLF_FONT_WIDTH_EXTRA_EXPANDED = 8,
	WLF_FONT_WIDTH_ULTRA_EXPANDED = 9,
};

/**
 * @brief System font information structure.
 */
struct wlf_font_info {
	char *family_name;                  /**< Font family name */
	char *style_name;                   /**< Font style name */
	char *postscript_name;              /**< PostScript name */
	char *file_path;                    /**< Path to font file */
	enum wlf_font_weight weight;        /**< Font weight */
	enum wlf_font_style style;          /**< Font style */
	enum wlf_font_width width;          /**< Font width */
	bool is_monospace;                  /**< Whether font is monospace */
	bool is_scalable;                   /**< Whether font is scalable */
	char **languages;                   /**< Supported languages (NULL-terminated array) */
	char **character_sets;              /**< Supported character sets (NULL-terminated array) */
};

/**
 * @brief Font enumeration callback function type.
 * @param info Font information structure.
 * @param user_data User-provided data.
 * @return true to continue enumeration, false to stop.
 */
typedef bool (*wlf_font_enum_callback)(const struct wlf_font_info *info, void *user_data);

/**
 * @brief Font backend interface structure.
 */
struct wlf_font_backend {
	const char *name;                   /**< Backend name */
	const char *description;            /**< Backend description */

	/**
	 * @brief Initialize the backend.
	 * @return true on success, false on failure.
	 */
	bool (*init)(void);

	/**
	 * @brief Cleanup the backend.
	 */
	void (*cleanup)(void);

	/**
	 * @brief Enumerate all available system fonts.
	 * @param callback Callback function to call for each font.
	 * @param user_data User data to pass to callback.
	 * @return true on success, false on failure.
	 */
	bool (*enumerate_fonts)(wlf_font_enum_callback callback, void *user_data);

	/**
	 * @brief Find fonts matching a pattern.
	 * @param pattern Font pattern (family name, style, etc.).
	 * @param callback Callback function to call for each matching font.
	 * @param user_data User data to pass to callback.
	 * @return true on success, false on failure.
	 */
	bool (*find_fonts)(const char *pattern, wlf_font_enum_callback callback, void *user_data);

	/**
	 * @brief Get the path to a font file matching the given criteria.
	 * @param family_name Font family name.
	 * @param style Font style.
	 * @param weight Font weight.
	 * @return Font file path (must be freed by caller), or NULL if not found.
	 */
	char* (*get_font_path)(const char *family_name, enum wlf_font_style style, enum wlf_font_weight weight);

	/**
	 * @brief Get default font for a specific language/script.
	 * @param language Language code (e.g., "en", "zh", "ja").
	 * @return Font file path (must be freed by caller), or NULL if not found.
	 */
	char* (*get_default_font)(const char *language);

	/**
	 * @brief Get system monospace font.
	 * @return Font file path (must be freed by caller), or NULL if not found.
	 */
	char* (*get_monospace_font)(void);

	/**
	 * @brief Check if backend is available on current platform.
	 * @return true if available, false otherwise.
	 */
	bool (*is_available)(void);
};

/**
 * @brief Initialize the font backend system.
 * @return true on success, false on failure.
 */
bool wlf_font_backend_init(void);

/**
 * @brief Cleanup the font backend system.
 */
void wlf_font_backend_cleanup(void);

/**
 * @brief Register a font backend.
 * @param backend Backend to register.
 * @return true on success, false on failure.
 */
bool wlf_font_backend_register(const struct wlf_font_backend *backend);

/**
 * @brief Get the active font backend.
 * @return Active backend, or NULL if none available.
 */
const struct wlf_font_backend* wlf_font_backend_get_active(void);

/**
 * @brief Get all available backends.
 * @param count Output parameter for number of backends.
 * @return Array of backend pointers (NULL-terminated).
 */
const struct wlf_font_backend** wlf_font_backend_get_all(size_t *count);

/**
 * @brief Free font info structure.
 * @param info Font info to free.
 */
void wlf_font_info_free(struct wlf_font_info *info);

/**
 * @brief Enumerate all available system fonts.
 * @param callback Callback function to call for each font.
 * @param user_data User data to pass to callback.
 * @return true on success, false on failure.
 */
bool wlf_font_enumerate_system_fonts(wlf_font_enum_callback callback, void *user_data);

/**
 * @brief Find fonts matching a pattern.
 * @param pattern Font pattern.
 * @param callback Callback function to call for each matching font.
 * @param user_data User data to pass to callback.
 * @return true on success, false on failure.
 */
bool wlf_font_find_system_fonts(const char *pattern, wlf_font_enum_callback callback, void *user_data);

/**
 * @brief Get system font path by criteria.
 * @param family_name Font family name.
 * @param style Font style.
 * @param weight Font weight.
 * @return Font file path (must be freed by caller), or NULL if not found.
 */
char* wlf_font_get_system_font_path(const char *family_name, enum wlf_font_style style, enum wlf_font_weight weight);

/**
 * @brief Get default system font for a language.
 * @param language Language code.
 * @return Font file path (must be freed by caller), or NULL if not found.
 */
char* wlf_font_get_system_default_font(const char *language);

/**
 * @brief Get system monospace font.
 * @return Font file path (must be freed by caller), or NULL if not found.
 */
char* wlf_font_get_system_monospace_font(void);

#endif // FONT_WLF_FONT_BACKEND_H
