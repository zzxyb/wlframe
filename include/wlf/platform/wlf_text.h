/**
 * @file        wlf_text.h
 * @brief       Cross-platform text shaping and rasterization interface.
 * @details     The scene layer uses this interface without depending on a
 *              particular text stack. Platform implementations provide shaped text
 *              metrics and premultiplied ARGB8888 pixels for texture upload.
 * @author      YaoBing Xiao
 * @date        2026-08-12
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-12, initial version\n
 */

#ifndef PLATFORM_WLF_TEXT_H
#define PLATFORM_WLF_TEXT_H

#include "wlf/types/wlf_color.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Font slant requested from a text implementation.
 */
enum wlf_text_font_slant {
	WLF_TEXT_FONT_SLANT_NORMAL, /**< Upright glyphs. */
	WLF_TEXT_FONT_SLANT_ITALIC, /**< Designed italic glyphs. */
	WLF_TEXT_FONT_SLANT_OBLIQUE, /**< Obliquely slanted glyphs. */
};

/**
 * @brief Font weight requested from a text implementation.
 */
enum wlf_text_font_weight {
	WLF_TEXT_FONT_WEIGHT_NORMAL, /**< Normal weight. */
	WLF_TEXT_FONT_WEIGHT_BOLD, /**< Bold weight. */
};

/**
 * @brief Text shaping and rasterization options.
 *
 * The text is UTF-8 encoded. A negative maximum width disables clipping;
 * zero also disables clipping and is kept as a convenient public API value.
 */
struct wlf_text_options {
	const char *text; /**< UTF-8 text to shape. */
	const char *font_family; /**< Requested family, or NULL for sans-serif. */
	double font_size; /**< Font size in logical units. */
	double raster_scale; /**< Device scale used for rasterization. */
	struct wlf_color color; /**< Text color. */
	int max_width; /**< Maximum logical width, or a non-positive value for none. */
	enum wlf_text_font_slant slant; /**< Requested font slant. */
	enum wlf_text_font_weight weight; /**< Requested font weight. */
};

/**
 * @brief Geometric metrics returned by a text implementation.
 *
 * The bounds are expressed in raster pixels. The baseline is measured from
 * the top of the returned bounds and is also expressed in raster pixels.
 */
struct wlf_text_metrics {
	double left; /**< Left edge of the union of ink and logical bounds. */
	double top; /**< Top edge of the union of ink and logical bounds. */
	double width; /**< Natural text width in raster pixels. */
	double height; /**< Text height in raster pixels. */
	double baseline; /**< Baseline offset from the top of the bounds. */
};

/**
 * @brief Rasterized text returned by a text implementation.
 *
 * @p data remains owned by the text implementation and is valid until
 * wlf_text_raster_destroy() is called. Empty text may return zero dimensions
 * and a NULL data pointer while still providing valid metrics.
 */
struct wlf_text_raster {
	struct wlf_text_metrics metrics; /**< Natural shaped text metrics. */
	uint32_t width; /**< Raster width in pixels after clipping. */
	uint32_t height; /**< Raster height in pixels. */
	uint32_t stride; /**< Raster row stride in bytes. */
	const void *data; /**< Premultiplied ARGB8888 pixel data. */
	void *private_data; /**< Opaque storage owned by the text implementation. */
};

struct wlf_text;

/**
 * @brief Virtual methods implemented by a platform text implementation.
 */

struct wlf_text_impl {
	const char *name; /**< Human-readable implementation name. */
	bool (*rasterize)(struct wlf_text *text,
		const struct wlf_text_options *options,
		struct wlf_text_raster *raster); /**< Shape and rasterize text. */
	void (*destroy_raster)(struct wlf_text *text,
		struct wlf_text_raster *raster); /**< Release raster-owned resources. */
	void (*destroy)(struct wlf_text *text); /**< Destroy the text implementation. */
};

/**
 * @brief Platform-independent text object.
 */
struct wlf_text {
	const struct wlf_text_impl *impl; /**< Text implementation methods. */
};

/**
 * @brief Initialize a text object with its implementation table.
 * @param text Text object to initialize.
 * @param impl Implementation table supplied by a platform implementation.
 */
void wlf_text_init(struct wlf_text *text, const struct wlf_text_impl *impl);

/**
 * @brief Create the text implementation for the current platform.
 *
 * Linux currently uses the Cairo/Pango/HarfBuzz implementation. The native
 * Core Text and DirectWrite implementations are reserved for later work, so
 * this function returns NULL on macOS and Windows for now.
 *
 * @return A newly allocated text object, or NULL when no implementation is available.
 */
struct wlf_text *wlf_text_autocreate(void);

/**
 * @brief Destroy a text object.
 * @param text Text object to destroy. NULL is allowed.
 */
void wlf_text_destroy(struct wlf_text *text);

/**
 * @brief Shape and rasterize text through a text implementation.
 * @param text Text object to use.
 * @param options Text shaping and rasterization options.
 * @param raster Output raster and metrics.
 * @return true on success, false on invalid input or implementation failure.
 */
bool wlf_text_rasterize(struct wlf_text *text,
	const struct wlf_text_options *options,
	struct wlf_text_raster *raster);

/**
 * @brief Release a raster returned by wlf_text_rasterize().
 * @param text Text object that produced the raster.
 * @param raster Raster to release. NULL is allowed.
 */
void wlf_text_raster_destroy(struct wlf_text *text,
	struct wlf_text_raster *raster);

/**
 * @brief Check whether a string is valid UTF-8.
 * @param text NUL-terminated string to check.
 * @return true for valid UTF-8, false for NULL or malformed input.
 */
bool wlf_text_is_valid_utf8(const char *text);

#endif // PLATFORM_WLF_TEXT_H
