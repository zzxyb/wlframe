#ifndef SCENE_WLF_TEXT_NODE_H
#define SCENE_WLF_TEXT_NODE_H

#include "wlf/pass/wlf_texture_pass.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/texture/wlf_texture.h"
#include "wlf/types/wlf_color.h"

enum wlf_text_font_slant {
	WLF_TEXT_FONT_SLANT_NORMAL,
	WLF_TEXT_FONT_SLANT_ITALIC,
	WLF_TEXT_FONT_SLANT_OBLIQUE,
};

enum wlf_text_font_weight {
	WLF_TEXT_FONT_WEIGHT_NORMAL,
	WLF_TEXT_FONT_WEIGHT_BOLD,
};

/** A scene node containing Pango/HarfBuzz-shaped, Cairo-rasterized UTF-8 text. */
struct wlf_text_node {
	struct wlf_scene_node base;
	char *text;
	char *font_family;
	double font_size;
	struct wlf_color color;
	int max_width;
	double natural_width;
	double baseline;
	enum wlf_text_font_slant font_slant;
	enum wlf_text_font_weight font_weight;
	struct wlf_renderer *renderer;
	struct wlf_texture *texture;
	struct wlf_listener renderer_destroy;
};

/**
 * Creates a text node. NULL text is treated as an empty string, NULL family
 * selects "sans-serif", and NULL color selects white. Text starts unclipped.
 */
struct wlf_text_node *wlf_text_node_create(struct wlf_scene_node *parent,
	double x, double y, const char *text, const char *font_family,
	double font_size, const struct wlf_color *color);

void wlf_text_node_set_text(struct wlf_text_node *node, const char *text);
void wlf_text_node_set_color(struct wlf_text_node *node,
	const struct wlf_color *color);
void wlf_text_node_set_font_family(struct wlf_text_node *node,
	const char *font_family);
void wlf_text_node_set_font_size(struct wlf_text_node *node,
	double font_size);
void wlf_text_node_set_font_style(struct wlf_text_node *node,
	enum wlf_text_font_slant slant, enum wlf_text_font_weight weight);
void wlf_text_node_set_max_width(struct wlf_text_node *node, int max_width);

bool wlf_scene_node_is_text(const struct wlf_scene_node *node);
struct wlf_text_node *wlf_text_node_from_node(struct wlf_scene_node *node);

void wlf_text_node_render(struct wlf_text_node *node,
	struct wlf_texture_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);

#endif // SCENE_WLF_TEXT_NODE_H
