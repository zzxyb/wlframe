#include "wlf/node/wlf_text_node.h"

#include "wlf/platform/wlf_text.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wlf_window.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void scene_node_render(struct wlf_render_list_entry *entry,
	const struct wlf_render_data *data);
static bool text_node_rasterize(struct wlf_text_node *node);

static bool text_node_invisible(struct wlf_scene_node *base) {
	struct wlf_text_node *node = wlf_text_node_from_node(base);
	return !base->state.enabled || node->texture == NULL ||
		base->state.width <= 0 || base->state.height <= 0 ||
		base->state.opacity <= 0 || node->color.a <= 0;
}

static void text_node_get_size(struct wlf_scene_node *node,
		uint32_t *width, uint32_t *height) {
	*width = node->state.width;
	*height = node->state.height;
}

static void scene_node_opaque_region(struct wlf_scene_node *node,
		int x, int y, pixman_region32_t *opaque) {
	(void)node;
	(void)x;
	(void)y;
	(void)opaque;
}

static void text_node_add_bounds(struct wlf_scene_node *node,
		int x, int y, pixman_region32_t *region) {
	pixman_region32_union_rect(region, region, x, y,
		node->state.width, node->state.height);
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
		int x, int y, pixman_region32_t *visible) {
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

	int x = 0;
	int y = 0;
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

static void handle_window_scale(struct wlf_listener *listener, void *data) {
	struct wlf_text_node *node =
		wlf_container_of(listener, node, window_scale);
	struct wlf_window *window = data;
	if (node->raster_scale == window->state.scale) {
		return;
	}
	node->raster_scale = window->state.scale;
	if (!text_node_rasterize(node)) {
		wlf_log(WLF_ERROR, "failed to rerasterize text after scale change");
	}
}

static bool text_node_rasterize(struct wlf_text_node *node) {
	struct wlf_renderer *renderer = node->renderer;
	if (renderer == NULL || renderer->impl->texture_from_buffer == NULL ||
			node->text_context == NULL) {
		return false;
	}

	struct wlf_text_raster raster = {0};
	if (!wlf_text_rasterize(node->text_context,
			&(struct wlf_text_options){
				.text = node->text,
				.font_family = node->font_family,
				.font_size = node->font_size,
				.raster_scale = node->raster_scale,
				.color = node->color,
				.max_width = node->max_width,
				.slant = node->font_slant,
				.weight = node->font_weight,
			}, &raster)) {
		wlf_log(WLF_ERROR, "failed to measure text node");
		return false;
	}

	node->natural_width = raster.metrics.width / node->raster_scale;
	node->baseline = raster.metrics.baseline / node->raster_scale;
	if (node->text[0] == '\0' || raster.width == 0 ||
			raster.height == 0 || raster.data == NULL || raster.stride == 0) {
		wlf_text_raster_destroy(node->text_context, &raster);
		text_node_set_texture(node, NULL);
		node->base.state.width = 0;
		node->base.state.height = 0;
		wlf_scene_node_update(&node->base, NULL);
		return true;
	}

	uint32_t raster_width = raster.width;
	uint32_t raster_height = raster.height;
	struct wlf_texture *texture = wlf_texture_from_pixels(renderer,
		WLF_FORMAT_ARGB8888, raster.stride, raster.width, raster.height,
		raster.data);
	wlf_text_raster_destroy(node->text_context, &raster);
	if (texture == NULL) {
		wlf_log(WLF_ERROR, "failed to create renderer texture for text node");
		return false;
	}

	text_node_set_texture(node, texture);
	node->base.state.width =
		(uint32_t)ceil(raster_width / node->raster_scale);
	node->base.state.height =
		(uint32_t)ceil(raster_height / node->raster_scale);
	wlf_scene_node_update(&node->base, NULL);
	return true;
}

static void text_node_destroy(struct wlf_scene_node *base) {
	struct wlf_text_node *node = wlf_text_node_from_node(base);
	wlf_linked_list_remove(&node->renderer_destroy.link);
	wlf_linked_list_remove(&node->window_scale.link);
	text_node_set_texture(node, NULL);
	wlf_text_destroy(node->text_context);
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
		int x, int y, const char *text, const char *font_family,
		double font_size, const struct wlf_color *color) {
	const char *text_value = text != NULL ? text : "";
	if (parent == NULL || parent->window == NULL ||
			parent->window->state.renderer == NULL || font_size <= 0 ||
			!wlf_text_is_valid_utf8(text_value)) {
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
	node->text_context = wlf_text_autocreate();
	if (node->text_context == NULL) {
		wlf_log(WLF_ERROR, "no text implementation is available for this platform");
		free(node->font_family);
		free(node->text);
		free(node);
		return NULL;
	}

	wlf_scene_node_init(&node->base, &text_node_impl, parent);
	node->base.state.x = x;
	node->base.state.y = y;
	node->font_size = font_size;
	node->raster_scale = parent->window->state.scale;
	node->color = color != NULL ? *color : WLF_COLOR_WHITE;
	node->max_width = -1;
	node->renderer = parent->window->state.renderer;
	node->renderer_destroy.notify = handle_renderer_destroy;
	wlf_signal_add(&node->renderer->events.destroy, &node->renderer_destroy);
	node->window_scale.notify = handle_window_scale;
	wlf_signal_add(&parent->window->events.scale, &node->window_scale);
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
	if (!wlf_text_is_valid_utf8(value)) {
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
	int x = 0;
	int y = 0;
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
