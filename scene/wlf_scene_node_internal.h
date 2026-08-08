#ifndef WLF_SCENE_NODE_INTERNAL_H
#define WLF_SCENE_NODE_INTERNAL_H

#include "wlf/scene/wlf_scene_node.h"

bool wlf_scene_node_add_render_list_entry(struct wlf_scene_node *node,
	double lx, double ly, void *data);

bool wlf_scene_node_init_render_region(struct wlf_render_list_entry *entry,
	const struct wlf_render_data *data, pixman_region32_t *render_region);

#endif
