#ifndef WLF_SHAPE_NODE_IMPL_H
#define WLF_SHAPE_NODE_IMPL_H

#include "wlf_shape_node_common.h"

#include <assert.h>
#include <stdlib.h>

#define WLF_DEFINE_SHAPE_NODE(prefix, node_type, is_function, from_function) \
static void prefix##_destroy(struct wlf_scene_node *base) { \
	struct node_type *node = from_function(base); \
	wlf_shape_destroy(&node->shape->base); \
	free(node); \
} \
static const struct wlf_scene_node_impl prefix##_impl = { \
	.destroy = prefix##_destroy, \
	.get_size = wlf_shape_node_common_get_size, \
	.invisible = wlf_shape_node_common_invisible, \
	.visibility = wlf_shape_node_common_visibility, \
	.at = wlf_shape_node_common_at, \
	.bounds = wlf_shape_node_common_bounds, \
	.in_box = wlf_shape_node_common_in_box, \
}; \
bool is_function(const struct wlf_scene_node *base) { \
	return base != NULL && base->impl == &prefix##_impl; \
} \
struct node_type *from_function(struct wlf_scene_node *base) { \
	assert(is_function(base)); \
	struct node_type *node = wlf_container_of(base, node, base); \
	return node; \
}

#endif
