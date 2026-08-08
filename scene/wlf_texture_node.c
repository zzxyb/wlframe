#include "wlf/scene/wlf_texture_node.h"

#include "wlf/scene/wlf_scene.h"
#include "wlf/utils/wlf_log.h"
#include "wlf_scene_node_internal.h"

#include <assert.h>
#include <stdlib.h>

static void scene_node_render(struct wlf_render_list_entry *entry,
	const struct wlf_render_data *data);

static bool texture_node_invisible(struct wlf_scene_node *node) {
	struct wlf_texture_node *texture_node = wlf_texture_node_from_node(node);
	return !node->state.enabled || texture_node->texture == NULL ||
		node->state.width <= 0 || node->state.height <= 0 ||
		node->state.opacity <= 0;
}

static void texture_node_get_size(struct wlf_scene_node *node,
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

static void texture_node_visibility(struct wlf_scene_node *node,
		pixman_region32_t *visible) {
	if (!node->state.enabled) {
		return;
	}
	pixman_region32_union(visible, visible, &node->state.visible);
}

static struct wlf_scene_node *texture_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	if (texture_node_invisible(node) || lx < 0 || ly < 0 ||
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

static void texture_node_bounds(struct wlf_scene_node *node,
		double x, double y, pixman_region32_t *visible) {
	if (!texture_node_invisible(node)) {
		pixman_region32_union_rect(visible, visible, (int)x, (int)y,
			node->state.width, node->state.height);
	}
}

static bool texture_node_in_box(struct wlf_scene_node *node,
		struct wlf_frect *box, scene_node_box_iterator_func_t iterator,
		void *user_data) {
	if (texture_node_invisible(node)) {
		return false;
	}
	double x, y;
	if (!wlf_scene_node_coords(node, &x, &y)) {
		return false;
	}
	if (x >= box->x + box->width || x + node->state.width <= box->x ||
			y >= box->y + box->height || y + node->state.height <= box->y) {
		return false;
	}
	return iterator(node, x, y, user_data);
}

static void texture_node_set_texture_internal(struct wlf_texture_node *node,
		struct wlf_texture *texture) {
	wlf_linked_list_remove(&node->renderer_destroy.link);
	wlf_linked_list_init(&node->renderer_destroy.link);
	wlf_texture_destroy(node->texture);
	node->texture = texture;
	if (texture != NULL) {
		wlf_signal_add(&texture->renderer->events.destroy,
			&node->renderer_destroy);
	}
}

static void handle_renderer_destroy(struct wlf_listener *listener, void *data) {
	(void)data;
	struct wlf_texture_node *node =
		wlf_container_of(listener, node, renderer_destroy);
	texture_node_set_texture_internal(node, NULL);
}

static void texture_node_destroy(struct wlf_scene_node *base) {
	struct wlf_texture_node *node = wlf_texture_node_from_node(base);
	texture_node_set_texture_internal(node, NULL);
	free(node);
}

static const struct wlf_scene_node_impl texture_node_impl = {
	.destroy = texture_node_destroy,
	.get_size = texture_node_get_size,
	.opaque_region = scene_node_opaque_region,
	.invisible = texture_node_invisible,
	.visibility = texture_node_visibility,
	.at = texture_node_at,
	.bounds = texture_node_bounds,
	.in_box = texture_node_in_box,
	.construct_render_list_iterator =
		wlf_scene_node_add_render_list_entry,
	.render = scene_node_render,
};

struct wlf_texture_node *wlf_texture_node_create(
		struct wlf_scene_node *parent, struct wlf_texture *texture,
		double x, double y, double width, double height) {
	if (parent == NULL || texture == NULL || width < 0 || height < 0) {
		return NULL;
	}
	struct wlf_texture_node *node = calloc(1, sizeof(*node));
	if (node == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_texture_node");
		return NULL;
	}

	wlf_scene_node_init(&node->base, &texture_node_impl, parent);
	node->base.state.x = x;
	node->base.state.y = y;
	node->base.state.width = width > 0 ? width : texture->width;
	node->base.state.height = height > 0 ? height : texture->height;
	node->filter_mode = WLF_SCALE_FILTER_BILINEAR;
	node->blend_mode = WLF_RENDER_BLEND_MODE_PREMULTIPLIED;
	node->renderer_destroy.notify = handle_renderer_destroy;
	wlf_linked_list_init(&node->renderer_destroy.link);
	node->texture = texture;
	wlf_signal_add(&texture->renderer->events.destroy, &node->renderer_destroy);
	wlf_scene_node_update(&node->base, NULL);
	return node;
}

void wlf_texture_node_set_texture(struct wlf_texture_node *node,
		struct wlf_texture *texture) {
	if (node == NULL || node->texture == texture) {
		return;
	}
	texture_node_set_texture_internal(node, texture);
	wlf_scene_node_update(&node->base, NULL);
}

void wlf_texture_node_set_dest_size(struct wlf_texture_node *node,
		double width, double height) {
	if (node == NULL || width < 0 || height < 0) {
		return;
	}
	if (width == 0 && node->texture != NULL) {
		width = node->texture->width;
	}
	if (height == 0 && node->texture != NULL) {
		height = node->texture->height;
	}
	node->base.state.width = width;
	node->base.state.height = height;
	wlf_scene_node_update(&node->base, NULL);
}

bool wlf_scene_node_is_texture(const struct wlf_scene_node *node) {
	return node != NULL && node->impl == &texture_node_impl;
}

struct wlf_texture_node *wlf_texture_node_from_node(
		struct wlf_scene_node *node) {
	assert(wlf_scene_node_is_texture(node));
	struct wlf_texture_node *texture_node =
		wlf_container_of(node, texture_node, base);
	return texture_node;
}

static void texture_node_render_at(struct wlf_texture_node *node,
		struct wlf_texture_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const pixman_region32_t *clip, double x, double y) {
	if (node == NULL || pass == NULL || texture_node_invisible(&node->base)) {
		return;
	}
	struct wlf_render_texture_options options = {
		.texture = node->texture,
		.dst_box = {
			.x = x,
			.y = y,
			.width = node->base.state.width,
			.height = node->base.state.height,
		},
		.opacity = node->base.state.opacity,
		.clip = clip,
		.filter_mode = node->filter_mode,
		.blend_mode = node->blend_mode,
	};
	wlf_render_pass_add_texture(pass, render_target_info, &options);
}

void wlf_texture_node_render(struct wlf_texture_node *node,
		struct wlf_texture_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const pixman_region32_t *clip) {
	double x = 0;
	double y = 0;
	if (!wlf_scene_node_coords(&node->base, &x, &y)) {
		return;
	}
	texture_node_render_at(node, pass, render_target_info, clip, x, y);
}

static void scene_node_render(struct wlf_render_list_entry *entry,
		const struct wlf_render_data *data) {
	pixman_region32_t render_region;
	if (!wlf_scene_node_init_render_region(entry, data, &render_region)) {
		pixman_region32_fini(&render_region);
		return;
	}
	texture_node_render_at(wlf_texture_node_from_node(entry->node),
		data->scene->texture_pass, data->target, &render_region,
		entry->x, entry->y);
	pixman_region32_fini(&render_region);
}
