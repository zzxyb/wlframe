#include "wlf/node/wlf_path_node.h"
#include "wlf/scene/wlf_scene.h"
#include "wlf_shape_node_impl.h"

#include <math.h>
#include <stdlib.h>

static void scene_node_render(struct wlf_render_list_entry *entry,
	const struct wlf_render_data *data);

WLF_DEFINE_SHAPE_NODE(path_node, wlf_path_node,
	wlf_scene_node_is_path, wlf_path_node_from_node, scene_node_render)

static bool bounds(const struct wlf_path_shape *shape,
		double *minx, double *miny, double *maxx, double *maxy) {
	*minx = *miny = INFINITY;
	*maxx = *maxy = -INFINITY;
	for (const struct wlf_path *path = shape->paths; path != NULL; path = path->next) {
		for (int i = 0; i < path->npts; i++) {
			*minx = fmin(*minx, path->pts[i * 2]);
			*miny = fmin(*miny, path->pts[i * 2 + 1]);
			*maxx = fmax(*maxx, path->pts[i * 2]);
			*maxy = fmax(*maxy, path->pts[i * 2 + 1]);
		}
	}
	if (!isfinite(*minx)) return false;
	double pad = 1;
	if (shape->state.has_stroke && shape->state.stroke_width > 0)
		pad += shape->state.stroke_width / 2;
	*minx -= pad; *miny -= pad; *maxx += pad; *maxy += pad;
	return true;
}

struct wlf_path_node *wlf_path_node_create(struct wlf_scene_node *parent,
		int x, int y, struct wlf_path_shape *shape) {
	if (shape == NULL) return NULL;
	struct wlf_path_node *node = calloc(1, sizeof(*node));
	if (node == NULL) return NULL;
	double minx, miny, maxx, maxy;
	if (!bounds(shape, &minx, &miny, &maxx, &maxy) ||
			!wlf_shape_node_common_init(&node->base, &path_node_impl,
				parent, x, y, minx, miny, maxx, maxy)) {
		free(node);
		return NULL;
	}
	node->shape = shape;
	node->blend_mode = WLF_RENDER_BLEND_MODE_PREMULTIPLIED;
	wlf_scene_node_update(&node->base, NULL);
	return node;
}

static void render_at(struct wlf_path_node *node,
		struct wlf_path_pass *pass, struct wlf_render_target_info *target,
		const pixman_region32_t *clip, double x, double y) {
	if (node == NULL || pass == NULL) return;
	double minx, miny, maxx, maxy, ox, oy;
	if (!bounds(node->shape, &minx, &miny, &maxx, &maxy) ||
			!wlf_shape_node_common_refresh_at(&node->base, minx, miny, maxx,
				maxy, x, y, &ox, &oy)) return;
	if (wlf_shape_node_common_invisible(&node->base)) return;
	wlf_render_pass_add_path(pass, target,
		&(struct wlf_render_path_options){
			.shape = node->shape, .offset_x = ox, .offset_y = oy,
			.opacity = node->base.state.opacity, .clip = clip,
			.blend_mode = node->blend_mode });
}

void wlf_path_node_render(struct wlf_path_node *node,
		struct wlf_path_pass *pass, struct wlf_render_target_info *target,
		const pixman_region32_t *clip) {
	int x, y;
	if (node == NULL || !wlf_scene_node_coords(&node->base, &x, &y)) {
		return;
	}
	render_at(node, pass, target, clip, x, y);
}

static void scene_node_render(struct wlf_render_list_entry *entry,
		const struct wlf_render_data *data) {
	pixman_region32_t render_region;
	if (!wlf_scene_node_init_render_region(entry, data, &render_region)) {
		pixman_region32_fini(&render_region);
		return;
	}
	render_at(wlf_path_node_from_node(entry->node), data->scene->path_pass,
		data->target, &render_region, entry->x, entry->y);
	pixman_region32_fini(&render_region);
}
