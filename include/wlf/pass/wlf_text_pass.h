/**
 * @file        wlf_text_pass.h
 * @brief       Text rendering pass interface in wlframe.
 * @details     This file defines the options, structures, and functions required
 *              to perform text drawing operations within a rendering pass.
 * @author      YaoBing Xiao
 * @date        2026-06-03
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-03, initial version\n
 */

#ifndef PASS_WLF_TEXT_PASS_H
#define PASS_WLF_TEXT_PASS_H

#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_region.h"
#include "wlf/pass/wlf_pass.h"
#include "wlf/math/wlf_frect.h"
#include "wlf/pass/wlf_render_target_info.h"

#include <stdbool.h>

/**
 * @brief Line wrapping policy for text layout.
 */
enum wlf_text_wrap_mode {
	WLF_TEXT_WRAP_NONE = 0,      /**< Render all text on a single line except explicit newlines */
	WLF_TEXT_WRAP_WORD,          /**< Wrap at word boundaries when possible */
	WLF_TEXT_WRAP_ANYWHERE,      /**< Wrap between any glyphs */
};

/**
 * @brief Elide policy used when text cannot fit the available width.
 */
enum wlf_text_elide_mode {
	WLF_TEXT_ELIDE_NONE = 0,     /**< Do not elide overflowing text */
	WLF_TEXT_ELIDE_LEFT,         /**< Replace the beginning with an ellipsis */
	WLF_TEXT_ELIDE_MIDDLE,       /**< Replace the middle with an ellipsis */
	WLF_TEXT_ELIDE_RIGHT,        /**< Replace the end with an ellipsis */
};

/**
 * @brief Horizontal alignment for laid out text.
 */
enum wlf_text_horizontal_alignment {
	WLF_TEXT_ALIGN_LEFT = 0,     /**< Align text to the left edge */
	WLF_TEXT_ALIGN_CENTER,       /**< Center text horizontally */
	WLF_TEXT_ALIGN_RIGHT,        /**< Align text to the right edge */
	WLF_TEXT_ALIGN_JUSTIFY,      /**< Justify text lines when supported by the backend */
};

/**
 * @brief Vertical alignment for laid out text.
 */
enum wlf_text_vertical_alignment {
	WLF_TEXT_ALIGN_TOP = 0,      /**< Align text to the top edge */
	WLF_TEXT_ALIGN_VCENTER,      /**< Center text vertically */
	WLF_TEXT_ALIGN_BOTTOM,       /**< Align text to the bottom edge */
};

/**
 * @brief Estimated text layout metrics resolved from rendering options.
 */
struct wlf_text_layout_metrics {
	double content_width;        /**< Estimated intrinsic content width */
	double content_height;       /**< Estimated intrinsic content height */
	int line_count;              /**< Estimated number of laid out lines */
	bool truncated;              /**< Whether wrapping or eliding truncates the visible content */
};

/**
 * @brief Configuration options for rendering a text run.
 */
struct wlf_render_text_options {
	const char *text;                             /**< UTF-8 text content to render */
	const char *font_family;                      /**< Font family name */
	float font_size;                              /**< Font size in logical pixels */
	struct wlf_color color;                       /**< Text fill color */
	struct wlf_frect box;                         /**< Layout box on the render target; empty means use content size */
	float padding_left;                           /**< Left padding inside the layout box */
	float padding_top;                            /**< Top padding inside the layout box */
	float padding_right;                          /**< Right padding inside the layout box */
	float padding_bottom;                         /**< Bottom padding inside the layout box */
	float line_height;                            /**< Per-line height; <= 0 uses font size */
	int maximum_line_count;                       /**< Maximum number of visible lines; <= 0 means unlimited */
	enum wlf_text_wrap_mode wrap_mode;            /**< Line wrapping policy */
	enum wlf_text_elide_mode elide;               /**< Elide policy for overflow */
	enum wlf_text_horizontal_alignment horizontal_alignment; /**< Horizontal text alignment */
	enum wlf_text_vertical_alignment vertical_alignment;     /**< Vertical text alignment */
	const float *alpha;                           /**< Optional alpha multiplier, leave NULL to use full opacity */
	const struct wlf_region *clip;                /**< Clip region, leave NULL to disable clipping */
	enum wlf_render_blend_mode blend_mode;        /**< Text blend mode */
};

struct wlf_render_text_pass;

/**
 * @brief Virtual method table for text rendering pass implementations.
 */
struct wlf_render_text_pass_impl {
	/**
	 * @brief Destroys text pass specific resources.
	 *
	 * @param pass Text pass instance.
	 */
	void (*destroy)(struct wlf_render_text_pass *pass);

	/**
	 * @brief Renders text onto the specified render target.
	 *
	 * @param pass Text pass instance.
	 * @param render_target_info Target surface metadata and context.
	 * @param options Configurations including text content, layout, style, and clipping.
	 */
	void (*render)(struct wlf_render_text_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_text_options *options);
};

/**
 * @brief Base text rendering pass.
 */
struct wlf_render_text_pass {
	const struct wlf_render_text_pass_impl *impl; /**< Implementation vtable */
	struct {
		struct wlf_signal destroy;         /**< Emitted before the text pass is destroyed */
	} events;
};

void wlf_render_text_pass_init(struct wlf_render_text_pass *pass,
	const struct wlf_render_text_pass_impl *impl);
void wlf_render_text_pass_destroy(struct wlf_render_text_pass *pass);
void wlf_render_text_pass_add_text(struct wlf_render_text_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const struct wlf_render_text_options *options);

/**
 * @brief Resolves estimated layout metrics from text rendering options.
 *
 * @param options Text rendering options.
 * @param metrics Output layout metrics.
 */
void wlf_render_text_options_get_layout_metrics(
	const struct wlf_render_text_options *options,
	struct wlf_text_layout_metrics *metrics);

/**
 * @brief Gets the effective layout box used for text rendering.
 *
 * @param options Text rendering options.
 * @param box Output layout rectangle.
 */
void wlf_render_text_options_get_box(const struct wlf_render_text_options *options,
	struct wlf_frect *box);

/**
 * @brief Gets the effective alpha value used for text rendering.
 *
 * @param options Text rendering options.
 * @return Effective alpha multiplier.
 */
float wlf_render_text_options_get_alpha(const struct wlf_render_text_options *options);

#endif // PASS_WLF_TEXT_PASS_H
