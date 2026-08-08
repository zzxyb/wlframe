#include "wlf/scene/wlf_ellipse_node.h"
#include "wlf_shape_node_impl.h"

#include <stdlib.h>

WLF_DEFINE_SHAPE_NODE(ellipse_node, wlf_ellipse_node,
	wlf_scene_node_is_ellipse, wlf_ellipse_node_from_node)

static void bounds(const struct wlf_ellipse_shape *shape,
		double *minx, double *miny, double *maxx, double *maxy) {
	double pad = 1 + (shape->state.has_stroke && shape->state.stroke_width > 0 ?
		shape->state.stroke_width / 2 : 0);
	*minx = shape->cx - shape->rx - pad;
	*miny = shape->cy - shape->ry - pad;
	*maxx = shape->cx + shape->rx + pad;
	*maxy = shape->cy + shape->ry + pad;
}

struct wlf_ellipse_node *wlf_ellipse_node_create(struct wlf_scene_node *parent,
		double x, double y, struct wlf_ellipse_shape *shape) {
	if (shape == NULL) return NULL;
	struct wlf_ellipse_node *node = calloc(1, sizeof(*node));
	if (node == NULL) return NULL;
	double minx, miny, maxx, maxy;
	bounds(shape, &minx, &miny, &maxx, &maxy);
	if (!wlf_shape_node_common_init(&node->base, &ellipse_node_impl,
			parent, x, y, minx, miny, maxx, maxy)) {
		free(node);
		return NULL;
	}
	node->shape = shape;
	node->blend_mode = WLF_RENDER_BLEND_MODE_PREMULTIPLIED;
	wlf_scene_node_update(&node->base, NULL);
	return node;
}

void wlf_ellipse_node_render(struct wlf_ellipse_node *node,
		struct wlf_ellipse_pass *pass, struct wlf_render_target_info *target,
		const pixman_region32_t *clip) {
	if (node == NULL || pass == NULL) return;
	double minx, miny, maxx, maxy, ox, oy;
	bounds(node->shape, &minx, &miny, &maxx, &maxy);
	if (!wlf_shape_node_common_refresh(&node->base, minx, miny, maxx, maxy, &ox, &oy)) return;
	if (wlf_shape_node_common_invisible(&node->base)) return;
	wlf_render_pass_add_ellipse(pass, target,
		&(struct wlf_render_ellipse_options){
			.shape = node->shape, .offset_x = ox, .offset_y = oy,
			.opacity = node->base.state.opacity, .clip = clip,
			.blend_mode = node->blend_mode });
}
