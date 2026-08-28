/**
 * @file        wlf_shape_node_common.h
 * @brief       Shared implementation helpers for shape scene nodes.
 * @details     Provides common sizing, hit-testing, visibility, and render
 *              list behavior used by concrete shape-node implementations.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WLF_SHAPE_NODE_COMMON_H
#define WLF_SHAPE_NODE_COMMON_H

#include "wlf/node/wlf_scene_node.h"

/**
 * @brief Initializes common state from the shape's geometric bounds.
 * @param node Scene node to initialize.
 * @param impl Node implementation table.
 * @param parent Parent scene node.
 * @param x Initial x position relative to @p parent.
 * @param y Initial y position relative to @p parent.
 * @param minx Minimum shape x coordinate.
 * @param miny Minimum shape y coordinate.
 * @param maxx Maximum shape x coordinate.
 * @param maxy Maximum shape y coordinate.
 * @return true on success, false when the bounds are invalid.
 */
bool wlf_shape_node_common_init(struct wlf_scene_node *node,
	const struct wlf_scene_node_impl *impl, struct wlf_scene_node *parent,
	int x, int y, double minx, double miny, double maxx, double maxy);
/**
 * @brief Refreshes common bounds and returns the geometry offset at a point.
 * @param node Shape node to update.
 * @param minx Minimum shape x coordinate.
 * @param miny Minimum shape y coordinate.
 * @param maxx Maximum shape x coordinate.
 * @param maxy Maximum shape y coordinate.
 * @param x Shape origin x coordinate.
 * @param y Shape origin y coordinate.
 * @param offset_x Output x offset into the cached node bounds.
 * @param offset_y Output y offset into the cached node bounds.
 * @return true on success, false when the bounds are invalid.
 */
bool wlf_shape_node_common_refresh_at(struct wlf_scene_node *node,
	double minx, double miny, double maxx, double maxy, double x, double y,
	double *offset_x, double *offset_y);
/**
 * @brief Returns the cached logical size of a shape node.
 * @param node Shape node to inspect.
 * @param width Output width.
 * @param height Output height.
 */
void wlf_shape_node_common_get_size(struct wlf_scene_node *node,
	uint32_t *width, uint32_t *height);
/**
 * @brief Checks whether the node has no renderable content.
 * @param node Shape node to inspect.
 * @return true when disabled, transparent, or empty, false otherwise.
 */
bool wlf_shape_node_common_invisible(struct wlf_scene_node *node);

/**
 * @brief Unions the node's visible region into @p visible.
 * @param node Shape node whose visibility is queried.
 * @param visible Region receiving the node's visible area.
 */
void wlf_shape_node_common_visibility(struct wlf_scene_node *node,
	pixman_region32_t *visible);
/**
 * @brief Hit-tests a shape node in local coordinates.
 * @param node Shape node to hit-test.
 * @param lx Local x coordinate.
 * @param ly Local y coordinate.
 * @param nx Optional output x coordinate.
 * @param ny Optional output y coordinate.
 * @return @p node on a hit, or NULL otherwise.
 */
struct wlf_scene_node *wlf_shape_node_common_at(struct wlf_scene_node *node,
	double lx, double ly, double *nx, double *ny);
/**
 * @brief Unions the node bounds into a region at a scene-space position.
 * @param node Shape node whose bounds are queried.
 * @param x Scene-space x coordinate.
 * @param y Scene-space y coordinate.
 * @param visible Region receiving the bounds.
 */
void wlf_shape_node_common_bounds(struct wlf_scene_node *node,
	int x, int y, pixman_region32_t *visible);
/**
 * @brief Tests box intersection and invokes the supplied iterator on a hit.
 * @param node Shape node to test.
 * @param box Query box in scene coordinates.
 * @param iterator Callback invoked when the node intersects the box.
 * @param data User data passed to @p iterator.
 * @return Callback result, or false when the node does not intersect.
 */
bool wlf_shape_node_common_in_box(struct wlf_scene_node *node,
	struct wlf_frect *box, scene_node_box_iterator_func_t iterator, void *data);
/**
 * @brief Adds an eligible shape node to the current render list.
 * @param node Shape node to add.
 * @param lx Node x coordinate in the render-list space.
 * @param ly Node y coordinate in the render-list space.
 * @param data Render-list construction context.
 * @return true when traversal should continue, false on failure.
 */
bool wlf_shape_node_common_construct_render_list_iterator(
	struct wlf_scene_node *node, int lx, int ly, void *data);

#endif
