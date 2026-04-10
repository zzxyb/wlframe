#include "wlf/scene/wlf_scene_rect.h"

#include <stdlib.h>

static void scene_rect_destroy(struct wlf_scene_node *node) {
	struct wlf_scene_rect *rect = (struct wlf_scene_rect *)node;
	free(rect);
}

static const struct wlf_scene_node_impl scene_rect_impl = {
	.destroy = scene_rect_destroy,
};

struct wlf_scene_rect *wlf_scene_rect_create(struct wlf_scene_tree *parent,
		double width, double height, const struct wlf_color *color) {
	struct wlf_scene_rect *rect = malloc(sizeof(*rect));
	if (rect == NULL) {
		return NULL;
	}

	wlf_scene_node_init(&rect->node, &scene_rect_impl);
	rect->node.state.width = width;
	rect->node.state.height = height;
	rect->color = color != NULL ? *color : WLF_COLOR_TRANSPARENT;

	if (parent != NULL) {
		wlf_scene_node_reparent(&rect->node, parent);
	}

	return rect;
}

bool wlf_scene_node_is_rect(const struct wlf_scene_node *node) {
	return node != NULL && node->impl == &scene_rect_impl;
}

struct wlf_scene_rect *wlf_scene_rect_from_node(struct wlf_scene_node *node) {
	if (!wlf_scene_node_is_rect(node)) {
		return NULL;
	}

	return (struct wlf_scene_rect *)node;
}
