#include "wlf/scene/wlf_text_node.h"

#include "wlf/scene/wlf_scene.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wlf_window.h"
#include "wlf_scene_node_internal.h"

#include <assert.h>
#include <math.h>
#include <pango/pangocairo.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct text_metrics {
	double left;
	double top;
	double width;
	double height;
	double baseline;
};

static void scene_node_render(struct wlf_render_list_entry *entry,
	const struct wlf_render_data *data);

static PangoStyle to_pango_style(enum wlf_text_font_slant slant) {
	switch (slant) {
	case WLF_TEXT_FONT_SLANT_ITALIC:
		return PANGO_STYLE_ITALIC;
	case WLF_TEXT_FONT_SLANT_OBLIQUE:
		return PANGO_STYLE_OBLIQUE;
	case WLF_TEXT_FONT_SLANT_NORMAL:
	default:
		return PANGO_STYLE_NORMAL;
	}
}

static PangoWeight to_pango_weight(enum wlf_text_font_weight weight) {
	return weight == WLF_TEXT_FONT_WEIGHT_BOLD ?
		PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;
}

static bool text_node_invisible(struct wlf_scene_node *base) {
	struct wlf_text_node *node = wlf_text_node_from_node(base);
	return !base->state.enabled || node->texture == NULL ||
		base->state.width <= 0 || base->state.height <= 0 ||
		base->state.opacity <= 0 || node->color.a <= 0;
}

static void text_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	*width = node->state.width;
	*height = node->state.height;
}

static void scene_node_opaque_region(struct wlf_scene_node *node,
		double x, double y, pixman_region32_t *opaque) {
	(void)node;
	(void)x;
	(void)y;
	(void)opaque;
}

static void text_node_add_bounds(struct wlf_scene_node *node,
		double x, double y, pixman_region32_t *region) {
	int left = (int)floor(x);
	int top = (int)floor(y);
	int right = (int)ceil(x + node->state.width);
	int bottom = (int)ceil(y + node->state.height);
	pixman_region32_union_rect(region, region, left, top,
		(uint32_t)(right - left), (uint32_t)(bottom - top));
}

static void text_node_visibility(struct wlf_scene_node *node,
		pixman_region32_t *visible) {
	if (!node->state.enabled) {
		return;
	}
	pixman_region32_union(visible, visible, &node->state.visible);
}

static struct wlf_scene_node *text_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	if (text_node_invisible(node) || lx < 0 || ly < 0 ||
			lx >= node->state.width || ly >= node->state.height) {
		return NULL;
	}
	if (nx != NULL) {
		*nx = lx;
	}
	if (ny != NULL) {
		*ny = ly;
	}
	return node;
}

static void text_node_bounds(struct wlf_scene_node *node,
		double x, double y, pixman_region32_t *visible) {
	if (!text_node_invisible(node)) {
		text_node_add_bounds(node, x, y, visible);
	}
}

static bool text_node_in_box(struct wlf_scene_node *node,
		struct wlf_frect *box, scene_node_box_iterator_func_t iterator,
		void *user_data) {
	if (text_node_invisible(node)) {
		return false;
	}

	double x = 0;
	double y = 0;
	if (!wlf_scene_node_coords(node, &x, &y) ||
			x >= box->x + box->width || x + node->state.width <= box->x ||
			y >= box->y + box->height || y + node->state.height <= box->y) {
		return false;
	}
	return iterator(node, x, y, user_data);
}

static void text_node_set_texture(struct wlf_text_node *node,
		struct wlf_texture *texture) {
	wlf_texture_destroy(node->texture);
	node->texture = texture;
}

static void handle_renderer_destroy(struct wlf_listener *listener, void *data) {
	(void)data;
	struct wlf_text_node *node =
		wlf_container_of(listener, node, renderer_destroy);
	wlf_linked_list_remove(&node->renderer_destroy.link);
	wlf_linked_list_init(&node->renderer_destroy.link);
	text_node_set_texture(node, NULL);
	node->renderer = NULL;
}

static PangoLayout *create_layout(cairo_t *cr,
		const struct wlf_text_node *node) {
	PangoLayout *layout = pango_cairo_create_layout(cr);
	if (layout == NULL) {
		return NULL;
	}

	PangoFontDescription *font = pango_font_description_new();
	if (font == NULL) {
		g_object_unref(layout);
		return NULL;
	}
	pango_font_description_set_family(font, node->font_family);
	pango_font_description_set_absolute_size(font,
		node->font_size * PANGO_SCALE);
	pango_font_description_set_style(font, to_pango_style(node->font_slant));
	pango_font_description_set_weight(font,
		to_pango_weight(node->font_weight));
	pango_layout_set_font_description(layout, font);
	pango_font_description_free(font);

	pango_layout_set_text(layout, node->text, -1);
	pango_layout_set_auto_dir(layout, true);
	return layout;
}

static void measure_layout(PangoLayout *layout, struct text_metrics *metrics) {
	PangoRectangle ink;
	PangoRectangle logical;
	pango_layout_get_pixel_extents(layout, &ink, &logical);

	int left = MIN(0, MIN(ink.x, logical.x));
	int top = MIN(0, MIN(ink.y, logical.y));
	int right = MAX(ink.x + ink.width, logical.x + logical.width);
	int bottom = MAX(ink.y + ink.height, logical.y + logical.height);
	*metrics = (struct text_metrics){
		.left = left,
		.top = top,
		.width = right - left,
		.height = bottom - top,
		.baseline = pango_layout_get_baseline(layout) /
			(double)PANGO_SCALE - top,
	};
}

static bool text_node_rasterize(struct wlf_text_node *node) {
	struct wlf_renderer *renderer = node->renderer;
	if (renderer == NULL || renderer->impl->texture_from_buffer == NULL) {
		return false;
	}

	cairo_surface_t *measure_surface = cairo_image_surface_create(
		CAIRO_FORMAT_ARGB32, 1, 1);
	cairo_t *measure = cairo_create(measure_surface);
	PangoLayout *layout = create_layout(measure, node);
	struct text_metrics metrics = {0};
	if (layout != NULL) {
		measure_layout(layout, &metrics);
	}
	bool measured = layout != NULL &&
		cairo_status(measure) == CAIRO_STATUS_SUCCESS;
	cairo_destroy(measure);
	cairo_surface_destroy(measure_surface);
	if (!measured || metrics.width > INT32_MAX || metrics.height > INT32_MAX) {
		wlf_log(WLF_ERROR, "failed to measure text node");
		if (layout != NULL) {
			g_object_unref(layout);
		}
		return false;
	}

	node->natural_width = metrics.width;
	node->baseline = metrics.baseline;
	int width = (int)metrics.width;
	int height = (int)metrics.height;
	if (node->max_width >= 0 && width > node->max_width) {
		width = node->max_width;
	}
	if (node->text[0] == '\0' || width <= 0 || height <= 0) {
		g_object_unref(layout);
		text_node_set_texture(node, NULL);
		node->base.state.width = 0;
		node->base.state.height = 0;
		wlf_scene_node_update(&node->base, NULL);
		return true;
	}

	cairo_surface_t *surface = cairo_image_surface_create(
		CAIRO_FORMAT_ARGB32, width, height);
	if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "failed to create Cairo text surface: %s",
			cairo_status_to_string(cairo_surface_status(surface)));
		g_object_unref(layout);
		cairo_surface_destroy(surface);
		return false;
	}

	cairo_t *cr = cairo_create(surface);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	struct wlf_color color = wlf_color_clamp(&node->color);
	cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
	cairo_move_to(cr, -metrics.left, -metrics.top);
	pango_cairo_update_layout(cr, layout);
	pango_cairo_show_layout(cr, layout);
	bool drawn = cairo_status(cr) == CAIRO_STATUS_SUCCESS;
	g_object_unref(layout);
	cairo_destroy(cr);
	if (!drawn) {
		wlf_log(WLF_ERROR, "failed to rasterize text node");
		cairo_surface_destroy(surface);
		return false;
	}
	cairo_surface_flush(surface);

	struct wlf_texture *texture = wlf_texture_from_pixels(renderer,
		WLF_FORMAT_ARGB8888, cairo_image_surface_get_stride(surface),
		(uint32_t)width, (uint32_t)height,
		cairo_image_surface_get_data(surface));
	cairo_surface_destroy(surface);
	if (texture == NULL) {
		wlf_log(WLF_ERROR, "failed to create renderer texture for text node");
		return false;
	}

	text_node_set_texture(node, texture);
	node->base.state.width = width;
	node->base.state.height = height;
	wlf_scene_node_update(&node->base, NULL);
	return true;
}

static void text_node_destroy(struct wlf_scene_node *base) {
	struct wlf_text_node *node = wlf_text_node_from_node(base);
	wlf_linked_list_remove(&node->renderer_destroy.link);
	text_node_set_texture(node, NULL);
	free(node->font_family);
	free(node->text);
	free(node);
}

static const struct wlf_scene_node_impl text_node_impl = {
	.destroy = text_node_destroy,
	.get_size = text_node_get_size,
	.opaque_region = scene_node_opaque_region,
	.invisible = text_node_invisible,
	.visibility = text_node_visibility,
	.at = text_node_at,
	.bounds = text_node_bounds,
	.in_box = text_node_in_box,
	.construct_render_list_iterator =
		wlf_scene_node_add_render_list_entry,
	.render = scene_node_render,
};

struct wlf_text_node *wlf_text_node_create(struct wlf_scene_node *parent,
		double x, double y, const char *text, const char *font_family,
		double font_size, const struct wlf_color *color) {
	const char *text_value = text != NULL ? text : "";
	if (parent == NULL || parent->window == NULL ||
			parent->window->state.renderer == NULL || font_size <= 0 ||
			!g_utf8_validate(text_value, -1, NULL)) {
		return NULL;
	}

	struct wlf_text_node *node = calloc(1, sizeof(*node));
	if (node == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_text_node");
		return NULL;
	}
	node->text = strdup(text_value);
	node->font_family = strdup(font_family != NULL ?
		font_family : "sans-serif");
	if (node->text == NULL || node->font_family == NULL) {
		free(node->font_family);
		free(node->text);
		free(node);
		return NULL;
	}

	wlf_scene_node_init(&node->base, &text_node_impl, parent);
	node->base.state.x = x;
	node->base.state.y = y;
	node->font_size = font_size;
	node->color = color != NULL ? *color : WLF_COLOR_WHITE;
	node->max_width = -1;
	node->renderer = parent->window->state.renderer;
	node->renderer_destroy.notify = handle_renderer_destroy;
	wlf_signal_add(&node->renderer->events.destroy, &node->renderer_destroy);
	if (!text_node_rasterize(node)) {
		wlf_scene_node_destroy(&node->base);
		return NULL;
	}
	return node;
}

void wlf_text_node_set_text(struct wlf_text_node *node, const char *text) {
	if (node == NULL) {
		return;
	}
	const char *value = text != NULL ? text : "";
	if (!g_utf8_validate(value, -1, NULL)) {
		wlf_log(WLF_ERROR, "text node requires valid UTF-8 text");
		return;
	}
	if (strcmp(node->text, value) == 0) {
		return;
	}
	char *copy = strdup(value);
	if (copy == NULL) {
		return;
	}
	free(node->text);
	node->text = copy;
	text_node_rasterize(node);
}

void wlf_text_node_set_color(struct wlf_text_node *node,
		const struct wlf_color *color) {
	if (node == NULL || color == NULL || wlf_color_equal(&node->color, color)) {
		return;
	}
	node->color = *color;
	text_node_rasterize(node);
}

void wlf_text_node_set_font_family(struct wlf_text_node *node,
		const char *font_family) {
	if (node == NULL) {
		return;
	}
	const char *value = font_family != NULL ? font_family : "sans-serif";
	if (strcmp(node->font_family, value) == 0) {
		return;
	}
	char *copy = strdup(value);
	if (copy == NULL) {
		return;
	}
	free(node->font_family);
	node->font_family = copy;
	text_node_rasterize(node);
}

void wlf_text_node_set_font_size(struct wlf_text_node *node,
		double font_size) {
	if (node == NULL || font_size <= 0 || node->font_size == font_size) {
		return;
	}
	node->font_size = font_size;
	text_node_rasterize(node);
}

void wlf_text_node_set_font_style(struct wlf_text_node *node,
		enum wlf_text_font_slant slant, enum wlf_text_font_weight weight) {
	if (node == NULL || slant < WLF_TEXT_FONT_SLANT_NORMAL ||
			slant > WLF_TEXT_FONT_SLANT_OBLIQUE ||
			weight < WLF_TEXT_FONT_WEIGHT_NORMAL ||
			weight > WLF_TEXT_FONT_WEIGHT_BOLD ||
			(node->font_slant == slant && node->font_weight == weight)) {
		return;
	}
	node->font_slant = slant;
	node->font_weight = weight;
	text_node_rasterize(node);
}

void wlf_text_node_set_max_width(struct wlf_text_node *node, int max_width) {
	if (node == NULL || node->max_width == max_width) {
		return;
	}
	node->max_width = max_width;
	text_node_rasterize(node);
}

bool wlf_scene_node_is_text(const struct wlf_scene_node *node) {
	return node != NULL && node->impl == &text_node_impl;
}

struct wlf_text_node *wlf_text_node_from_node(struct wlf_scene_node *node) {
	assert(wlf_scene_node_is_text(node));
	struct wlf_text_node *text_node =
		wlf_container_of(node, text_node, base);
	return text_node;
}

static void text_node_render_at(struct wlf_text_node *node,
		struct wlf_texture_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const pixman_region32_t *clip, double x, double y) {
	if (node == NULL || pass == NULL || text_node_invisible(&node->base)) {
		return;
	}

	wlf_render_pass_add_texture(pass, render_target_info,
		&(struct wlf_render_texture_options){
			.texture = node->texture,
			.dst_box = {
				.x = x,
				.y = y,
				.width = node->base.state.width,
				.height = node->base.state.height,
			},
			.opacity = node->base.state.opacity,
			.clip = clip,
			.filter_mode = WLF_SCALE_FILTER_BILINEAR,
			.blend_mode = WLF_RENDER_BLEND_MODE_PREMULTIPLIED,
		});
}

void wlf_text_node_render(struct wlf_text_node *node,
		struct wlf_texture_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const pixman_region32_t *clip) {
	double x = 0;
	double y = 0;
	if (!wlf_scene_node_coords(&node->base, &x, &y)) {
		return;
	}
	text_node_render_at(node, pass, render_target_info, clip, x, y);
}

static void scene_node_render(struct wlf_render_list_entry *entry,
		const struct wlf_render_data *data) {
	pixman_region32_t render_region;
	if (!wlf_scene_node_init_render_region(entry, data, &render_region)) {
		pixman_region32_fini(&render_region);
		return;
	}
	text_node_render_at(wlf_text_node_from_node(entry->node),
		data->scene->texture_pass, data->target, &render_region,
		entry->x, entry->y);
	pixman_region32_fini(&render_region);
}
