/**
 * @file        wlf_font.h
 * @brief       Font loading and glyph rasterization utility for wlframe.
 * @details     This file provides structures and functions for font loading, glyph caching,
 *              and text rendering, inspired by fcft library design principles.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef FONT_WLF_FONT_H
#define FONT_WLF_FONT_H

#include "wlf/font/wlf_font_backend.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Subpixel rendering modes for font antialiasing.
 */
enum wlf_font_subpixel {
	WLF_FONT_SUBPIXEL_DEFAULT,          /**< Use subpixel order from FontConfig */
	WLF_FONT_SUBPIXEL_NONE,             /**< Disable subpixel antialiasing (use grayscale) */
	WLF_FONT_SUBPIXEL_HORIZONTAL_RGB,   /**< Horizontal RGB subpixel layout */
	WLF_FONT_SUBPIXEL_HORIZONTAL_BGR,   /**< Horizontal BGR subpixel layout */
	WLF_FONT_SUBPIXEL_VERTICAL_RGB,     /**< Vertical RGB subpixel layout */
	WLF_FONT_SUBPIXEL_VERTICAL_BGR,     /**< Vertical BGR subpixel layout */
};

/**
 * @brief Font scaling filter types.
 */
enum wlf_font_filter {
	WLF_FONT_FILTER_NONE,               /**< No filtering */
	WLF_FONT_FILTER_BILINEAR,           /**< Bilinear filtering */
	WLF_FONT_FILTER_LANCZOS3,           /**< Lanczos3 filtering */
};

/**
 * @brief Font hinting modes.
 */
enum wlf_font_hinting {
	WLF_FONT_HINTING_NONE,              /**< No hinting */
	WLF_FONT_HINTING_SLIGHT,            /**< Slight hinting */
	WLF_FONT_HINTING_MEDIUM,            /**< Medium hinting */
	WLF_FONT_HINTING_FULL,              /**< Full hinting */
};

/**
 * @brief Glyph bitmap data structure.
 */
struct wlf_glyph {
	uint32_t codepoint;                 /**< Unicode codepoint */
	struct {
		int width;                      /**< Glyph bitmap width */
		int height;                     /**< Glyph bitmap height */
	} size;                             /**< Glyph bitmap size */
	struct {
		int x;                          /**< Bearing X offset */
		int y;                          /**< Bearing Y offset */
	} bearing;                          /**< Glyph bearing (offset from baseline) */
	struct {
		int x;                          /**< Advance X offset */
		int y;                          /**< Advance Y offset */
	} advance;                          /**< Advance to next glyph position */
	uint8_t *bitmap;                    /**< Glyph bitmap data (RGBA format) */
	bool is_color;                      /**< Whether this is a color glyph */
	bool cached;                        /**< Whether this glyph is cached */
};

/**
 * @brief Font configuration options.
 */
struct wlf_font_options {
	enum wlf_font_subpixel subpixel;    /**< Subpixel rendering mode */
	enum wlf_font_filter filter;       /**< Scaling filter */
	enum wlf_font_hinting hinting;     /**< Hinting mode */
	bool antialias;                     /**< Enable antialiasing */
	bool autohint;                      /**< Use autohinter */
	bool embolden;                      /**< Embolden font */
	double oblique;                     /**< Oblique angle (0.0 = no oblique) */
	double dpi;                         /**< DPI for font rendering */
};

/**
 * @brief Font instance structure.
 */
struct wlf_font {
	char *family;                       /**< Font family name */
	char *style;                        /**< Font style */
	int size;                           /**< Font size in pixels */
	int height;                         /**< Line height */
	int ascent;                         /**< Ascent from baseline */
	int descent;                        /**< Descent from baseline */

	struct {
		int x;                          /**< Maximum advance X */
		int y;                          /**< Maximum advance Y */
	} max_advance;                      /**< Maximum advance */

	struct {
		int position;                   /**< Underline position */
		int thickness;                  /**< Underline thickness */
	} underline;

	struct {
		int position;                   /**< Strikeout position */
		int thickness;                  /**< Strikeout thickness */
	} strikeout;

	struct wlf_font_options options;    /**< Font rendering options */
	void *ft_face;                      /**< FreeType face (opaque) */
	void *glyph_cache;                  /**< Glyph cache (opaque) */
	bool is_loaded;                     /**< Whether font is successfully loaded */
};

/**
 * @brief Text metrics structure.
 */
struct wlf_text_metrics {
	struct {
		int width;                      /**< Text bounding box width */
		int height;                     /**< Text bounding box height */
	} size;                             /**< Text bounding box size */
	int baseline_y;                     /**< Baseline Y position */
	int advance_x;                      /**< Total advance width */
};

/**
 * @brief Default font options.
 */
static const struct wlf_font_options WLF_FONT_OPTIONS_DEFAULT = {
	.subpixel = WLF_FONT_SUBPIXEL_DEFAULT,
	.filter = WLF_FONT_FILTER_BILINEAR,
	.hinting = WLF_FONT_HINTING_SLIGHT,
	.antialias = true,
	.autohint = false,
	.embolden = false,
	.oblique = 0.0,
	.dpi = 96.0,
};

/**
 * @brief Initialize the font subsystem.
 * @return true on success, false on failure.
 */
bool wlf_font_init(void);

/**
 * @brief Cleanup the font subsystem.
 */
void wlf_font_cleanup(void);

/**
 * @brief Load a font from a FontConfig pattern string.
 * @param pattern FontConfig pattern (e.g., "Monospace:size=12").
 * @param options Font rendering options (NULL for defaults).
 * @return Loaded font instance, or NULL on failure.
 */
struct wlf_font* wlf_font_load(const char *pattern, const struct wlf_font_options *options);

/**
 * @brief Load a font from a file path.
 * @param path Font file path.
 * @param size Font size in pixels.
 * @param options Font rendering options (NULL for defaults).
 * @return Loaded font instance, or NULL on failure.
 */
struct wlf_font* wlf_font_load_from_file(const char *path, int size, const struct wlf_font_options *options);

/**
 * @brief Destroy a font instance and free its resources.
 * @param font Font instance to destroy.
 */
void wlf_font_destroy(struct wlf_font *font);

/**
 * @brief Rasterize a single glyph.
 * @param font Font instance.
 * @param codepoint Unicode codepoint.
 * @return Rasterized glyph, or NULL on failure. Must be freed with wlf_glyph_destroy().
 */
struct wlf_glyph* wlf_font_rasterize_glyph(struct wlf_font *font, uint32_t codepoint);

/**
 * @brief Get text metrics without rasterizing.
 * @param font Font instance.
 * @param text UTF-8 encoded text.
 * @param metrics Output text metrics.
 * @return true on success, false on failure.
 */
bool wlf_font_get_text_metrics(struct wlf_font *font, const char *text, struct wlf_text_metrics *metrics);

/**
 * @brief Rasterize a text string.
 * @param font Font instance.
 * @param text UTF-8 encoded text.
 * @param color Text color (RGBA format).
 * @return Rasterized text bitmap, or NULL on failure. Must be freed with wlf_glyph_destroy().
 */
struct wlf_glyph* wlf_font_rasterize_text(struct wlf_font *font, const char *text, uint32_t color);

/**
 * @brief Destroy a glyph and free its resources.
 * @param glyph Glyph to destroy.
 */
void wlf_glyph_destroy(struct wlf_glyph *glyph);

/**
 * @brief Clear the glyph cache for a font.
 * @param font Font instance.
 */
void wlf_font_clear_cache(struct wlf_font *font);

/**
 * @brief Get the number of cached glyphs for a font.
 * @param font Font instance.
 * @return Number of cached glyphs.
 */
size_t wlf_font_get_cache_size(struct wlf_font *font);

/**
 * @brief Set the maximum cache size for a font.
 * @param font Font instance.
 * @param max_size Maximum number of glyphs to cache (0 = unlimited).
 */
void wlf_font_set_max_cache_size(struct wlf_font *font, size_t max_size);

/**
 * @brief Check if a font supports a specific codepoint.
 * @param font Font instance.
 * @param codepoint Unicode codepoint.
 * @return true if supported, false otherwise.
 */
bool wlf_font_has_glyph(struct wlf_font *font, uint32_t codepoint);

/**
 * @brief Get kerning adjustment between two glyphs.
 * @param font Font instance.
 * @param left_codepoint Left glyph codepoint.
 * @param right_codepoint Right glyph codepoint.
 * @param kerning_x Output parameter for kerning X adjustment in pixels.
 * @param kerning_y Output parameter for kerning Y adjustment in pixels.
 */
void wlf_font_get_kerning(struct wlf_font *font, uint32_t left_codepoint, uint32_t right_codepoint, int *kerning_x, int *kerning_y);

/* System font access functions */

/**
 * @brief Load a font from system using family name and style criteria.
 * @param family_name Font family name.
 * @param style Font style.
 * @param weight Font weight.
 * @param size Font size in pixels.
 * @param options Font rendering options (NULL for defaults).
 * @return Loaded font instance, or NULL on failure.
 */
struct wlf_font* wlf_font_load_system_font(const char *family_name, enum wlf_font_style style,
                                           enum wlf_font_weight weight, int size,
                                           const struct wlf_font_options *options);

/**
 * @brief Load default system font for a specific language.
 * @param language Language code (e.g., "en", "zh", "ja"). NULL for system default.
 * @param size Font size in pixels.
 * @param options Font rendering options (NULL for defaults).
 * @return Loaded font instance, or NULL on failure.
 */
struct wlf_font* wlf_font_load_system_default(const char *language, int size,
                                              const struct wlf_font_options *options);

/**
 * @brief Load system monospace font.
 * @param size Font size in pixels.
 * @param options Font rendering options (NULL for defaults).
 * @return Loaded font instance, or NULL on failure.
 */
struct wlf_font* wlf_font_load_system_monospace(int size, const struct wlf_font_options *options);

#endif // FONT_WLF_FONT_H
