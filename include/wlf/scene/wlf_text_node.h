/**
 * @file        wlf_text_node.h
 * @brief       Text scene-node interface.
 * @details     Provides a scene node that shapes UTF-8 text and stores its
 *              rasterized result in a renderer texture.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef SCENE_WLF_TEXT_NODE_H
#define SCENE_WLF_TEXT_NODE_H

#include "wlf/pass/wlf_texture_pass.h"
#include "wlf/platform/wlf_text.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/texture/wlf_texture.h"
#include "wlf/types/wlf_color.h"

/**
 * @brief A scene node containing rasterized UTF-8 text.
 *
 * Text is shaped and rasterized through the platform text implementation into a
 * renderer texture owned by the node.
 */
struct wlf_text_node {
	struct wlf_scene_node base;
	char *text;
	char *font_family;
	double font_size;
	struct wlf_color color;
	int max_width;
	double natural_width;
	double baseline;
	double raster_scale;
	enum wlf_text_font_slant font_slant;
	enum wlf_text_font_weight font_weight;
	struct wlf_text *text_context;
	struct wlf_renderer *renderer;
	struct wlf_texture *texture;
	struct wlf_listener renderer_destroy;
	struct wlf_listener window_scale;
};

/**
 * @brief Creates a text node.
 * @details NULL text is treated as an empty string, NULL family selects
 *          "sans-serif", and NULL color selects white. Text starts unclipped.
 * @param parent Parent scene node.
 * @param x Initial x position relative to @p parent.
 * @param y Initial y position relative to @p parent.
 * @param text UTF-8 text to rasterize.
 * @param font_family Font family, or NULL for the default sans-serif family.
 * @param font_size Font size in logical units.
 * @param color Text color, or NULL for opaque white.
 * @return New text node, or NULL on allocation or rasterization failure.
 */
struct wlf_text_node *wlf_text_node_create(struct wlf_scene_node *parent,
	int x, int y, const char *text, const char *font_family,
	double font_size, const struct wlf_color *color);

/**
 * @brief Replaces the UTF-8 text and rerasterizes the node.
 * @param node Text node to update.
 * @param text New UTF-8 text, or NULL for an empty string.
 */
void wlf_text_node_set_text(struct wlf_text_node *node, const char *text);

/**
 * @brief Replaces the text color and rerasterizes the node.
 * @param node Text node to update.
 * @param color New color, or NULL for opaque white.
 */
void wlf_text_node_set_color(struct wlf_text_node *node,
	const struct wlf_color *color);

/**
 * @brief Replaces the font family and rerasterizes the node.
 * @param node Text node to update.
 * @param font_family New family, or NULL for the default sans-serif family.
 */
void wlf_text_node_set_font_family(struct wlf_text_node *node,
	const char *font_family);

/**
 * @brief Replaces the font size and rerasterizes the node.
 * @param node Text node to update.
 * @param font_size New font size in logical units.
 */
void wlf_text_node_set_font_size(struct wlf_text_node *node,
	double font_size);

/**
 * @brief Sets the font slant and weight and rerasterizes the node.
 * @param node Text node to update.
 * @param slant New font slant.
 * @param weight New font weight.
 */
void wlf_text_node_set_font_style(struct wlf_text_node *node,
	enum wlf_text_font_slant slant, enum wlf_text_font_weight weight);

/**
 * @brief Sets the maximum text width and rerasterizes the node.
 * @param node Text node to update.
 * @param max_width Maximum width in logical units; zero disables clipping.
 */
void wlf_text_node_set_max_width(struct wlf_text_node *node, int max_width);

/**
 * @brief Checks whether a scene node is a text node.
 * @param node Scene node to inspect.
 * @return true when @p node is a text node, false otherwise.
 */
bool wlf_scene_node_is_text(const struct wlf_scene_node *node);

/**
 * @brief Casts a scene node to a text node.
 * @param node Scene node known to be a text node.
 * @return The enclosing text node.
 * @note The function asserts when @p node has another type.
 */
struct wlf_text_node *wlf_text_node_from_node(struct wlf_scene_node *node);

/**
 * @brief Renders a text node through the supplied texture pass.
 * @param node Text node to render.
 * @param pass Texture pass used for rendering.
 * @param render_target_info Destination render target.
 * @param clip Optional clip region.
 */
void wlf_text_node_render(struct wlf_text_node *node,
	struct wlf_texture_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);

#endif // SCENE_WLF_TEXT_NODE_H
