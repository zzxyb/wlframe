/**
 * @file        wlf_scene_text.h
 * @brief       Text scene node type for wlframe.
 * @details     This file provides the text-based scene node used by wlframe scene graphs.
 *              A text scene node is a concrete `wlf_scene_node` implementation that stores
 *              UTF-8 text content together with layout properties such as wrapping, eliding,
 *              alignment, padding, and maximum line count.
 * @author      YaoBing Xiao
 * @date        2026-06-03
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-03, initial version\n
 */

#ifndef SCENE_WLF_SCENE_TEXT_H
#define SCENE_WLF_SCENE_TEXT_H

#include "wlf/math/wlf_region.h"
#include "wlf/pass/wlf_text_pass.h"
#include "wlf/types/wlf_color.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"

/**
 * @brief Text scene node.
 *
 * A concrete scene node that renders UTF-8 text in the scene graph.
 * It stores textual content, typography attributes, and layout constraints,
 * while exposing derived content metrics such as line count and truncation.
 */
struct wlf_scene_text {
	struct wlf_scene_node node;                       /**< Base scene node */

	char text[256];                                   /**< UTF-8 text content */
	char font_family[64];                             /**< Font family name */
	float font_size;                                  /**< Font size in logical pixels */
	struct wlf_color color;                           /**< Text fill color */

	double box_width;                                 /**< Explicit layout box width, or 0 to use content width */
	double box_height;                                /**< Explicit layout box height, or 0 to use content height */
	float padding_left;                               /**< Left padding inside the layout box */
	float padding_top;                                /**< Top padding inside the layout box */
	float padding_right;                              /**< Right padding inside the layout box */
	float padding_bottom;                             /**< Bottom padding inside the layout box */
	float line_height;                                /**< Per-line height, or <= 0 to use font size */
	int maximum_line_count;                           /**< Maximum visible lines, or <= 0 for unlimited */
	enum wlf_text_wrap_mode wrap_mode;                /**< Text wrapping policy */
	enum wlf_text_elide_mode elide;                   /**< Text elide policy */
	enum wlf_text_horizontal_alignment horizontal_alignment; /**< Horizontal alignment */
	enum wlf_text_vertical_alignment vertical_alignment;     /**< Vertical alignment */

	double content_width;                             /**< Estimated intrinsic content width */
	double content_height;                            /**< Estimated intrinsic content height */
	int line_count;                                   /**< Estimated visible line count */
	bool truncated;                                   /**< Whether the visible text is truncated */

	struct wlf_region *opaque_region;                 /**< Optional opaque region in local coordinates */
};

struct wlf_scene_text *wlf_scene_text_create(struct wlf_scene_tree *parent,
	const char *text, const char *font_family, float font_size,
	const struct wlf_color *color);
void wlf_scene_text_set_text(struct wlf_scene_text *scene_text,
	const char *text);
void wlf_scene_text_set_font_family(struct wlf_scene_text *scene_text,
	const char *font_family);
void wlf_scene_text_set_font_size(struct wlf_scene_text *scene_text,
	float font_size);
void wlf_scene_text_set_color(struct wlf_scene_text *scene_text,
	const struct wlf_color *color);
void wlf_scene_text_set_wrap_mode(struct wlf_scene_text *scene_text,
	enum wlf_text_wrap_mode wrap_mode);
void wlf_scene_text_set_elide(struct wlf_scene_text *scene_text,
	enum wlf_text_elide_mode elide);
void wlf_scene_text_set_horizontal_alignment(struct wlf_scene_text *scene_text,
	enum wlf_text_horizontal_alignment alignment);
void wlf_scene_text_set_vertical_alignment(struct wlf_scene_text *scene_text,
	enum wlf_text_vertical_alignment alignment);
void wlf_scene_text_set_padding(struct wlf_scene_text *scene_text,
	float left, float top, float right, float bottom);
void wlf_scene_text_set_line_height(struct wlf_scene_text *scene_text,
	float line_height);
void wlf_scene_text_set_maximum_line_count(struct wlf_scene_text *scene_text,
	int maximum_line_count);
void wlf_scene_text_set_box(struct wlf_scene_text *scene_text,
	double width, double height);
void wlf_scene_text_set_opaque_region(struct wlf_scene_text *scene_text,
	const struct wlf_region *region);

double wlf_scene_text_get_content_width(const struct wlf_scene_text *scene_text);
double wlf_scene_text_get_content_height(const struct wlf_scene_text *scene_text);
int wlf_scene_text_get_line_count(const struct wlf_scene_text *scene_text);
bool wlf_scene_text_is_truncated(const struct wlf_scene_text *scene_text);

bool wlf_scene_node_is_text(const struct wlf_scene_node *node);
struct wlf_scene_text *wlf_scene_text_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_TEXT_H
