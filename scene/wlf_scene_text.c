#include "wlf/scene/wlf_scene_text.h"

#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void scene_text_copy_string(char *dst, size_t dst_size, const char *src) {
	assert(dst != NULL);
	assert(dst_size > 0);

	if (src == NULL) {
		dst[0] = '\0';
		return;
	}

	strncpy(dst, src, dst_size - 1);
	dst[dst_size - 1] = '\0';
}

static void scene_text_sync_layout(struct wlf_scene_text *scene_text) {
	struct wlf_render_text_options options = {
		.text = scene_text->text,
		.font_family = scene_text->font_family,
		.font_size = scene_text->font_size,
		.color = scene_text->color,
		.box = {
			.width = scene_text->box_width,
			.height = scene_text->box_height,
		},
		.padding_left = scene_text->padding_left,
		.padding_top = scene_text->padding_top,
		.padding_right = scene_text->padding_right,
		.padding_bottom = scene_text->padding_bottom,
		.line_height = scene_text->line_height,
		.maximum_line_count = scene_text->maximum_line_count,
		.wrap_mode = scene_text->wrap_mode,
		.elide = scene_text->elide,
		.horizontal_alignment = scene_text->horizontal_alignment,
		.vertical_alignment = scene_text->vertical_alignment,
	};

	struct wlf_text_layout_metrics metrics;
	wlf_render_text_options_get_layout_metrics(&options, &metrics);

	scene_text->content_width = metrics.content_width;
	scene_text->content_height = metrics.content_height;
	scene_text->line_count = metrics.line_count;
	scene_text->truncated = metrics.truncated;

	struct wlf_frect box;
	wlf_render_text_options_get_box(&options, &box);
	scene_text->node.state.width = box.width;
	scene_text->node.state.height = box.height;
}

static void scene_text_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_text *scene_text = (struct wlf_scene_text *)node;

	if (scene_text->opaque_region != NULL) {
		wlf_region_fini(scene_text->opaque_region);
		free(scene_text->opaque_region);
	}

	free(scene_text);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_text *scene_text = wlf_scene_text_from_node(node);
	*width = scene_text->node.state.width;
	*height = scene_text->node.state.height;
}

static void scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
		double y, struct wlf_region *opaque) {
	struct wlf_scene_text *scene_text = wlf_scene_text_from_node(node);

	wlf_region_clear(opaque);

	if (!node->state.enabled || scene_text->opaque_region == NULL ||
			!wlf_region_not_empty(scene_text->opaque_region) ||
			node->state.opacity < 1.0f || scene_text->color.a < 1.0f) {
		return;
	}

	long n_rects = 0;
	const struct wlf_frect *rects =
		wlf_region_rectangles(scene_text->opaque_region, &n_rects);
	for (long i = 0; i < n_rects; ++i) {
		struct wlf_frect rect = rects[i];
		rect.x += x;
		rect.y += y;
		wlf_region_union_rect(opaque, opaque, &rect);
	}
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_text *scene_text = wlf_scene_text_from_node(node);

	if (scene_text->text[0] == '\0' || scene_text->font_size <= 0.0f ||
			scene_text->color.a <= 0.0f || node->state.opacity <= 0.0f) {
		return true;
	}

	return scene_text->node.state.width <= 0.0 ||
		scene_text->node.state.height <= 0.0;
}

static void scene_node_visibility(struct wlf_scene_node *node,
		struct wlf_region *visible) {
	if (!node->state.enabled || wlf_scene_node_invisible(node)) {
		return;
	}

	wlf_region_union(visible, visible, &node->state.visible);
}

static bool scene_node_at_iterator(struct wlf_scene_node *node,
		double lx, double ly, void *data) {
	struct wlf_node_at_data *at_data = data;

	double rx = at_data->lx - lx;
	double ry = at_data->ly - ly;

	at_data->rx = rx;
	at_data->ry = ry;
	at_data->node = node;
	return true;
}

static struct wlf_scene_node *scene_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	struct wlf_frect box = {
		.x = lx,
		.y = ly,
		.width = 1,
		.height = 1,
	};

	struct wlf_node_at_data data = {
		.lx = lx,
		.ly = ly,
	};

	if (wlf_scene_node_in_box(node, &box, scene_node_at_iterator, &data)) {
		if (nx) {
			*nx = data.rx;
		}
		if (ny) {
			*ny = data.ry;
		}
		return data.node;
	}

	return NULL;
}

static bool _scene_nodes_in_box(struct wlf_scene_node *node, struct wlf_frect *box,
		scene_node_box_iterator_func_t iterator, void *user_data,
		double lx, double ly) {
	if (!node->state.enabled || wlf_scene_node_invisible(node)) {
		return false;
	}

	struct wlf_frect node_box = {
		.x = lx,
		.y = ly,
	};
	scene_node_get_size(node, &node_box.width, &node_box.height);

	if (wlf_frect_intersection(&node_box, &node_box, box) &&
			iterator(node, lx, ly, user_data)) {
		return true;
	}

	return false;
}

static bool scene_nodes_in_box(struct wlf_scene_node *node, struct wlf_frect *box,
		scene_node_box_iterator_func_t iterator, void *user_data) {
	double x, y;
	wlf_scene_node_coords(node, &x, &y);

	return _scene_nodes_in_box(node, box, iterator, user_data, x, y);
}

static const struct wlf_scene_node_impl scene_node_impl = {
	.destroy = scene_text_destroy,
	.set_enabled = NULL,
	.set_position = NULL,
	.set_opacity = NULL,
	.get_size = scene_node_get_size,
	.get_children = NULL,
	.get_opaque_region = scene_node_get_opaque_region,
	.invisible = scene_node_invisible,
	.visibility = scene_node_visibility,
	.at = scene_node_at,
	.coords = NULL,
	.update = NULL,
	.bounds = NULL,
	.in_box = scene_nodes_in_box,
};

struct wlf_scene_text *wlf_scene_text_create(struct wlf_scene_tree *parent,
		const char *text, const char *font_family, float font_size,
		const struct wlf_color *color) {
	assert(parent != NULL);
	assert(font_size >= 0.0f);

	struct wlf_scene_text *scene_text = calloc(1, sizeof(*scene_text));
	if (scene_text == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_scene_text");
		return NULL;
	}

	wlf_scene_node_init(&scene_text->node, &scene_node_impl, &parent->base);
	scene_text->font_size = font_size;
	scene_text->color = color != NULL ? *color : (struct wlf_color){1.0f, 1.0f, 1.0f, 1.0f};
	scene_text->wrap_mode = WLF_TEXT_WRAP_NONE;
	scene_text->elide = WLF_TEXT_ELIDE_NONE;
	scene_text->horizontal_alignment = WLF_TEXT_ALIGN_LEFT;
	scene_text->vertical_alignment = WLF_TEXT_ALIGN_TOP;

	scene_text_copy_string(scene_text->text, sizeof(scene_text->text), text);
	scene_text_copy_string(scene_text->font_family, sizeof(scene_text->font_family),
		font_family != NULL ? font_family : "sans-serif");

	scene_text_sync_layout(scene_text);
	return scene_text;
}

void wlf_scene_text_set_text(struct wlf_scene_text *scene_text,
		const char *text) {
	char next[sizeof(scene_text->text)];
	scene_text_copy_string(next, sizeof(next), text);

	if (strcmp(scene_text->text, next) == 0) {
		return;
	}

	scene_text_copy_string(scene_text->text, sizeof(scene_text->text), next);
	scene_text_sync_layout(scene_text);
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_font_family(struct wlf_scene_text *scene_text,
		const char *font_family) {
	char next[sizeof(scene_text->font_family)];
	scene_text_copy_string(next, sizeof(next),
		font_family != NULL ? font_family : "sans-serif");

	if (strcmp(scene_text->font_family, next) == 0) {
		return;
	}

	scene_text_copy_string(scene_text->font_family,
		sizeof(scene_text->font_family), next);
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_font_size(struct wlf_scene_text *scene_text,
		float font_size) {
	assert(font_size >= 0.0f);

	if (scene_text->font_size == font_size) {
		return;
	}

	scene_text->font_size = font_size;
	scene_text_sync_layout(scene_text);
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_color(struct wlf_scene_text *scene_text,
		const struct wlf_color *color) {
	assert(color != NULL);

	if (wlf_color_equal(&scene_text->color, color)) {
		return;
	}

	scene_text->color = *color;
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_wrap_mode(struct wlf_scene_text *scene_text,
		enum wlf_text_wrap_mode wrap_mode) {
	if (scene_text->wrap_mode == wrap_mode) {
		return;
	}

	scene_text->wrap_mode = wrap_mode;
	scene_text_sync_layout(scene_text);
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_elide(struct wlf_scene_text *scene_text,
		enum wlf_text_elide_mode elide) {
	if (scene_text->elide == elide) {
		return;
	}

	scene_text->elide = elide;
	scene_text_sync_layout(scene_text);
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_horizontal_alignment(struct wlf_scene_text *scene_text,
		enum wlf_text_horizontal_alignment alignment) {
	if (scene_text->horizontal_alignment == alignment) {
		return;
	}

	scene_text->horizontal_alignment = alignment;
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_vertical_alignment(struct wlf_scene_text *scene_text,
		enum wlf_text_vertical_alignment alignment) {
	if (scene_text->vertical_alignment == alignment) {
		return;
	}

	scene_text->vertical_alignment = alignment;
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_padding(struct wlf_scene_text *scene_text,
		float left, float top, float right, float bottom) {
	assert(left >= 0.0f);
	assert(top >= 0.0f);
	assert(right >= 0.0f);
	assert(bottom >= 0.0f);

	if (scene_text->padding_left == left &&
			scene_text->padding_top == top &&
			scene_text->padding_right == right &&
			scene_text->padding_bottom == bottom) {
		return;
	}

	scene_text->padding_left = left;
	scene_text->padding_top = top;
	scene_text->padding_right = right;
	scene_text->padding_bottom = bottom;
	scene_text_sync_layout(scene_text);
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_line_height(struct wlf_scene_text *scene_text,
		float line_height) {
	if (scene_text->line_height == line_height) {
		return;
	}

	scene_text->line_height = line_height;
	scene_text_sync_layout(scene_text);
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_maximum_line_count(struct wlf_scene_text *scene_text,
		int maximum_line_count) {
	if (scene_text->maximum_line_count == maximum_line_count) {
		return;
	}

	scene_text->maximum_line_count = maximum_line_count;
	scene_text_sync_layout(scene_text);
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_box(struct wlf_scene_text *scene_text,
		double width, double height) {
	assert(width >= 0.0);
	assert(height >= 0.0);

	if (scene_text->box_width == width && scene_text->box_height == height) {
		return;
	}

	scene_text->box_width = width;
	scene_text->box_height = height;
	scene_text_sync_layout(scene_text);
	wlf_scene_node_update(&scene_text->node, NULL);
}

void wlf_scene_text_set_opaque_region(struct wlf_scene_text *scene_text,
		const struct wlf_region *region) {
	if (region == NULL) {
		if (scene_text->opaque_region == NULL) {
			return;
		}

		wlf_region_fini(scene_text->opaque_region);
		free(scene_text->opaque_region);
		scene_text->opaque_region = NULL;
		wlf_scene_node_update(&scene_text->node, NULL);
		return;
	}

	if (scene_text->opaque_region != NULL &&
			wlf_region_equal(scene_text->opaque_region, region)) {
		return;
	}

	if (scene_text->opaque_region == NULL) {
		scene_text->opaque_region = malloc(sizeof(*scene_text->opaque_region));
		if (scene_text->opaque_region == NULL) {
			wlf_log_errno(WLF_ERROR, "failed to allocate scene text opaque region");
			return;
		}
		wlf_region_init(scene_text->opaque_region);
	}

	wlf_region_copy(scene_text->opaque_region, region);
	wlf_scene_node_update(&scene_text->node, NULL);
}

double wlf_scene_text_get_content_width(const struct wlf_scene_text *scene_text) {
	return scene_text->content_width;
}

double wlf_scene_text_get_content_height(const struct wlf_scene_text *scene_text) {
	return scene_text->content_height;
}

int wlf_scene_text_get_line_count(const struct wlf_scene_text *scene_text) {
	return scene_text->line_count;
}

bool wlf_scene_text_is_truncated(const struct wlf_scene_text *scene_text) {
	return scene_text->truncated;
}

bool wlf_scene_node_is_text(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_text *wlf_scene_text_from_node(struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);

	struct wlf_scene_text *scene_text =
		wlf_container_of(node, scene_text, node);

	return scene_text;
}
