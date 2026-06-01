#include "wlf/scene/wlf_scene_lame_texture.h"

#include "wlf/texture/wlf_texture.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static double clamp_positive_exponent(double value) {
	return value > 0.0 ? value : 2.0;
}

static bool point_in_superellipse(double width, double height,
		double lame_m, double lame_n, double x, double y) {
	if (x < 0.0 || y < 0.0 || x > width || y > height) {
		return false;
	}
	if (width <= 0.0 || height <= 0.0) {
		return false;
	}

	double a = width / 2.0;
	double b = height / 2.0;
	double cx = a;
	double cy = b;
	lame_m = clamp_positive_exponent(lame_m);
	lame_n = clamp_positive_exponent(lame_n);

	double nx = fabs((x - cx) / a);
	double ny = fabs((y - cy) / b);
	return pow(nx, lame_m) + pow(ny, lame_n) <= 1.0;
}

static void scene_texture_get_source_box(
		struct wlf_scene_lame_texture *scene_texture, struct wlf_frect *box) {
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
		struct wlf_scene_lame_texture *scene_texture,
		double *width, double *height) {
	assert(width != NULL);
	assert(height != NULL);

	struct wlf_frect src_box;
	scene_texture_get_source_box(scene_texture, &src_box);
	*width = scene_texture->dst_width > 0 ? scene_texture->dst_width : src_box.width;
	*height = scene_texture->dst_height > 0 ? scene_texture->dst_height : src_box.height;
}

static void scene_texture_update_size_state(
		struct wlf_scene_lame_texture *scene_texture) {
	double width = 0.0, height = 0.0;
	wlf_scene_node_get_size(&scene_texture->node, &width, &height);
	scene_texture->node.state.width = width;
	scene_texture->node.state.height = height;
}

static void scene_lame_texture_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_lame_texture *scene_texture =
		(struct wlf_scene_lame_texture *)node;

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
	struct wlf_scene_lame_texture *scene_texture =
		wlf_scene_lame_texture_from_node(node);
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
	(void)node;
	(void)x;
	(void)y;
	wlf_region_clear(opaque);
}

static bool scene_node_invisible(struct wlf_scene_node *node) {
	struct wlf_scene_lame_texture *scene_texture =
		wlf_scene_lame_texture_from_node(node);
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
	at_data->rx = at_data->lx - lx;
	at_data->ry = at_data->ly - ly;
	at_data->node = node;
	return true;
}

static struct wlf_scene_node *scene_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	struct wlf_scene_lame_texture *scene_texture =
		wlf_scene_lame_texture_from_node(node);
	double width, height;
	wlf_scene_node_get_size(node, &width, &height);

	if (!point_in_superellipse(width, height,
			scene_texture->lame_m, scene_texture->lame_n, lx, ly)) {
		return NULL;
	}

	struct wlf_node_at_data data = {
		.lx = lx,
		.ly = ly,
	};
	scene_node_at_iterator(node, 0.0, 0.0, &data);
	if (nx) {
		*nx = data.rx;
	}
	if (ny) {
		*ny = data.ry;
	}
	return data.node;
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
	.destroy = scene_lame_texture_destroy,
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

struct wlf_scene_lame_texture *wlf_scene_lame_texture_create(
		struct wlf_scene_tree *parent, struct wlf_texture *texture,
		bool own_texture) {
	assert(parent != NULL);

	struct wlf_scene_lame_texture *scene_texture =
		calloc(1, sizeof(*scene_texture));
	if (scene_texture == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_scene_lame_texture");
		return NULL;
	}

	wlf_scene_node_init(&scene_texture->node, &scene_node_impl, &parent->base);
	scene_texture->texture = texture;
	scene_texture->opacity = 1.0f;
	scene_texture->filter_mode = WLF_SCALE_FILTER_BILINEAR;
	scene_texture->transform = WLF_OUTPUT_TRANSFORM_NORMAL;
	scene_texture->own_texture = own_texture;
	scene_texture->lame_m = 2.0;
	scene_texture->lame_n = 2.0;
	scene_texture_update_size_state(scene_texture);
	return scene_texture;
}

void wlf_scene_lame_texture_set_texture(
		struct wlf_scene_lame_texture *scene_texture,
		struct wlf_texture *texture) {
	wlf_scene_lame_texture_set_texture_with_damage(scene_texture, texture, NULL);
}

void wlf_scene_lame_texture_set_texture_with_damage(
		struct wlf_scene_lame_texture *scene_texture,
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

void wlf_scene_lame_texture_set_opaque_region(
		struct wlf_scene_lame_texture *scene_texture,
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
			wlf_log_errno(WLF_ERROR,
				"failed to allocate scene lame texture opaque region");
			return;
		}
		wlf_region_init(scene_texture->opaque_region);
	}

	wlf_region_copy(scene_texture->opaque_region, region);
	wlf_scene_node_update(&scene_texture->node, NULL);
}

void wlf_scene_lame_texture_set_source_box(
		struct wlf_scene_lame_texture *scene_texture,
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

void wlf_scene_lame_texture_set_dest_size(
		struct wlf_scene_lame_texture *scene_texture, int width, int height) {
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

void wlf_scene_lame_texture_set_transform(
		struct wlf_scene_lame_texture *scene_texture,
		enum wlf_output_transform transform) {
	if (scene_texture->transform == transform) {
		return;
	}

	scene_texture->transform = transform;
	scene_texture_update_size_state(scene_texture);
	wlf_scene_node_update(&scene_texture->node, NULL);
}

void wlf_scene_lame_texture_set_filter_mode(
		struct wlf_scene_lame_texture *scene_texture,
		enum wlf_scale_filter_mode filter_mode) {
	if (scene_texture->filter_mode == filter_mode) {
		return;
	}

	scene_texture->filter_mode = filter_mode;
	wlf_scene_node_update(&scene_texture->node, NULL);
}

void wlf_scene_lame_texture_set_lame_exponents(
		struct wlf_scene_lame_texture *scene_texture,
		double lame_m, double lame_n) {
	lame_m = clamp_positive_exponent(lame_m);
	lame_n = clamp_positive_exponent(lame_n);
	if (scene_texture->lame_m == lame_m && scene_texture->lame_n == lame_n) {
		return;
	}

	scene_texture->lame_m = lame_m;
	scene_texture->lame_n = lame_n;
	wlf_scene_node_update(&scene_texture->node, NULL);
}

bool wlf_scene_node_is_lame_texture(const struct wlf_scene_node *node) {
	return node->impl == &scene_node_impl;
}

struct wlf_scene_lame_texture *wlf_scene_lame_texture_from_node(
		struct wlf_scene_node *node) {
	assert(node->impl == &scene_node_impl);
	struct wlf_scene_lame_texture *scene_texture = NULL;
	return wlf_container_of(node, scene_texture, node);
}
