#include "wlf/scene/wlf_rect_node.h"

#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void rect_node_destroy(struct wlf_scene_node *node) {
	struct wlf_rect_node *rect = wlf_rect_node_from_node(node);
	free(rect);
}

static void rect_node_get_size(struct wlf_scene_node *node,
		double *width, double *height) {
	*width = node->state.width;
	*height = node->state.height;
}

static void rect_node_get_opaque_region(struct wlf_scene_node *node,
		double x, double y, pixman_region32_t *opaque) {
	struct wlf_rect_node *rect = wlf_rect_node_from_node(node);
	if (node->state.opacity != 1.0f || rect->color.a != 1.0) {
		return;
	}
	pixman_region32_union_rect(opaque, opaque, (int)x, (int)y,
		(uint32_t)node->state.width, (uint32_t)node->state.height);
}

static bool rect_node_invisible(struct wlf_scene_node *node) {
	struct wlf_rect_node *rect = wlf_rect_node_from_node(node);

	return !node->state.enabled || node->state.width <= 0 ||
		node->state.height <= 0 || node->state.opacity <= 0 ||
		rect->color.a <= 0;
}

static void rect_node_visibility(struct wlf_scene_node *node,
		pixman_region32_t *visible) {
	if (rect_node_invisible(node)) {
		return;
	}

	double x = 0;
	double y = 0;
	if (!wlf_scene_node_coords(node, &x, &y)) {
		return;
	}

	pixman_region32_union_rect(visible, visible, (int)x, (int)y,
		(uint32_t)node->state.width, (uint32_t)node->state.height);
}

static struct wlf_scene_node *rect_node_at(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	if (rect_node_invisible(node) || lx < 0 || ly < 0 ||
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

static void rect_node_bounds(struct wlf_scene_node *node,
		double x, double y, pixman_region32_t *visible) {
	if (rect_node_invisible(node)) {
		return;
	}

	pixman_region32_union_rect(visible, visible, (int)x, (int)y,
		(uint32_t)node->state.width, (uint32_t)node->state.height);
}

static bool rect_node_in_box(struct wlf_scene_node *node, struct wlf_frect *box,
		scene_node_box_iterator_func_t iterator, void *user_data) {
	if (rect_node_invisible(node)) {
		return false;
	}

	double x = 0;
	double y = 0;
	if (!wlf_scene_node_coords(node, &x, &y)) {
		return false;
	}

	bool intersects = x < box->x + box->width &&
		x + node->state.width > box->x &&
		y < box->y + box->height &&
		y + node->state.height > box->y;
	if (!intersects) {
		return false;
	}

	return iterator(node, x, y, user_data);
}

static const struct wlf_scene_node_impl rect_node_impl = {
	.destroy = rect_node_destroy,
	.set_enabled = NULL,
	.set_position = NULL,
	.set_opacity = NULL,
	.get_size = rect_node_get_size,
	.get_children = NULL,
	.get_opaque_region = rect_node_get_opaque_region,
	.invisible = rect_node_invisible,
	.visibility = rect_node_visibility,
	.at = rect_node_at,
	.coords = NULL,
	.update = NULL,
	.bounds = rect_node_bounds,
	.in_box = rect_node_in_box,
};

struct wlf_rect_node *wlf_rect_node_create(struct wlf_scene_node *parent,
		double x, double y, double width, double height,
		const struct wlf_color *color) {
	struct wlf_rect_node *rect = calloc(1, sizeof(*rect));
	if (rect == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_rect_node");
		return NULL;
	}

	wlf_scene_node_init(&rect->base, &rect_node_impl, parent);
	rect->base.state.x = x;
	rect->base.state.y = y;
	rect->base.state.width = width;
	rect->base.state.height = height;
	rect->color = color != NULL ? *color : WLF_COLOR_WHITE;
	rect->blend_mode = WLF_RENDER_BLEND_MODE_PREMULTIPLIED;
	wlf_scene_node_update(&rect->base, NULL);

	return rect;
}

bool wlf_scene_node_is_rect(const struct wlf_scene_node *node) {
	return node != NULL && node->impl == &rect_node_impl;
}

struct wlf_rect_node *wlf_rect_node_from_node(struct wlf_scene_node *node) {
	assert(wlf_scene_node_is_rect(node));

	struct wlf_rect_node *rect = wlf_container_of(node, rect, base);
	return rect;
}

void wlf_rect_node_render(struct wlf_rect_node *rect,
		struct wlf_rect_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const pixman_region32_t *clip) {
	if (rect_node_invisible(&rect->base)) {
		return;
	}

	double x = 0;
	double y = 0;
	if (!wlf_scene_node_coords(&rect->base, &x, &y)) {
		return;
	}

	struct wlf_color color = rect->color;
	color.a *= rect->base.state.opacity;
	struct wlf_render_rect_options options = {
		.box = {
			.x = x,
			.y = y,
			.width = rect->base.state.width,
			.height = rect->base.state.height,
		},
		.color = color,
		.clip = clip,
		.blend_mode = rect->blend_mode,
	};
	wlf_render_pass_add_rect(pass, render_target_info, &options);
}
