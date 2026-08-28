/**
 * @file        wlf_shape_node_impl.h
 * @brief       Macro for defining concrete shape-node implementations.
 * @details     The macro supplies the common scene-node virtual methods and
 *              leaves rendering and type names to the concrete node header.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WLF_SHAPE_NODE_IMPL_H
#define WLF_SHAPE_NODE_IMPL_H

#include "wlf_shape_node_common.h"

#include <assert.h>
#include <stdlib.h>

/**
 * @brief Defines lifecycle, type checking, and common methods for a shape node.
 * @param prefix Prefix used for the generated private implementation symbol.
 * @param node_type Concrete scene-node structure type.
 * @param is_function Public type-predicate function name.
 * @param from_function Public cast function name.
 * @param render_function Concrete render callback.
 */
#define WLF_DEFINE_SHAPE_NODE(prefix, node_type, is_function, from_function, \
		render_function) \
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
	.construct_render_list_iterator = \
		wlf_shape_node_common_construct_render_list_iterator, \
	.render = render_function, \
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
