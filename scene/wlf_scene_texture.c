#include "wlf/scene/wlf_scene_texture.h"

#include "wlf/texture/wlf_texture.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static void scene_texture_get_source_box(struct wlf_scene_texture *scene_texture,
		struct wlf_frect *box) {
	assert(box != NULL);

	if (scene_texture->texture == NULL) {
		*box = (struct wlf_frect){0};
		return;
	}

	*box = scene_texture->src_box;
	if (wlf_frect_is_empty(box)) {
		*box = (struct wlf_frect){
			.width = scene_texture->texture->width,
			.height = scene_texture->texture->height,
		};
	}
}

static void scene_texture_get_untransformed_size(
		struct wlf_scene_texture *scene_texture, double *width, double *height) {
	assert(width != NULL);
	assert(height != NULL);

	struct wlf_frect src_box;
	scene_texture_get_source_box(scene_texture, &src_box);

	if (scene_texture->dst_width > 0) {
		*width = scene_texture->dst_width;
	} else {
		*width = src_box.width;
	}

	if (scene_texture->dst_height > 0) {
		*height = scene_texture->dst_height;
	} else {
		*height = src_box.height;
	}
}

static void scene_texture_update_size_state(struct wlf_scene_texture *scene_texture) {
	double width = 0, height = 0;
	wlf_scene_node_get_size(&scene_texture->node, &width, &height);
	scene_texture->node.state.width = width;
	scene_texture->node.state.height = height;
}

static void scene_texture_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_texture *scene_texture = (struct wlf_scene_texture *)node;

	if (scene_texture->own_texture && scene_texture->texture != NULL) {
		wlf_texture_destroy(scene_texture->texture);
	}

	if (scene_texture->opaque_region != NULL) {
		wlf_region_fini(scene_texture->opaque_region);
		free(scene_texture->opaque_region);
	}

	free(scene_texture);
}

static void scene_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	struct wlf_scene_texture *scene_texture = wlf_scene_texture_from_node(node);
	double render_width, render_height;
	scene_texture_get_untransformed_size(scene_texture, &render_width, &render_height);

	struct wlf_frect transformed = {
		.width = render_width,
		.height = render_height,
	};
	wlf_frect_transform(&transformed, &transformed, scene_texture->transform,
		render_width, render_height);

	*width = transformed.width;
	*height = transformed.height;
}

static void scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
		double y, struct wlf_region *opaque) {
	struct wlf_scene_texture *scene_texture = wlf_scene_texture_from_node(node);

	wlf_region_clear(opaque);

	if (!node->state.enabled || scene_texture->texture == NULL ||
			scene_texture->opacity < 1.0f) {
		return;
	}

	if (scene_texture->opaque_region == NULL ||
			!wlf_region_not_empty(scene_texture->opaque_region)) {
		double width, height;
		wlf_scene_node_get_size(node, &width, &height);
		const struct wlf_frect rect = {
			.x = x,
			.y = y,
			.width = width,
			.height = height,
		};
		wlf_region_union_rect(opaque, opaque, &rect);
		return;
	}

	double render_width, render_height;
	scene_texture_get_untransformed_size(scene_texture, &render_width, &render_height);

	long n_rects = 0;
	const struct wlf_frect *rects =
		wlf_region_rectangles(scene_texture->opaque_region, &n_rects);
	for (long i = 0; i < n_rects; ++i) {
		struct wlf_frect rect = rects[i];
		struct wlf_frect transformed;
		wlf_frect_transform(&transformed, &rect, scene_texture->transform,
			render_width, render_height);
		transformed.x += x;
		transformed.y += y;
		wlf_region_union_rect(opaque, opaque, &transformed);
	}
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_texture *scene_texture = wlf_scene_texture_from_node(node);

	if (scene_texture->texture == NULL || scene_texture->opacity <= 0.0f) {
		return true;
	}

	double width, height;
	wlf_scene_node_get_size(node, &width, &height);
	return width <= 0.0 || height <= 0.0;
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
	.destroy = scene_texture_destroy,
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

struct wlf_scene_texture *wlf_scene_texture_create(struct wlf_scene_tree *parent,
		struct wlf_texture *texture, bool own_texture) {
	assert(parent != NULL);

	struct wlf_scene_texture *scene_texture = calloc(1, sizeof(*scene_texture));
	if (scene_texture == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_scene_texture");
		return NULL;
	}

	wlf_scene_node_init(&scene_texture->node, &scene_node_impl, &parent->base);
	scene_texture->texture = texture;
	scene_texture->opacity = 1.0f;
	scene_texture->filter_mode = WLF_SCALE_FILTER_BILINEAR;
	scene_texture->transform = WLF_OUTPUT_TRANSFORM_NORMAL;
	scene_texture->own_texture = own_texture;

	scene_texture_update_size_state(scene_texture);
	return scene_texture;
}

void wlf_scene_texture_set_texture(struct wlf_scene_texture *scene_texture,
		struct wlf_texture *texture) {
	wlf_scene_texture_set_texture_with_damage(scene_texture, texture, NULL);
}

void wlf_scene_texture_set_texture_with_damage(struct wlf_scene_texture *scene_texture,
		struct wlf_texture *texture, const struct wlf_region *region) {
	if (scene_texture->texture == texture && region == NULL) {
		return;
	}

	if (scene_texture->own_texture && scene_texture->texture != NULL &&
			scene_texture->texture != texture) {
		wlf_texture_destroy(scene_texture->texture);
	}

	scene_texture->texture = texture;
	scene_texture_update_size_state(scene_texture);
	wlf_scene_node_update(&scene_texture->node, (struct wlf_region *)region);
}

void wlf_scene_texture_set_opaque_region(struct wlf_scene_texture *scene_texture,
		const struct wlf_region *region) {
	if (region == NULL) {
		if (scene_texture->opaque_region == NULL) {
			return;
		}

		wlf_region_fini(scene_texture->opaque_region);
		free(scene_texture->opaque_region);
		scene_texture->opaque_region = NULL;
		wlf_scene_node_update(&scene_texture->node, NULL);
		return;
	}

	if (scene_texture->opaque_region != NULL &&
			wlf_region_equal(scene_texture->opaque_region, region)) {
		return;
	}

	if (scene_texture->opaque_region == NULL) {
		scene_texture->opaque_region = malloc(sizeof(*scene_texture->opaque_region));
		if (scene_texture->opaque_region == NULL) {
			wlf_log_errno(WLF_ERROR, "failed to allocate scene texture opaque region");
			return;
		}
		wlf_region_init(scene_texture->opaque_region);
	}

	wlf_region_copy(scene_texture->opaque_region, region);
	wlf_scene_node_update(&scene_texture->node, NULL);
}

void wlf_scene_texture_set_source_box(struct wlf_scene_texture *scene_texture,
		const struct wlf_frect *box) {
	struct wlf_frect next_box = {0};
	if (box != NULL) {
		assert(box->width >= 0.0 && box->height >= 0.0);
		next_box = *box;
	}

	if (wlf_frect_equal(&scene_texture->src_box, &next_box)) {
		return;
	}

	scene_texture->src_box = next_box;
	scene_texture_update_size_state(scene_texture);
	wlf_scene_node_update(&scene_texture->node, NULL);
}

void wlr_scene_buffer_set_dest_size(struct wlf_scene_texture *scene_texture,
		int width, int height) {
	assert(width >= 0 && height >= 0);

	if (scene_texture->dst_width == width &&
			scene_texture->dst_height == height) {
		return;
	}

	scene_texture->dst_width = width;
	scene_texture->dst_height = height;
	scene_texture_update_size_state(scene_texture);
	wlf_scene_node_update(&scene_texture->node, NULL);
}

void wlr_scene_buffer_set_transform(struct wlf_scene_texture *scene_texture,
		enum wlf_output_transform transform) {
	if (scene_texture->transform == transform) {
		return;
	}

	scene_texture->transform = transform;
	scene_texture_update_size_state(scene_texture);
	wlf_scene_node_update(&scene_texture->node, NULL);
}

void wlr_scene_buffer_set_filter_mode(struct wlf_scene_texture *scene_texture,
		enum wlf_scale_filter_mode filter_mode) {
	if (scene_texture->filter_mode == filter_mode) {
		return;
	}

	scene_texture->filter_mode = filter_mode;
	wlf_scene_node_update(&scene_texture->node, NULL);
}

bool wlf_scene_node_is_texture(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_texture *wlf_scene_texture_from_node(struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);

	struct wlf_scene_texture *scene_texture =
		wlf_container_of(node, scene_texture, node);

	return scene_texture;
}
